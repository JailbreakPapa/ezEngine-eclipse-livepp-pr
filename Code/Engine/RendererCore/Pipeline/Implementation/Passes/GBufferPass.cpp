#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/ClusteredDataProvider.h>
#include <RendererCore/Pipeline/Passes/GBufferPass.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/RenderContext/RenderContext.h>

#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/Texture.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezGBufferPass, 1, ezRTTIDefaultAllocator<ezGBufferPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("DepthStencil", m_PinDepthStencil),
    EZ_MEMBER_PROPERTY("GBuffer0", m_PinGBuffer0),
    EZ_MEMBER_PROPERTY("GBuffer1", m_PinGBuffer1),
    EZ_MEMBER_PROPERTY("GBuffer2", m_PinGBuffer2),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Rendering")
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezGBufferPass::ezGBufferPass(const char* szName)
  : ezRenderPipelinePass(szName, true)
{
}

ezGBufferPass::~ezGBufferPass() = default;

bool ezGBufferPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  // DepthStencil is passthrough
  if (!inputs[m_PinDepthStencil.m_uiInputIndex])
  {
    ezLog::Error("No depth stencil input connected to pass '{0}'!", GetName());
    return false;
  }

  outputs[m_PinDepthStencil.m_uiOutputIndex] = *inputs[m_PinDepthStencil.m_uiInputIndex];

  ezUInt32 uiWidth = inputs[m_PinDepthStencil.m_uiInputIndex]->m_uiWidth;
  ezUInt32 uiHeight = inputs[m_PinDepthStencil.m_uiInputIndex]->m_uiHeight;

  // GBuffer0: Albedo.rgb + Metallic (RGBA8)
  auto& gbuffer0 = outputs[m_PinGBuffer0.m_uiOutputIndex];
  gbuffer0.m_uiWidth = uiWidth;
  gbuffer0.m_uiHeight = uiHeight;
  gbuffer0.m_Format = ezGALResourceFormat::RGBAUByteNormalized;
  gbuffer0.m_TextureFlags = ezGALTextureUsageFlags::RenderTarget | ezGALTextureUsageFlags::ShaderResource;

  // GBuffer1: Octahedral Normal + Roughness + Flags (RGB10A2)
  auto& gbuffer1 = outputs[m_PinGBuffer1.m_uiOutputIndex];
  gbuffer1.m_uiWidth = uiWidth;
  gbuffer1.m_uiHeight = uiHeight;
  gbuffer1.m_Format = ezGALResourceFormat::RGB10A2UIntNormalized;
  gbuffer1.m_TextureFlags = ezGALTextureUsageFlags::RenderTarget | ezGALTextureUsageFlags::ShaderResource;

  // GBuffer2: Emission.rgb + AO (RGBA8)
  auto& gbuffer2 = outputs[m_PinGBuffer2.m_uiOutputIndex];
  gbuffer2.m_uiWidth = uiWidth;
  gbuffer2.m_uiHeight = uiHeight;
  gbuffer2.m_Format = ezGALResourceFormat::RGBAUByteNormalized;
  gbuffer2.m_TextureFlags = ezGALTextureUsageFlags::RenderTarget | ezGALTextureUsageFlags::ShaderResource;

  return true;
}

void ezGBufferPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  auto pDepthStencil = inputs[m_PinDepthStencil.m_uiInputIndex];
  auto pGBuffer0 = outputs[m_PinGBuffer0.m_uiOutputIndex];
  auto pGBuffer1 = outputs[m_PinGBuffer1.m_uiOutputIndex];
  auto pGBuffer2 = outputs[m_PinGBuffer2.m_uiOutputIndex];

  if (pDepthStencil == nullptr || pGBuffer0 == nullptr || pGBuffer1 == nullptr || pGBuffer2 == nullptr)
    return;

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  // Set up MRT rendering: 3 GBuffer color targets + depth stencil
  ezGALRenderingSetup renderingSetup;
  renderingSetup.SetColorTarget(0, pDevice->GetDefaultRenderTargetView(pGBuffer0->m_TextureHandle));
  renderingSetup.SetClearColor(0, ezColor(0, 0, 0, 0));
  renderingSetup.SetColorTarget(1, pDevice->GetDefaultRenderTargetView(pGBuffer1->m_TextureHandle));
  renderingSetup.SetClearColor(1, ezColor(0, 0, 0, 0));
  renderingSetup.SetColorTarget(2, pDevice->GetDefaultRenderTargetView(pGBuffer2->m_TextureHandle));
  renderingSetup.SetClearColor(2, ezColor(0, 0, 0, 0));
  renderingSetup.SetDepthStencilTarget(pDevice->GetDefaultRenderTargetView(pDepthStencil->m_TextureHandle));

  // Set GBuffer permutation
  renderViewContext.m_pRenderContext->SetShaderPermutationVariable("RENDER_PASS", "RENDER_PASS_GBUFFER");
  renderViewContext.m_pRenderContext->SetShaderPermutationVariable("SHADING_QUALITY", "SHADING_QUALITY_NORMAL");

  // Bind clustered lighting data (needed for decals in GBuffer fill and for shadow/light data structures)
  auto pClusteredData = GetPipeline()->GetFrameDataProvider<ezClusteredDataProvider>()->GetData(renderViewContext);
  pClusteredData->BindResources(renderViewContext.m_pRenderContext);

  renderViewContext.m_pRenderContext->BeginRendering(std::move(renderingSetup), renderViewContext.m_pViewData->m_ViewPortRect, "GBuffer Fill", renderViewContext.m_pCamera->IsStereoscopic());

  // Render all opaque and masked objects
  RenderDataWithCategory(renderViewContext, ezDefaultRenderDataCategories::LitOpaqueStatic);
  RenderDataWithCategory(renderViewContext, ezDefaultRenderDataCategories::LitOpaqueDynamic);
  RenderDataWithCategory(renderViewContext, ezDefaultRenderDataCategories::LitMaskedStatic);
  RenderDataWithCategory(renderViewContext, ezDefaultRenderDataCategories::LitMaskedDynamic);

  renderViewContext.m_pRenderContext->EndRendering();
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_GBufferPass);
