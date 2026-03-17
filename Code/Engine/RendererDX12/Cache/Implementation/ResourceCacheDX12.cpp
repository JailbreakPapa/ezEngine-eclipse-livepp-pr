#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/Cache/ResourceCacheDX12.h>

ezResourceCacheDX12::~ezResourceCacheDX12()
{
  DeInitialize();
}

void ezResourceCacheDX12::Initialize(ID3D12Device* pDevice)
{
  m_pDevice = pDevice;

  // Try to create a pipeline library for caching PSOs
  // This is optional - it simply improves startup time by caching PSO compilation
  ID3D12Device1* pDevice1 = nullptr;
  if (SUCCEEDED(pDevice->QueryInterface(IID_PPV_ARGS(&pDevice1))))
  {
    // Create an empty pipeline library
    HRESULT hr = pDevice1->CreatePipelineLibrary(nullptr, 0, IID_PPV_ARGS(&m_pPipelineLibrary));
    if (FAILED(hr))
    {
      m_pPipelineLibrary = nullptr;
    }
    pDevice1->Release();
  }
}

void ezResourceCacheDX12::DeInitialize()
{
  EZ_GAL_DX12_RELEASE(m_pPipelineLibrary);
  m_pDevice = nullptr;
}

ID3D12PipelineState* ezResourceCacheDX12::LoadGraphicsPipeline(const wchar_t* szName, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
{
  if (!m_pPipelineLibrary)
    return nullptr;

  ID3D12PipelineState* pPipeline = nullptr;
  HRESULT hr = m_pPipelineLibrary->LoadGraphicsPipeline(szName, &desc, IID_PPV_ARGS(&pPipeline));
  if (SUCCEEDED(hr))
    return pPipeline;

  return nullptr;
}

ID3D12PipelineState* ezResourceCacheDX12::LoadComputePipeline(const wchar_t* szName, const D3D12_COMPUTE_PIPELINE_STATE_DESC& desc)
{
  if (!m_pPipelineLibrary)
    return nullptr;

  ID3D12PipelineState* pPipeline = nullptr;
  HRESULT hr = m_pPipelineLibrary->LoadComputePipeline(szName, &desc, IID_PPV_ARGS(&pPipeline));
  if (SUCCEEDED(hr))
    return pPipeline;

  return nullptr;
}

void ezResourceCacheDX12::StorePipeline(const wchar_t* szName, ID3D12PipelineState* pPipeline)
{
  if (!m_pPipelineLibrary || !pPipeline)
    return;

  // StorePipeline may fail if the name already exists, which is fine
  m_pPipelineLibrary->StorePipeline(szName, pPipeline);
}
