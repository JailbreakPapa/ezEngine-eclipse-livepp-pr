#include <RendererDX12/RendererDX12PCH.h>

#include <Foundation/Logging/Log.h>
#include <RendererDX12/Device/DeviceDX12.h>
#include <RendererDX12/Resources/ReadbackBufferDX12.h>

#include <d3d12.h>

ezGALReadbackBufferDX12::ezGALReadbackBufferDX12(const ezGALBufferCreationDescription& Description)
  : ezGALReadbackBuffer(Description)
{
}

ezGALReadbackBufferDX12::~ezGALReadbackBufferDX12() = default;

ezResult ezGALReadbackBufferDX12::InitPlatform(ezGALDevice* pDevice)
{
  ezGALDeviceDX12* pDXDevice = static_cast<ezGALDeviceDX12*>(pDevice);
  ID3D12Device* pD3D12Device = pDXDevice->GetDXDevice();

  D3D12_RESOURCE_DESC resourceDesc = {};
  resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  resourceDesc.Alignment = 0;
  resourceDesc.Width = m_Description.m_uiTotalSize;
  resourceDesc.Height = 1;
  resourceDesc.DepthOrArraySize = 1;
  resourceDesc.MipLevels = 1;
  resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
  resourceDesc.SampleDesc.Count = 1;
  resourceDesc.SampleDesc.Quality = 0;
  resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

  D3D12_HEAP_PROPERTIES heapProps = {};
  heapProps.Type = D3D12_HEAP_TYPE_READBACK;
  heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

  HRESULT hr = pD3D12Device->CreateCommittedResource(
    &heapProps,
    D3D12_HEAP_FLAG_NONE,
    &resourceDesc,
    D3D12_RESOURCE_STATE_COPY_DEST,
    nullptr,
    IID_PPV_ARGS(&m_pReadbackResource));

  if (FAILED(hr))
  {
    ezLog::Error("Failed to create D3D12 readback buffer: {}", ezArgErrorCode(hr));
    return EZ_FAILURE;
  }

  return EZ_SUCCESS;
}

ezResult ezGALReadbackBufferDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  EZ_GAL_DX12_RELEASE(m_pReadbackResource);
  return EZ_SUCCESS;
}

void ezGALReadbackBufferDX12::SetDebugNamePlatform(const char* szName) const
{
  if (m_pReadbackResource != nullptr)
  {
    ezStringWChar wName(szName);
    m_pReadbackResource->SetName(wName.GetData());
  }
}
