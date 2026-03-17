#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/Device/DeviceDX12.h>
#include <RendererDX12/Shader/PipelineLayoutDX12.h>
#include <RendererDX12/State/ComputePipelineDX12.h>

#include <Foundation/Logging/Log.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Shader/Shader.h>
#include <RendererFoundation/Shader/ShaderByteCode.h>

ezGALComputePipelineDX12::ezGALComputePipelineDX12(const ezGALComputePipelineCreationDescription& description)
  : ezGALComputePipeline(description)
{
}

ezGALComputePipelineDX12::~ezGALComputePipelineDX12() = default;

ezResult ezGALComputePipelineDX12::InitPlatform(ezGALDevice* pDevice)
{
  ezGALDeviceDX12* pDX12Device = static_cast<ezGALDeviceDX12*>(pDevice);

  const ezGALShader* pShader = pDevice->GetShader(m_Description.m_hShader);
  if (pShader == nullptr)
  {
    ezLog::Error("Compute pipeline creation failed: invalid shader handle.");
    return EZ_FAILURE;
  }

  const ezGALShaderCreationDescription& shaderDesc = pShader->GetDescription();
  if (!shaderDesc.HasByteCodeForStage(ezGALShaderStage::ComputeShader))
  {
    ezLog::Error("Compute pipeline creation failed: shader has no compute stage bytecode.");
    return EZ_FAILURE;
  }

  D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};

  // Get root signature from pipeline layout
  const ezGALPipelineLayoutDX12* pPipelineLayout = static_cast<const ezGALPipelineLayoutDX12*>(pDevice->GetPipelineLayout(pShader->GetPipelineLayout()));
  if (pPipelineLayout == nullptr)
  {
    ezLog::Error("Compute pipeline creation failed: shader has no pipeline layout.");
    return EZ_FAILURE;
  }
  psoDesc.pRootSignature = pPipelineLayout->GetRootSignature();
  if (psoDesc.pRootSignature == nullptr)
  {
    ezLog::Error("Compute pipeline creation failed: shader has no root signature.");
    return EZ_FAILURE;
  }

  const auto& byteCode = shaderDesc.m_ByteCodes[ezGALShaderStage::ComputeShader];
  psoDesc.CS = {byteCode->GetByteCode(), byteCode->GetSize()};

  HRESULT hr = pDX12Device->GetDXDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState));
  if (FAILED(hr))
  {
    ezLog::Error("Failed to create D3D12 compute pipeline state object (HRESULT: {0}).", ezArgErrorCode(hr));
    return EZ_FAILURE;
  }

  return EZ_SUCCESS;
}

ezResult ezGALComputePipelineDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  EZ_GAL_DX12_RELEASE(m_pPipelineState);
  return EZ_SUCCESS;
}

void ezGALComputePipelineDX12::SetDebugName(const char* szName)
{
  if (m_pPipelineState != nullptr && szName != nullptr && *szName != '\0')
  {
    ezStringWChar wName(szName);
    m_pPipelineState->SetName(wName.GetData());
  }
}
