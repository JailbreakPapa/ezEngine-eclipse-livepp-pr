#include <RendererDX12/RendererDX12PCH.h>

#include <Foundation/Logging/Log.h>
#include <RendererDX12/Device/DeviceDX12.h>
#include <RendererDX12/MemoryAllocator/MemoryAllocatorDX12.h>
#include <RendererDX12/Pools/DescriptorHeapPoolDX12.h>
#include <RendererDX12/Resources/BufferDX12.h>
#include <RendererDX12/Utils/ConversionUtilsDX12.h>
#include <Foundation/Platform/Win/Utils/HResultUtils.h>
#include <d3d12.h>

ezGALBufferDX12::ezGALBufferDX12(const ezGALBufferCreationDescription& Description)
  : ezGALBuffer(Description)
{
}

ezGALBufferDX12::~ezGALBufferDX12() = default;

ezResult ezGALBufferDX12::InitPlatform(ezGALDevice* pDevice, ezArrayPtr<const ezUInt8> pInitialData)
{
  ezGALDeviceDX12* pDXDevice = static_cast<ezGALDeviceDX12*>(pDevice);
  ID3D12Device* pD3D12Device = pDXDevice->GetDXDevice();

  // Determine index format for index buffers.
  if (m_Description.m_BufferFlags.IsSet(ezGALBufferUsageFlags::IndexBuffer))
  {
    m_IndexFormat = m_Description.m_uiStructSize == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
  }

  // Build D3D12 resource description.
  D3D12_RESOURCE_DESC resourceDesc = {};
  resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  resourceDesc.Alignment = 0;
  resourceDesc.Height = 1;
  resourceDesc.DepthOrArraySize = 1;
  resourceDesc.MipLevels = 1;
  resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
  resourceDesc.SampleDesc.Count = 1;
  resourceDesc.SampleDesc.Quality = 0;
  resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

  ezUInt64 uiTotalSize = m_Description.m_uiTotalSize;

  // Constant buffers must be 256-byte aligned in D3D12.
  if (m_Description.m_BufferFlags.IsSet(ezGALBufferUsageFlags::ConstantBuffer))
  {
    uiTotalSize = ezMemoryUtils::AlignSize<ezUInt64>(uiTotalSize, 256);
  }

  resourceDesc.Width = uiTotalSize;
  resourceDesc.Flags = static_cast<D3D12_RESOURCE_FLAGS>(ezConversionUtilsDX12::ToD3D12ResourceFlags(m_Description));

  // Determine heap type and initial state.
  ezDX12AllocationInfo allocInfo;

  const bool bIsCPUAccessible =
    m_Description.m_BufferFlags.IsSet(ezGALBufferUsageFlags::ConstantBuffer) ||
    (m_Description.m_BufferFlags.IsSet(ezGALBufferUsageFlags::Transient) &&
      !m_Description.m_BufferFlags.IsSet(ezGALBufferUsageFlags::UnorderedAccess));

  if (bIsCPUAccessible)
  {
    allocInfo.m_HeapType = D3D12_HEAP_TYPE_UPLOAD;
  }
  else
  {
    allocInfo.m_HeapType = D3D12_HEAP_TYPE_DEFAULT;
  }

  // Upload heaps must start in GENERIC_READ state.
  D3D12_RESOURCE_STATES initialState = (allocInfo.m_HeapType == D3D12_HEAP_TYPE_UPLOAD)
                                          ? D3D12_RESOURCE_STATE_GENERIC_READ
                                          : D3D12_RESOURCE_STATE_COMMON;

  HRESULT hr = ezMemoryAllocatorDX12::CreateBuffer(resourceDesc, allocInfo, initialState, &m_pResource, &m_pAllocation);
  if (FAILED(hr))
  {
    ezLog::Error("Failed to create D3D12 buffer resource: {0}, Device Reason: {1}", ezArgErrorCode(hr), ezHRESULTtoString(pD3D12Device->GetDeviceRemovedReason()));
    return EZ_FAILURE;
  }

  m_CurrentState = initialState;

  // Upload initial data if provided.
  if (!pInitialData.IsEmpty())
  {
    if (allocInfo.m_HeapType == D3D12_HEAP_TYPE_UPLOAD)
    {
      // For upload heap buffers, map and copy directly.
      void* pMappedData = nullptr;
      D3D12_RANGE readRange = {0, 0}; // We do not read from this resource on the CPU.
      hr = m_pResource->Map(0, &readRange, &pMappedData);
      if (SUCCEEDED(hr))
      {
        ezMemoryUtils::RawByteCopy(pMappedData, pInitialData.GetPtr(), pInitialData.GetCount());
        D3D12_RANGE writeRange = {0, pInitialData.GetCount()};
        m_pResource->Unmap(0, &writeRange);
      }
      else
      {
        ezLog::Error("Failed to map D3D12 upload buffer for initial data: {}", ezArgErrorCode(hr));
        return EZ_FAILURE;
      }
    }
    else
    {
      // For default heap buffers, initial data upload is deferred to the device.
      // The device will handle staging upload via command list after resource creation.
    }
  }

  // Create SRV descriptor if the buffer supports shader resource access.
  // Pure constant buffers use CBVs (created at bind time), not SRVs.
  const bool bNeedsSRV = m_Description.m_BufferFlags.IsSet(ezGALBufferUsageFlags::ShaderResource) &&
    m_Description.m_BufferFlags.IsAnySet(ezGALBufferUsageFlags::ByteAddressBuffer | ezGALBufferUsageFlags::StructuredBuffer | ezGALBufferUsageFlags::TexelBuffer);

  if (bNeedsSRV)
  {
    m_SRVDescriptor = pDXDevice->GetDescriptorHeapPool().AllocateStagingSrvUavCbv();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if (m_Description.m_BufferFlags.IsSet(ezGALBufferUsageFlags::ByteAddressBuffer))
    {
      srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
      srvDesc.Buffer.FirstElement = 0;
      srvDesc.Buffer.NumElements = static_cast<UINT>(m_Description.m_uiTotalSize / 4);
      srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
    }
    else if (m_Description.m_BufferFlags.IsSet(ezGALBufferUsageFlags::StructuredBuffer))
    {
      srvDesc.Format = DXGI_FORMAT_UNKNOWN;
      srvDesc.Buffer.FirstElement = 0;
      srvDesc.Buffer.NumElements = static_cast<UINT>(m_Description.m_uiTotalSize / m_Description.m_uiStructSize);
      srvDesc.Buffer.StructureByteStride = m_Description.m_uiStructSize;
    }
    else if (m_Description.m_BufferFlags.IsSet(ezGALBufferUsageFlags::TexelBuffer))
    {
      srvDesc.Format = ezConversionUtilsDX12::ToDXGIFormat(m_Description.m_Format);
      ezUInt32 uiBytesPerElement = ezGALResourceFormat::GetBitsPerElement(m_Description.m_Format) / 8;
      srvDesc.Buffer.FirstElement = 0;
      srvDesc.Buffer.NumElements = static_cast<UINT>(m_Description.m_uiTotalSize / uiBytesPerElement);
    }

    pD3D12Device->CreateShaderResourceView(m_pResource, &srvDesc, m_SRVDescriptor);
  }

  // Create UAV descriptor if the buffer supports unordered access.
  // Only typed buffer variants (ByteAddress, Structured, Texel) can have valid UAV descriptors.
  const bool bNeedsUAV = m_Description.m_BufferFlags.IsSet(ezGALBufferUsageFlags::UnorderedAccess) &&
    m_Description.m_BufferFlags.IsAnySet(ezGALBufferUsageFlags::ByteAddressBuffer | ezGALBufferUsageFlags::StructuredBuffer | ezGALBufferUsageFlags::TexelBuffer);

  if (bNeedsUAV)
  {
    m_UAVDescriptor = pDXDevice->GetDescriptorHeapPool().AllocateStagingSrvUavCbv();

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

    if (m_Description.m_BufferFlags.IsSet(ezGALBufferUsageFlags::ByteAddressBuffer))
    {
      uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
      uavDesc.Buffer.FirstElement = 0;
      uavDesc.Buffer.NumElements = static_cast<UINT>(m_Description.m_uiTotalSize / 4);
      uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
    }
    else if (m_Description.m_BufferFlags.IsSet(ezGALBufferUsageFlags::StructuredBuffer))
    {
      uavDesc.Format = DXGI_FORMAT_UNKNOWN;
      uavDesc.Buffer.FirstElement = 0;
      uavDesc.Buffer.NumElements = static_cast<UINT>(m_Description.m_uiTotalSize / m_Description.m_uiStructSize);
      uavDesc.Buffer.StructureByteStride = m_Description.m_uiStructSize;
    }
    else if (m_Description.m_BufferFlags.IsSet(ezGALBufferUsageFlags::TexelBuffer))
    {
      uavDesc.Format = ezConversionUtilsDX12::ToDXGIFormat(m_Description.m_Format);
      ezUInt32 uiBytesPerElement = ezGALResourceFormat::GetBitsPerElement(m_Description.m_Format) / 8;
      uavDesc.Buffer.FirstElement = 0;
      uavDesc.Buffer.NumElements = static_cast<UINT>(m_Description.m_uiTotalSize / uiBytesPerElement);
    }

    pD3D12Device->CreateUnorderedAccessView(m_pResource, nullptr, &uavDesc, m_UAVDescriptor);
  }

  return EZ_SUCCESS;
}

ezResult ezGALBufferDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  ezMemoryAllocatorDX12::DestroyResource(m_pResource, m_pAllocation);
  m_SRVDescriptor = {};
  m_UAVDescriptor = {};
  m_CurrentState = D3D12_RESOURCE_STATE_COMMON;

  return EZ_SUCCESS;
}

void ezGALBufferDX12::SetDebugNamePlatform(const char* szName) const
{
  if (m_pResource != nullptr)
  {
    ezStringWChar wName(szName);
    m_pResource->SetName(wName.GetData());
  }
}
