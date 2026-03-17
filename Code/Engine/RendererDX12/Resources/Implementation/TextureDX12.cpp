#include <RendererDX12/RendererDX12PCH.h>

#include <Foundation/Logging/Log.h>
#include <RendererDX12/Device/DeviceDX12.h>
#include <RendererDX12/MemoryAllocator/MemoryAllocatorDX12.h>
#include <RendererDX12/Pools/DescriptorHeapPoolDX12.h>
#include <RendererDX12/Resources/TextureDX12.h>
#include <RendererDX12/Utils/ConversionUtilsDX12.h>

#include <d3d12.h>

ezGALTextureDX12::ezGALTextureDX12(const ezGALTextureCreationDescription& Description)
  : ezGALTexture(Description)
{
}

ezGALTextureDX12::~ezGALTextureDX12() = default;

ezResult ezGALTextureDX12::InitPlatform(ezGALDevice* pDevice, ezArrayPtr<ezGALSystemMemoryDescription> pInitialData)
{
  ezGALDeviceDX12* pDXDevice = static_cast<ezGALDeviceDX12*>(pDevice);
  ID3D12Device* pD3D12Device = pDXDevice->GetDXDevice();

  if (m_Description.m_pExisitingNativeObject != nullptr)
  {
    return InitFromNativeObject(pDXDevice);
  }

  // Build D3D12 resource description.
  const ezGALFormatLookupEntryDX12& formatEntry = pDXDevice->GetFormatLookupTable().GetFormatInfo(m_Description.m_Format);
  D3D12_RESOURCE_DESC resourceDesc = {};
  resourceDesc.MipLevels = static_cast<UINT16>(m_Description.m_uiMipLevelCount);
  resourceDesc.SampleDesc = ezConversionUtilsDX12::ToD3D12SampleDesc(m_Description.m_SampleCount);
  resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  resourceDesc.Flags = ezConversionUtilsDX12::ToD3D12TextureResourceFlags(m_Description);

  // D3D12 requires MSAA textures to have ALLOW_RENDER_TARGET or ALLOW_DEPTH_STENCIL.
  if (resourceDesc.SampleDesc.Count > 1 &&
    !(resourceDesc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)))
  {
    if (ezGALResourceFormat::IsDepthFormat(m_Description.m_Format))
      resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    else
      resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  }

  // For depth textures and render targets, use the typeless storage format to allow multiple view types.
  // For regular textures, use the typed resource view format directly.
  if (resourceDesc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
  {
    resourceDesc.Format = formatEntry.m_eStorage;
  }
  else
  {
    resourceDesc.Format = formatEntry.m_eResourceViewType;
  }

  if (resourceDesc.Format == DXGI_FORMAT_UNKNOWN)
  {
    ezLog::Error("No valid DXGI format available for the given texture format.");
    return EZ_FAILURE;
  }

  m_DXGIFormat = formatEntry.m_eResourceViewType;

  // For depth formats, the resource view format comes from the depth-only type.
  if (ezGALResourceFormat::IsDepthFormat(m_Description.m_Format))
  {
    m_DXGIFormat = formatEntry.m_eDepthOnlyType;
  }

  switch (m_Description.m_Type)
  {
    case ezGALTextureType::Texture2D:
    case ezGALTextureType::Texture2DShared:
    case ezGALTextureType::Texture2DProxy:
    case ezGALTextureType::Texture2DArray:
    {
      resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
      resourceDesc.Width = m_Description.m_uiWidth;
      resourceDesc.Height = m_Description.m_uiHeight;
      resourceDesc.DepthOrArraySize = static_cast<UINT16>(m_Description.m_uiArraySize);
      resourceDesc.Alignment = 0;
    }
    break;

    case ezGALTextureType::TextureCube:
    {
      resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
      resourceDesc.Width = m_Description.m_uiWidth;
      resourceDesc.Height = m_Description.m_uiHeight;
      resourceDesc.DepthOrArraySize = 6;
      resourceDesc.Alignment = 0;
    }
    break;

    case ezGALTextureType::TextureCubeArray:
    {
      resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
      resourceDesc.Width = m_Description.m_uiWidth;
      resourceDesc.Height = m_Description.m_uiHeight;
      resourceDesc.DepthOrArraySize = static_cast<UINT16>(m_Description.m_uiArraySize * 6);
      resourceDesc.Alignment = 0;
    }
    break;

    case ezGALTextureType::Texture3D:
    {
      resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
      resourceDesc.Width = m_Description.m_uiWidth;
      resourceDesc.Height = m_Description.m_uiHeight;
      resourceDesc.DepthOrArraySize = static_cast<UINT16>(m_Description.m_uiDepth);
      resourceDesc.Alignment = 0;
    }
    break;

    default:
      EZ_ASSERT_NOT_IMPLEMENTED;
      return EZ_FAILURE;
  }

  // Determine heap type and initial state.
  ezDX12AllocationInfo allocInfo;
  allocInfo.m_HeapType = D3D12_HEAP_TYPE_DEFAULT;

  D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;

  if (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
  {
    initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
  }
  else if (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
  {
    initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;
  }

  // Set up optimized clear value for render targets.
  D3D12_CLEAR_VALUE clearValue = {};
  D3D12_CLEAR_VALUE* pClearValue = nullptr;
  if (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
  {
    clearValue.Format = formatEntry.m_eDepthStencilType;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;
    pClearValue = &clearValue;
  }
  else if (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
  {
    clearValue.Format = formatEntry.m_eRenderTarget;
    clearValue.Color[0] = 0.0f;
    clearValue.Color[1] = 0.0f;
    clearValue.Color[2] = 0.0f;
    clearValue.Color[3] = 0.0f;
    pClearValue = &clearValue;
  }

  HRESULT hr = ezMemoryAllocatorDX12::CreateTexture(resourceDesc, allocInfo, initialState, pClearValue, &m_pResource, &m_pAllocation);
  if (FAILED(hr))
  {
    ezLog::Error("Failed to create D3D12 texture resource: {} (Dim={}, Format={}, W={}, H={}, DepthOrArray={}, Mips={}, Samples={}, Flags={})",
      ezArgErrorCode(hr),
      (ezUInt32)resourceDesc.Dimension,
      (ezUInt32)resourceDesc.Format,
      (ezUInt64)resourceDesc.Width,
      (ezUInt32)resourceDesc.Height,
      (ezUInt32)resourceDesc.DepthOrArraySize,
      (ezUInt32)resourceDesc.MipLevels,
      (ezUInt32)resourceDesc.SampleDesc.Count,
      (ezUInt32)resourceDesc.Flags);
    return EZ_FAILURE;
  }

  m_CurrentState = initialState;

  // Create SRV descriptor if the texture supports shader resource access.
  if (m_Description.m_TextureFlags.IsAnySet(ezGALTextureUsageFlags::ShaderResource | ezGALTextureUsageFlags::DynamicMipGeneration))
  {
    m_SRVDescriptor = pDXDevice->GetDescriptorHeapPool().AllocateStagingSrvUavCbv();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = m_DXGIFormat;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    switch (m_Description.m_Type)
    {
      case ezGALTextureType::Texture2D:
      case ezGALTextureType::Texture2DShared:
        if (m_Description.m_SampleCount == ezGALMSAASampleCount::None)
        {
          srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
          srvDesc.Texture2D.MostDetailedMip = 0;
          srvDesc.Texture2D.MipLevels = m_Description.m_uiMipLevelCount;
          srvDesc.Texture2D.PlaneSlice = 0;
          srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        }
        else
        {
          srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        }
        break;

      case ezGALTextureType::Texture2DProxy:
      case ezGALTextureType::Texture2DArray:
        if (m_Description.m_SampleCount == ezGALMSAASampleCount::None)
        {
          srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
          srvDesc.Texture2DArray.MostDetailedMip = 0;
          srvDesc.Texture2DArray.MipLevels = m_Description.m_uiMipLevelCount;
          srvDesc.Texture2DArray.FirstArraySlice = 0;
          srvDesc.Texture2DArray.ArraySize = m_Description.m_uiArraySize;
          srvDesc.Texture2DArray.PlaneSlice = 0;
          srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
        }
        else
        {
          srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
          srvDesc.Texture2DMSArray.FirstArraySlice = 0;
          srvDesc.Texture2DMSArray.ArraySize = m_Description.m_uiArraySize;
        }
        break;

      case ezGALTextureType::TextureCube:
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MostDetailedMip = 0;
        srvDesc.TextureCube.MipLevels = m_Description.m_uiMipLevelCount;
        srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
        break;

      case ezGALTextureType::TextureCubeArray:
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
        srvDesc.TextureCubeArray.MostDetailedMip = 0;
        srvDesc.TextureCubeArray.MipLevels = m_Description.m_uiMipLevelCount;
        srvDesc.TextureCubeArray.First2DArrayFace = 0;
        srvDesc.TextureCubeArray.NumCubes = m_Description.m_uiArraySize;
        srvDesc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
        break;

      case ezGALTextureType::Texture3D:
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        srvDesc.Texture3D.MostDetailedMip = 0;
        srvDesc.Texture3D.MipLevels = m_Description.m_uiMipLevelCount;
        srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
        break;

      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
        return EZ_FAILURE;
    }

    pD3D12Device->CreateShaderResourceView(m_pResource, &srvDesc, m_SRVDescriptor);
  }

  // Initial data upload for textures is deferred to the device, which handles
  // staging upload and layout transitions via command lists.

  return EZ_SUCCESS;
}

ezResult ezGALTextureDX12::InitFromNativeObject(ezGALDeviceDX12* pDXDevice)
{
  EZ_IGNORE_UNUSED(pDXDevice);
  m_pResource = static_cast<ID3D12Resource*>(m_Description.m_pExisitingNativeObject);
  m_CurrentState = D3D12_RESOURCE_STATE_COMMON;

  D3D12_RESOURCE_DESC desc = m_pResource->GetDesc();
  m_DXGIFormat = desc.Format;

  return EZ_SUCCESS;
}

ezResult ezGALTextureDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  // If the texture was created from a native object, we do not own it.
  if (m_Description.m_pExisitingNativeObject != nullptr)
  {
    m_pResource = nullptr;
    m_pAllocation = nullptr;
  }
  else
  {
    ezMemoryAllocatorDX12::DestroyResource(m_pResource, m_pAllocation);
  }

  m_SRVDescriptor = {};
  m_CurrentState = D3D12_RESOURCE_STATE_COMMON;
  m_DXGIFormat = DXGI_FORMAT_UNKNOWN;

  return EZ_SUCCESS;
}

void ezGALTextureDX12::SetDebugNamePlatform(const char* szName) const
{
  if (m_pResource != nullptr)
  {
    ezStringWChar wName(szName);
    m_pResource->SetName(wName.GetData());
  }
}
