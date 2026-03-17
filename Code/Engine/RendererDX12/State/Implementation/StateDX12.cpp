#include <RendererDX12/RendererDX12PCH.h>

#include <Foundation/Logging/Log.h>
#include <RendererDX12/Device/DeviceDX12.h>
#include <RendererDX12/Pools/DescriptorHeapPoolDX12.h>
#include <RendererDX12/State/StateDX12.h>
#include <RendererDX12/Utils/ConversionUtilsDX12.h>

// Blend state

ezGALBlendStateDX12::ezGALBlendStateDX12(const ezGALBlendStateCreationDescription& Description)
  : ezGALBlendState(Description)
{
}

ezGALBlendStateDX12::~ezGALBlendStateDX12() = default;

ezResult ezGALBlendStateDX12::InitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  m_BlendDesc.AlphaToCoverageEnable = m_Description.m_bAlphaToCoverage;
  m_BlendDesc.IndependentBlendEnable = m_Description.m_bIndependentBlend;

  for (ezInt32 i = 0; i < EZ_GAL_MAX_RENDERTARGET_COUNT; ++i)
  {
    const auto& src = m_Description.m_RenderTargetBlendDescriptions[i];
    auto& dst = m_BlendDesc.RenderTarget[i];

    dst.BlendEnable = src.m_bBlendingEnabled;
    dst.LogicOpEnable = FALSE;
    dst.SrcBlend = ezConversionUtilsDX12::ToD3D12Blend(src.m_SourceBlend);
    dst.DestBlend = ezConversionUtilsDX12::ToD3D12Blend(src.m_DestBlend);
    dst.BlendOp = ezConversionUtilsDX12::ToD3D12BlendOp(src.m_BlendOp);
    dst.SrcBlendAlpha = ezConversionUtilsDX12::ToD3D12Blend(src.m_SourceBlendAlpha);
    dst.DestBlendAlpha = ezConversionUtilsDX12::ToD3D12Blend(src.m_DestBlendAlpha);
    dst.BlendOpAlpha = ezConversionUtilsDX12::ToD3D12BlendOp(src.m_BlendOpAlpha);
    dst.LogicOp = D3D12_LOGIC_OP_NOOP;
    dst.RenderTargetWriteMask = src.m_uiWriteMask & 0x0F;
  }

  return EZ_SUCCESS;
}

ezResult ezGALBlendStateDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  m_BlendDesc = {};
  return EZ_SUCCESS;
}

// Depth stencil state

ezGALDepthStencilStateDX12::ezGALDepthStencilStateDX12(const ezGALDepthStencilStateCreationDescription& Description)
  : ezGALDepthStencilState(Description)
{
}

ezGALDepthStencilStateDX12::~ezGALDepthStencilStateDX12() = default;

ezResult ezGALDepthStencilStateDX12::InitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  m_DepthStencilDesc.DepthEnable = m_Description.m_bDepthEnable;
  m_DepthStencilDesc.DepthWriteMask = m_Description.m_bDepthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
  m_DepthStencilDesc.DepthFunc = ezConversionUtilsDX12::ToD3D12ComparisonFunc(m_Description.m_DepthTestFunc);

  m_DepthStencilDesc.StencilEnable = m_Description.m_bStencilEnable;
  m_DepthStencilDesc.StencilReadMask = m_Description.m_uiStencilReadMask;
  m_DepthStencilDesc.StencilWriteMask = m_Description.m_uiStencilWriteMask;

  m_DepthStencilDesc.FrontFace.StencilFailOp = ezConversionUtilsDX12::ToD3D12StencilOp(m_Description.m_FrontFaceStencilOp.m_FailOp);
  m_DepthStencilDesc.FrontFace.StencilDepthFailOp = ezConversionUtilsDX12::ToD3D12StencilOp(m_Description.m_FrontFaceStencilOp.m_DepthFailOp);
  m_DepthStencilDesc.FrontFace.StencilPassOp = ezConversionUtilsDX12::ToD3D12StencilOp(m_Description.m_FrontFaceStencilOp.m_PassOp);
  m_DepthStencilDesc.FrontFace.StencilFunc = ezConversionUtilsDX12::ToD3D12ComparisonFunc(m_Description.m_FrontFaceStencilOp.m_StencilFunc);

  const ezGALStencilOpDescription& backFaceStencilOp = m_Description.m_BackFaceStencilOp;
  m_DepthStencilDesc.BackFace.StencilFailOp = ezConversionUtilsDX12::ToD3D12StencilOp(backFaceStencilOp.m_FailOp);
  m_DepthStencilDesc.BackFace.StencilDepthFailOp = ezConversionUtilsDX12::ToD3D12StencilOp(backFaceStencilOp.m_DepthFailOp);
  m_DepthStencilDesc.BackFace.StencilPassOp = ezConversionUtilsDX12::ToD3D12StencilOp(backFaceStencilOp.m_PassOp);
  m_DepthStencilDesc.BackFace.StencilFunc = ezConversionUtilsDX12::ToD3D12ComparisonFunc(backFaceStencilOp.m_StencilFunc);

  return EZ_SUCCESS;
}

ezResult ezGALDepthStencilStateDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  m_DepthStencilDesc = {};
  return EZ_SUCCESS;
}

// Rasterizer state

ezGALRasterizerStateDX12::ezGALRasterizerStateDX12(const ezGALRasterizerStateCreationDescription& Description)
  : ezGALRasterizerState(Description)
{
}

ezGALRasterizerStateDX12::~ezGALRasterizerStateDX12() = default;

ezResult ezGALRasterizerStateDX12::InitPlatform(ezGALDevice* pDevice)
{
  m_RasterizerDesc.FillMode = m_Description.m_bWireFrame ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
  m_RasterizerDesc.CullMode = ezConversionUtilsDX12::ToD3D12CullMode(m_Description.m_CullMode);
  m_RasterizerDesc.FrontCounterClockwise = m_Description.m_bFrontCounterClockwise;
  m_RasterizerDesc.DepthBias = m_Description.m_iDepthBias;
  m_RasterizerDesc.DepthBiasClamp = m_Description.m_fDepthBiasClamp;
  m_RasterizerDesc.SlopeScaledDepthBias = m_Description.m_fSlopeScaledDepthBias;
  m_RasterizerDesc.DepthClipEnable = TRUE;
  m_RasterizerDesc.MultisampleEnable = TRUE;
  m_RasterizerDesc.AntialiasedLineEnable = TRUE;
  m_RasterizerDesc.ForcedSampleCount = 0;
  m_RasterizerDesc.ConservativeRaster =
    m_Description.m_bConservativeRasterization ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

  if (m_Description.m_bConservativeRasterization && !pDevice->GetCapabilities().m_bSupportsConservativeRasterization)
  {
    ezLog::Error("Rasterizer state enables conservative rasterization which is not available on this device.");
    return EZ_FAILURE;
  }

  // D3D12 does not have a ScissorEnable flag in the rasterizer desc. Scissor test is always
  // enabled; the scissor rect simply defaults to the full viewport if not explicitly set.
  // The m_bScissorTest flag is still tracked so the command list can decide whether to apply
  // a custom scissor rect.

  return EZ_SUCCESS;
}

ezResult ezGALRasterizerStateDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  m_RasterizerDesc = {};
  return EZ_SUCCESS;
}

// Sampler state

// clang-format off
static const D3D12_FILTER s_GALFilterTableIndexToDX12[16] =
{
  D3D12_FILTER_MIN_MAG_MIP_POINT,
  D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR,
  D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
  D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR,
  D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT,
  D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
  D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
  D3D12_FILTER_MIN_MAG_MIP_LINEAR,
  D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
  D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
  D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
  D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
  D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
  D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
  D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
  D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
};
// clang-format on

ezGALSamplerStateDX12::ezGALSamplerStateDX12(const ezGALSamplerStateCreationDescription& Description)
  : ezGALSamplerState(Description)
{
}

ezGALSamplerStateDX12::~ezGALSamplerStateDX12() = default;

ezResult ezGALSamplerStateDX12::InitPlatform(ezGALDevice* pDevice)
{
  m_SamplerDesc.AddressU = ezConversionUtilsDX12::ToD3D12TextureAddressMode(m_Description.m_AddressU);
  m_SamplerDesc.AddressV = ezConversionUtilsDX12::ToD3D12TextureAddressMode(m_Description.m_AddressV);
  m_SamplerDesc.AddressW = ezConversionUtilsDX12::ToD3D12TextureAddressMode(m_Description.m_AddressW);
  m_SamplerDesc.BorderColor[0] = m_Description.m_BorderColor.r;
  m_SamplerDesc.BorderColor[1] = m_Description.m_BorderColor.g;
  m_SamplerDesc.BorderColor[2] = m_Description.m_BorderColor.b;
  m_SamplerDesc.BorderColor[3] = m_Description.m_BorderColor.a;
  m_SamplerDesc.ComparisonFunc = ezConversionUtilsDX12::ToD3D12ComparisonFunc(m_Description.m_SampleCompareFunc);

  if (m_Description.m_MagFilter == ezGALTextureFilterMode::Anisotropic ||
      m_Description.m_MinFilter == ezGALTextureFilterMode::Anisotropic ||
      m_Description.m_MipFilter == ezGALTextureFilterMode::Anisotropic)
  {
    if (m_Description.m_SampleCompareFunc == ezGALCompareFunc::Never)
      m_SamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
    else
      m_SamplerDesc.Filter = D3D12_FILTER_COMPARISON_ANISOTROPIC;
  }
  else
  {
    ezUInt32 uiTableIndex = 0;

    if (m_Description.m_MipFilter == ezGALTextureFilterMode::Linear)
      uiTableIndex |= 1;

    if (m_Description.m_MagFilter == ezGALTextureFilterMode::Linear)
      uiTableIndex |= 2;

    if (m_Description.m_MinFilter == ezGALTextureFilterMode::Linear)
      uiTableIndex |= 4;

    if (m_Description.m_SampleCompareFunc != ezGALCompareFunc::Never)
      uiTableIndex |= 8;

    m_SamplerDesc.Filter = s_GALFilterTableIndexToDX12[uiTableIndex];
  }

  m_SamplerDesc.MaxAnisotropy = m_Description.m_uiMaxAnisotropy;
  m_SamplerDesc.MaxLOD = m_Description.m_fMaxMip;
  m_SamplerDesc.MinLOD = m_Description.m_fMinMip;
  m_SamplerDesc.MipLODBias = m_Description.m_fMipLodBias;

  // Allocate a staging sampler descriptor so this sampler can be referenced later
  // when building descriptor tables.
  ezGALDeviceDX12* pDX12Device = static_cast<ezGALDeviceDX12*>(pDevice);
  m_SamplerDescriptor = pDX12Device->GetDescriptorHeapPool().AllocateStagingSampler();

  pDX12Device->GetDXDevice()->CreateSampler(&m_SamplerDesc, m_SamplerDescriptor);

  return EZ_SUCCESS;
}

ezResult ezGALSamplerStateDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  m_SamplerDesc = {};
  m_SamplerDescriptor = {};
  return EZ_SUCCESS;
}

void ezGALSamplerStateDX12::SetDebugNamePlatform(const char* szName) const
{
  EZ_IGNORE_UNUSED(szName);
  // D3D12 sampler descriptors are not named objects.
}
