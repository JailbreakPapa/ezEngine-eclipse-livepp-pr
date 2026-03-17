#pragma once

#include <RendererDX12/RendererDX12DLL.h>
#include <d3d12.h>

/// Optional PSO caching via ID3D12PipelineLibrary.
class EZ_RENDERERDX12_DLL ezResourceCacheDX12
{
public:
  ezResourceCacheDX12() = default;
  ~ezResourceCacheDX12();

  void Initialize(ID3D12Device* pDevice);
  void DeInitialize();

  /// Tries to load a cached PSO. Returns nullptr if not found.
  ID3D12PipelineState* LoadGraphicsPipeline(const wchar_t* szName, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);
  ID3D12PipelineState* LoadComputePipeline(const wchar_t* szName, const D3D12_COMPUTE_PIPELINE_STATE_DESC& desc);

  /// Stores a PSO in the cache for future reuse.
  void StorePipeline(const wchar_t* szName, ID3D12PipelineState* pPipeline);

private:
  ID3D12Device* m_pDevice = nullptr;
  ID3D12PipelineLibrary* m_pPipelineLibrary = nullptr;
};
