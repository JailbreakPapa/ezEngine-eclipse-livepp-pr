#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/Device/DeviceDX12.h>
#include <RendererDX12/Shader/PipelineLayoutDX12.h>
#include <RendererDX12/Shader/VertexDeclarationDX12.h>
#include <RendererDX12/State/GraphicsPipelineDX12.h>
#include <RendererDX12/State/StateDX12.h>
#include <RendererDX12/Utils/ConversionUtilsDX12.h>

#include <Foundation/Logging/Log.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Shader/Shader.h>
#include <RendererFoundation/Shader/ShaderByteCode.h>

ezGALGraphicsPipelineDX12::ezGALGraphicsPipelineDX12(const ezGALGraphicsPipelineCreationDescription& description)
  : ezGALGraphicsPipeline(description)
{
}

ezGALGraphicsPipelineDX12::~ezGALGraphicsPipelineDX12() = default;

ezResult ezGALGraphicsPipelineDX12::InitPlatform(ezGALDevice* pDevice)
{
  ezGALDeviceDX12* pDX12Device = static_cast<ezGALDeviceDX12*>(pDevice);

  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

  // --- Shader bytecodes ---

  const ezGALShader* pShader = pDevice->GetShader(m_Description.m_hShader);
  if (pShader == nullptr)
  {
    ezLog::Error("Graphics pipeline creation failed: invalid shader handle.");
    return EZ_FAILURE;
  }

  const ezGALShaderCreationDescription& shaderDesc = pShader->GetDescription();

  if (shaderDesc.HasByteCodeForStage(ezGALShaderStage::VertexShader))
  {
    const auto& byteCode = shaderDesc.m_ByteCodes[ezGALShaderStage::VertexShader];
    psoDesc.VS = {byteCode->GetByteCode(), byteCode->GetSize()};
  }

  if (shaderDesc.HasByteCodeForStage(ezGALShaderStage::PixelShader))
  {
    const auto& byteCode = shaderDesc.m_ByteCodes[ezGALShaderStage::PixelShader];
    psoDesc.PS = {byteCode->GetByteCode(), byteCode->GetSize()};
  }

  if (shaderDesc.HasByteCodeForStage(ezGALShaderStage::HullShader))
  {
    const auto& byteCode = shaderDesc.m_ByteCodes[ezGALShaderStage::HullShader];
    psoDesc.HS = {byteCode->GetByteCode(), byteCode->GetSize()};
  }

  if (shaderDesc.HasByteCodeForStage(ezGALShaderStage::DomainShader))
  {
    const auto& byteCode = shaderDesc.m_ByteCodes[ezGALShaderStage::DomainShader];
    psoDesc.DS = {byteCode->GetByteCode(), byteCode->GetSize()};
  }

  if (shaderDesc.HasByteCodeForStage(ezGALShaderStage::GeometryShader))
  {
    const auto& byteCode = shaderDesc.m_ByteCodes[ezGALShaderStage::GeometryShader];
    psoDesc.GS = {byteCode->GetByteCode(), byteCode->GetSize()};
  }

  // --- Root signature from pipeline layout ---

  // Get root signature from pipeline layout
  const ezGALPipelineLayoutDX12* pPipelineLayout = static_cast<const ezGALPipelineLayoutDX12*>(pDevice->GetPipelineLayout(pShader->GetPipelineLayout()));
  if (pPipelineLayout == nullptr)
  {
    ezLog::Error("Graphics pipeline creation failed: shader has no pipeline layout.");
    return EZ_FAILURE;
  }
  psoDesc.pRootSignature = pPipelineLayout->GetRootSignature();

  if (psoDesc.pRootSignature == nullptr)
  {
    ezLog::Error("Graphics pipeline creation failed: shader has no root signature.");
    return EZ_FAILURE;
  }

  // --- Input layout from vertex declaration ---

  const ezGALVertexDeclarationDX12* pVertexDecl = nullptr;
  if (!m_Description.m_hVertexDeclaration.IsInvalidated())
  {
    const ezGALVertexDeclaration* pVertexDeclBase = pDevice->GetVertexDeclaration(m_Description.m_hVertexDeclaration);
    if (pVertexDeclBase != nullptr)
    {
      pVertexDecl = static_cast<const ezGALVertexDeclarationDX12*>(pVertexDeclBase);
      psoDesc.InputLayout.pInputElementDescs = pVertexDecl->GetInputElements().GetPtr();
      psoDesc.InputLayout.NumElements = pVertexDecl->GetInputElements().GetCount();
    }
  }

  // --- Blend state ---

  if (!m_Description.m_hBlendState.IsInvalidated())
  {
    const ezGALBlendState* pBlendStateBase = pDevice->GetBlendState(m_Description.m_hBlendState);
    if (pBlendStateBase != nullptr)
    {
      const ezGALBlendStateDX12* pBlendState = static_cast<const ezGALBlendStateDX12*>(pBlendStateBase);
      psoDesc.BlendState = pBlendState->GetBlendDesc();
    }
  }
  else
  {
    // Default blend state: no blending, write all channels
    D3D12_BLEND_DESC defaultBlend = {};
    defaultBlend.AlphaToCoverageEnable = FALSE;
    defaultBlend.IndependentBlendEnable = FALSE;
    for (auto& rt : defaultBlend.RenderTarget)
    {
      rt.BlendEnable = FALSE;
      rt.LogicOpEnable = FALSE;
      rt.SrcBlend = D3D12_BLEND_ONE;
      rt.DestBlend = D3D12_BLEND_ZERO;
      rt.BlendOp = D3D12_BLEND_OP_ADD;
      rt.SrcBlendAlpha = D3D12_BLEND_ONE;
      rt.DestBlendAlpha = D3D12_BLEND_ZERO;
      rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
      rt.LogicOp = D3D12_LOGIC_OP_NOOP;
      rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }
    psoDesc.BlendState = defaultBlend;
  }

  // --- Rasterizer state ---

  if (!m_Description.m_hRasterizerState.IsInvalidated())
  {
    const ezGALRasterizerState* pRasterizerStateBase = pDevice->GetRasterizerState(m_Description.m_hRasterizerState);
    if (pRasterizerStateBase != nullptr)
    {
      const ezGALRasterizerStateDX12* pRasterizerState = static_cast<const ezGALRasterizerStateDX12*>(pRasterizerStateBase);
      psoDesc.RasterizerState = pRasterizerState->GetRasterizerDesc();
    }
  }
  else
  {
    // Default rasterizer state: solid fill, back-face culling, clockwise front face
    D3D12_RASTERIZER_DESC defaultRasterizer = {};
    defaultRasterizer.FillMode = D3D12_FILL_MODE_SOLID;
    defaultRasterizer.CullMode = D3D12_CULL_MODE_BACK;
    defaultRasterizer.FrontCounterClockwise = FALSE;
    defaultRasterizer.DepthBias = 0;
    defaultRasterizer.DepthBiasClamp = 0.0f;
    defaultRasterizer.SlopeScaledDepthBias = 0.0f;
    defaultRasterizer.DepthClipEnable = TRUE;
    defaultRasterizer.MultisampleEnable = FALSE;
    defaultRasterizer.AntialiasedLineEnable = FALSE;
    defaultRasterizer.ForcedSampleCount = 0;
    defaultRasterizer.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    psoDesc.RasterizerState = defaultRasterizer;
  }

  // --- Depth stencil state ---

  if (!m_Description.m_hDepthStencilState.IsInvalidated())
  {
    const ezGALDepthStencilState* pDepthStencilStateBase = pDevice->GetDepthStencilState(m_Description.m_hDepthStencilState);
    if (pDepthStencilStateBase != nullptr)
    {
      const ezGALDepthStencilStateDX12* pDepthStencilState = static_cast<const ezGALDepthStencilStateDX12*>(pDepthStencilStateBase);
      psoDesc.DepthStencilState = pDepthStencilState->GetDepthStencilDesc();
    }
  }
  else
  {
    // Default depth stencil state: depth test on, depth write on, less comparison
    D3D12_DEPTH_STENCIL_DESC defaultDepthStencil = {};
    defaultDepthStencil.DepthEnable = TRUE;
    defaultDepthStencil.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    defaultDepthStencil.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    defaultDepthStencil.StencilEnable = FALSE;
    defaultDepthStencil.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    defaultDepthStencil.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    defaultDepthStencil.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    defaultDepthStencil.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    defaultDepthStencil.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    defaultDepthStencil.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    defaultDepthStencil.BackFace = defaultDepthStencil.FrontFace;
    psoDesc.DepthStencilState = defaultDepthStencil;
  }

  // --- Primitive topology type ---

  psoDesc.PrimitiveTopologyType = ezConversionUtilsDX12::ToD3D12PrimitiveTopologyType(m_Description.m_Topology);

  // --- Render target formats from render pass descriptor ---

  const ezGALRenderPassDescriptor& renderPass = m_Description.m_RenderPass;

  psoDesc.NumRenderTargets = renderPass.m_uiRTCount;
  for (ezUInt32 i = 0; i < renderPass.m_uiRTCount; ++i)
  {
    psoDesc.RTVFormats[i] = ezConversionUtilsDX12::ToDXGIFormat(renderPass.m_ColorFormat[i]);
  }

  if (renderPass.m_DepthFormat != ezGALResourceFormat::Invalid)
  {
    psoDesc.DSVFormat = ezConversionUtilsDX12::ToDXGIFormat(renderPass.m_DepthFormat);
  }
  else
  {
    psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
  }

  // --- MSAA ---

  psoDesc.SampleDesc = ezConversionUtilsDX12::ToD3D12SampleDesc(renderPass.m_Msaa);
  psoDesc.SampleMask = UINT_MAX;

  // --- Create PSO ---

  ezLog::Dev("PSO: VS={} PS={} HS={} DS={} GS={}, RT={}, DSV={}, Sample={}, Topo={}, InputElems={}, RootSig={}",
    psoDesc.VS.BytecodeLength, psoDesc.PS.BytecodeLength, psoDesc.HS.BytecodeLength, psoDesc.DS.BytecodeLength, psoDesc.GS.BytecodeLength,
    psoDesc.NumRenderTargets, (int)psoDesc.DSVFormat, psoDesc.SampleDesc.Count, (int)psoDesc.PrimitiveTopologyType,
    psoDesc.InputLayout.NumElements, (void*)psoDesc.pRootSignature);

  HRESULT hr = pDX12Device->GetDXDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState));
  if (FAILED(hr))
  {
    ezLog::Error("Failed to create D3D12 graphics pipeline state object (HRESULT: {0}).", ezArgErrorCode(hr));
    return EZ_FAILURE;
  }

  return EZ_SUCCESS;
}

ezResult ezGALGraphicsPipelineDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  EZ_GAL_DX12_RELEASE(m_pPipelineState);
  return EZ_SUCCESS;
}

void ezGALGraphicsPipelineDX12::SetDebugName(const char* szName)
{
  if (m_pPipelineState != nullptr && szName != nullptr && *szName != '\0')
  {
    ezStringWChar wName(szName);
    m_pPipelineState->SetName(wName.GetData());
  }
}
