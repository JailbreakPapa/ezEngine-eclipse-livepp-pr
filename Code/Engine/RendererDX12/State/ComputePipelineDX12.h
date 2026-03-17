#pragma once

#include <RendererDX12/RendererDX12DLL.h>
#include <RendererFoundation/State/ComputePipeline.h>

#include <d3d12.h>

class ezGALDeviceDX12;

class EZ_RENDERERDX12_DLL ezGALComputePipelineDX12 : public ezGALComputePipeline
{
public:
  ID3D12PipelineState* GetPipelineState() const { return m_pPipelineState; }

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  ezGALComputePipelineDX12(const ezGALComputePipelineCreationDescription& description);
  ~ezGALComputePipelineDX12();

  virtual ezResult InitPlatform(ezGALDevice* pDevice) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;
  virtual void SetDebugName(const char* szName) override;

  ID3D12PipelineState* m_pPipelineState = nullptr;
};
