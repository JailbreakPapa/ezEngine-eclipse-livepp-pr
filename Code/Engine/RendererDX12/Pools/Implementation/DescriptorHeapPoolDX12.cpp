#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/Pools/DescriptorHeapPoolDX12.h>

namespace
{
  ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type,
    ezUInt32 uiCount, bool bShaderVisible)
  {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = type;
    desc.NumDescriptors = uiCount;
    desc.Flags = bShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ID3D12DescriptorHeap* pHeap = nullptr;
    HRESULT hr = pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pHeap));
    EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create descriptor heap: {}", ezArgErrorCode(hr));
    EZ_IGNORE_UNUSED(hr);
    return pHeap;
  }
} // namespace

ezDescriptorHeapPoolDX12::~ezDescriptorHeapPoolDX12()
{
  DeInitialize();
}

void ezDescriptorHeapPoolDX12::Initialize(ID3D12Device* pDevice)
{
  m_pDevice = pDevice;

  // Get descriptor sizes
  m_uiSrvUavCbvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  m_uiSamplerDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  m_uiRTVDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  m_uiDSVDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

  // Create shader-visible heaps
  m_pSrvUavCbvHeap = CreateDescriptorHeap(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, MAX_SRV_UAV_CBV, true);
  m_pSamplerHeap = CreateDescriptorHeap(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, MAX_SAMPLERS, true);

  // Create staging (non-shader-visible) heaps
  m_pStagingSrvUavCbvHeap = CreateDescriptorHeap(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, STAGING_HEAP_SIZE, false);
  m_pStagingSamplerHeap = CreateDescriptorHeap(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, STAGING_HEAP_SIZE, false);
  m_pStagingRTVHeap = CreateDescriptorHeap(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, STAGING_HEAP_SIZE, false);
  m_pStagingDSVHeap = CreateDescriptorHeap(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, STAGING_HEAP_SIZE, false);

  m_uiSrvUavCbvTransientStart = 0;
  m_uiSamplerTransientStart = 0;
  m_uiStagingSrvUavCbvNext = 0;
  m_uiStagingSamplerNext = 0;
  m_uiStagingRTVNext = 0;
  m_uiStagingDSVNext = 0;
}

void ezDescriptorHeapPoolDX12::DeInitialize()
{
  EZ_GAL_DX12_RELEASE(m_pSrvUavCbvHeap);
  EZ_GAL_DX12_RELEASE(m_pSamplerHeap);
  EZ_GAL_DX12_RELEASE(m_pStagingSrvUavCbvHeap);
  EZ_GAL_DX12_RELEASE(m_pStagingSamplerHeap);
  EZ_GAL_DX12_RELEASE(m_pStagingRTVHeap);
  EZ_GAL_DX12_RELEASE(m_pStagingDSVHeap);
  m_pDevice = nullptr;
}

ezDescriptorRangeDX12 ezDescriptorHeapPoolDX12::AllocateTransientSrvUavCbv(ezUInt32 uiCount)
{
  EZ_ASSERT_DEV(m_uiSrvUavCbvTransientStart + uiCount <= MAX_SRV_UAV_CBV,
    "Shader-visible SRV/UAV/CBV descriptor heap exhausted");

  ezDescriptorRangeDX12 range;
  range.m_uiStartIndex = m_uiSrvUavCbvTransientStart;
  range.m_uiCount = uiCount;

  D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = m_pSrvUavCbvHeap->GetCPUDescriptorHandleForHeapStart();
  cpuStart.ptr += static_cast<SIZE_T>(m_uiSrvUavCbvTransientStart) * m_uiSrvUavCbvDescriptorSize;
  range.m_cpuStart = cpuStart;

  D3D12_GPU_DESCRIPTOR_HANDLE gpuStart = m_pSrvUavCbvHeap->GetGPUDescriptorHandleForHeapStart();
  gpuStart.ptr += static_cast<UINT64>(m_uiSrvUavCbvTransientStart) * m_uiSrvUavCbvDescriptorSize;
  range.m_gpuStart = gpuStart;

  m_uiSrvUavCbvTransientStart += uiCount;
  return range;
}

ezDescriptorRangeDX12 ezDescriptorHeapPoolDX12::AllocateTransientSampler(ezUInt32 uiCount)
{
  EZ_ASSERT_DEV(m_uiSamplerTransientStart + uiCount <= MAX_SAMPLERS,
    "Shader-visible sampler descriptor heap exhausted");

  ezDescriptorRangeDX12 range;
  range.m_uiStartIndex = m_uiSamplerTransientStart;
  range.m_uiCount = uiCount;

  D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = m_pSamplerHeap->GetCPUDescriptorHandleForHeapStart();
  cpuStart.ptr += static_cast<SIZE_T>(m_uiSamplerTransientStart) * m_uiSamplerDescriptorSize;
  range.m_cpuStart = cpuStart;

  D3D12_GPU_DESCRIPTOR_HANDLE gpuStart = m_pSamplerHeap->GetGPUDescriptorHandleForHeapStart();
  gpuStart.ptr += static_cast<UINT64>(m_uiSamplerTransientStart) * m_uiSamplerDescriptorSize;
  range.m_gpuStart = gpuStart;

  m_uiSamplerTransientStart += uiCount;
  return range;
}

void ezDescriptorHeapPoolDX12::BeginFrame()
{
  // Only reset the shader-visible transient allocators.
  // Staging heap allocations are persistent (they hold resource SRVs, UAVs, RTVs, DSVs
  // that live for the lifetime of their resources) and must NOT be reset.
  m_uiSrvUavCbvTransientStart = 0;
  m_uiSamplerTransientStart = 0;
}

D3D12_CPU_DESCRIPTOR_HANDLE ezDescriptorHeapPoolDX12::AllocateStagingSrvUavCbv()
{
  EZ_ASSERT_DEV(m_uiStagingSrvUavCbvNext < STAGING_HEAP_SIZE, "Staging SRV/UAV/CBV heap exhausted");

  D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pStagingSrvUavCbvHeap->GetCPUDescriptorHandleForHeapStart();
  handle.ptr += static_cast<SIZE_T>(m_uiStagingSrvUavCbvNext) * m_uiSrvUavCbvDescriptorSize;
  m_uiStagingSrvUavCbvNext++;
  return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE ezDescriptorHeapPoolDX12::AllocateStagingSampler()
{
  EZ_ASSERT_DEV(m_uiStagingSamplerNext < STAGING_HEAP_SIZE, "Staging sampler heap exhausted");

  D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pStagingSamplerHeap->GetCPUDescriptorHandleForHeapStart();
  handle.ptr += static_cast<SIZE_T>(m_uiStagingSamplerNext) * m_uiSamplerDescriptorSize;
  m_uiStagingSamplerNext++;
  return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE ezDescriptorHeapPoolDX12::AllocateStagingRTV()
{
  EZ_ASSERT_DEV(m_uiStagingRTVNext < STAGING_HEAP_SIZE, "Staging RTV heap exhausted");

  D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pStagingRTVHeap->GetCPUDescriptorHandleForHeapStart();
  handle.ptr += static_cast<SIZE_T>(m_uiStagingRTVNext) * m_uiRTVDescriptorSize;
  m_uiStagingRTVNext++;
  return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE ezDescriptorHeapPoolDX12::AllocateStagingDSV()
{
  EZ_ASSERT_DEV(m_uiStagingDSVNext < STAGING_HEAP_SIZE, "Staging DSV heap exhausted");

  D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pStagingDSVHeap->GetCPUDescriptorHandleForHeapStart();
  handle.ptr += static_cast<SIZE_T>(m_uiStagingDSVNext) * m_uiDSVDescriptorSize;
  m_uiStagingDSVNext++;
  return handle;
}

void ezDescriptorHeapPoolDX12::CopyToShaderVisible(ezUInt32 uiCount, const D3D12_CPU_DESCRIPTOR_HANDLE* pSrcHandles,
  const ezDescriptorRangeDX12& destRange)
{
  EZ_ASSERT_DEV(uiCount <= destRange.m_uiCount, "Source descriptor count exceeds destination range");

  for (ezUInt32 i = 0; i < uiCount; ++i)
  {
    D3D12_CPU_DESCRIPTOR_HANDLE dest = destRange.m_cpuStart;
    dest.ptr += static_cast<SIZE_T>(i) * m_uiSrvUavCbvDescriptorSize;
    m_pDevice->CopyDescriptorsSimple(1, dest, pSrcHandles[i], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  }
}
