#include <RendererDX12/RendererDX12PCH.h>

#include <Foundation/Logging/Log.h>
#include <RendererDX12/Device/DeviceDX12.h>
#include <RendererDX12/MemoryAllocator/MemoryAllocatorDX12.h>
#include <RendererDX12/Resources/SharedTextureDX12.h>
#include <RendererDX12/Utils/ConversionUtilsDX12.h>

#include <d3d12.h>

//////////////////////////////////////////////////////////////////////////
// ezGALSharedTextureDX12
//////////////////////////////////////////////////////////////////////////

ezGALSharedTextureDX12::ezGALSharedTextureDX12(const ezGALTextureCreationDescription& Description, ezEnum<ezGALSharedTextureType> sharedType, ezGALPlatformSharedHandle hSharedHandle)
  : ezGALTextureDX12(Description)
  , m_SharedType(sharedType)
  , m_hSharedHandle(hSharedHandle)
{
}

ezGALSharedTextureDX12::~ezGALSharedTextureDX12() = default;

ezResult ezGALSharedTextureDX12::InitPlatform(ezGALDevice* pDevice, ezArrayPtr<ezGALSystemMemoryDescription> pInitialData)
{
  EZ_IGNORE_UNUSED(pInitialData);

  EZ_ASSERT_DEBUG(m_SharedType != ezGALSharedTextureType::None, "Shared texture must either be exported or imported.");
  EZ_ASSERT_DEBUG(m_Description.m_Type == ezGALTextureType::Texture2DShared, "Shared texture must be of type ezGALTextureType::Texture2DShared.");

  ezGALDeviceDX12* pDXDevice = static_cast<ezGALDeviceDX12*>(pDevice);
  ID3D12Device* pD3D12Device = pDXDevice->GetDXDevice();

  if (m_SharedType == ezGALSharedTextureType::Imported)
  {
    // Open the shared texture handle from another process.
    HRESULT hr = pD3D12Device->OpenSharedHandle(
      (HANDLE)m_hSharedHandle.m_hSharedTexture,
      IID_PPV_ARGS(&m_pResource));
    if (FAILED(hr))
    {
      ezLog::Error("Failed to open shared D3D12 texture: {}", ezArgErrorCode(hr));
      return EZ_FAILURE;
    }

    D3D12_RESOURCE_DESC desc = m_pResource->GetDesc();
    m_DXGIFormat = desc.Format;
    m_CurrentState = D3D12_RESOURCE_STATE_COMMON;

    // Open the shared fence for cross-process synchronization.
    if (m_hSharedHandle.m_hSemaphore != 0)
    {
      hr = pD3D12Device->OpenSharedHandle(
        (HANDLE)m_hSharedHandle.m_hSemaphore,
        IID_PPV_ARGS(&m_pFence));
      if (FAILED(hr))
      {
        ezLog::Error("Failed to open shared fence: {}", ezArgErrorCode(hr));
        return EZ_FAILURE;
      }
    }

    return EZ_SUCCESS;
  }

  // Exported: create the texture resource and then create shared handles.
  D3D12_RESOURCE_DESC resourceDesc = {};
  resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  resourceDesc.Width = m_Description.m_uiWidth;
  resourceDesc.Height = m_Description.m_uiHeight;
  resourceDesc.DepthOrArraySize = 1;
  resourceDesc.MipLevels = static_cast<UINT16>(m_Description.m_uiMipLevelCount);
  resourceDesc.Format = ezConversionUtilsDX12::ToDXGIFormat(m_Description.m_Format);
  resourceDesc.SampleDesc = ezConversionUtilsDX12::ToD3D12SampleDesc(m_Description.m_SampleCount);
  resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  resourceDesc.Flags = ezConversionUtilsDX12::ToD3D12TextureResourceFlags(m_Description);

  if (resourceDesc.Format == DXGI_FORMAT_UNKNOWN)
  {
    ezLog::Error("No valid DXGI format for shared texture.");
    return EZ_FAILURE;
  }

  m_DXGIFormat = resourceDesc.Format;

  // Shared textures require D3D12_HEAP_FLAG_SHARED on the heap.
  ezDX12AllocationInfo allocInfo;
  allocInfo.m_HeapType = D3D12_HEAP_TYPE_DEFAULT;
  allocInfo.m_ExtraHeapFlags = D3D12_HEAP_FLAG_SHARED;

  D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;

  HRESULT hr = ezMemoryAllocatorDX12::CreateTexture(resourceDesc, allocInfo, initialState, nullptr, &m_pResource, &m_pAllocation);
  if (FAILED(hr))
  {
    ezLog::Error("Failed to create shared D3D12 texture resource: {}", ezArgErrorCode(hr));
    return EZ_FAILURE;
  }

  m_CurrentState = initialState;

  // Create a shared handle for the texture resource.
  HANDLE hSharedTexture = nullptr;
  hr = pD3D12Device->CreateSharedHandle(m_pResource, nullptr, GENERIC_ALL, nullptr, &hSharedTexture);
  if (FAILED(hr))
  {
    ezLog::Error("Failed to create shared handle for texture: {}", ezArgErrorCode(hr));
    return EZ_FAILURE;
  }
  m_hSharedHandle.m_hSharedTexture = (ezUInt64)hSharedTexture;

  // Create a shared fence for cross-process synchronization.
  hr = pD3D12Device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&m_pFence));
  if (FAILED(hr))
  {
    ezLog::Error("Failed to create shared fence: {}", ezArgErrorCode(hr));
    return EZ_FAILURE;
  }

  hr = pD3D12Device->CreateSharedHandle(m_pFence, nullptr, GENERIC_ALL, nullptr, &m_hSharedFenceHandle);
  if (FAILED(hr))
  {
    ezLog::Error("Failed to create shared handle for fence: {}", ezArgErrorCode(hr));
    return EZ_FAILURE;
  }
  m_hSharedHandle.m_hSemaphore = (ezUInt64)m_hSharedFenceHandle;

  return EZ_SUCCESS;
}

ezResult ezGALSharedTextureDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  if (m_hSharedFenceHandle != nullptr)
  {
    CloseHandle(m_hSharedFenceHandle);
    m_hSharedFenceHandle = nullptr;
  }

  EZ_GAL_DX12_RELEASE(m_pFence);

  return SUPER::DeInitPlatform(pDevice);
}

ezGALPlatformSharedHandle ezGALSharedTextureDX12::GetSharedHandle() const
{
  return m_hSharedHandle;
}

void ezGALSharedTextureDX12::WaitSemaphoreGPU(ezUInt64 uiValue) const
{
  // In D3D12, GPU waits are performed via ID3D12CommandQueue::Wait on a fence.
  // The device/command queue enqueues the wait before executing command lists
  // that access this texture. This is handled by the device class.
}

void ezGALSharedTextureDX12::SignalSemaphoreGPU(ezUInt64 uiValue) const
{
  // In D3D12, GPU signals are performed via ID3D12CommandQueue::Signal on a fence.
  // The device/command queue enqueues the signal after executing command lists
  // that access this texture. This is handled by the device class.
}
