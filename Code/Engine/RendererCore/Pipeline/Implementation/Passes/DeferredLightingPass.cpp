#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/ClusteredDataProvider.h>
#include <RendererCore/Lights/ScreenSpaceShadowDataProvider.h>
#include <RendererCore/Lights/SSRDataProvider.h>
#include <RendererCore/Pipeline/Passes/DeferredLightingPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Textures/Texture2DResource.h>
#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/Texture.h>

#include <Foundation/Configuration/CVar.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezDeferredLightingPass, 1, ezRTTIDefaultAllocator<ezDeferredLightingPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("GBuffer0", m_PinGBuffer0),
    EZ_MEMBER_PROPERTY("GBuffer1", m_PinGBuffer1),
    EZ_MEMBER_PROPERTY("GBuffer2", m_PinGBuffer2),
    EZ_MEMBER_PROPERTY("DepthStencil", m_PinDepthStencil),
    EZ_MEMBER_PROPERTY("SSAO", m_PinSSAO),
    EZ_MEMBER_PROPERTY("ColorOutput", m_PinColorOutput),
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

ezDeferredLightingPass::ezDeferredLightingPass(const char* szName)
  : ezRenderPipelinePass(szName, true)
{
  m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/DeferredLighting.ezShader");
  m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezDeferredLightingConstants>();
}

ezDeferredLightingPass::~ezDeferredLightingPass()
{
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
}

bool ezDeferredLightingPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  if (!inputs[m_PinGBuffer0.m_uiInputIndex])
  {
    ezLog::Error("No GBuffer0 input connected to '{0}'!", GetName());
    return false;
  }

  if (!inputs[m_PinDepthStencil.m_uiInputIndex])
  {
    ezLog::Error("No depth stencil input connected to '{0}'!", GetName());
    return false;
  }

  // Output is an HDR color buffer at the same resolution as the GBuffer
  auto& colorOut = outputs[m_PinColorOutput.m_uiOutputIndex];
  colorOut.m_uiWidth = inputs[m_PinGBuffer0.m_uiInputIndex]->m_uiWidth;
  colorOut.m_uiHeight = inputs[m_PinGBuffer0.m_uiInputIndex]->m_uiHeight;
  colorOut.m_Format = ezGALResourceFormat::RGBAHalf;
  colorOut.m_TextureFlags = ezGALTextureUsageFlags::RenderTarget | ezGALTextureUsageFlags::ShaderResource;

  return true;
}

void ezDeferredLightingPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  auto pGBuffer0 = inputs[m_PinGBuffer0.m_uiInputIndex];
  auto pGBuffer1 = inputs[m_PinGBuffer1.m_uiInputIndex];
  auto pGBuffer2 = inputs[m_PinGBuffer2.m_uiInputIndex];
  auto pDepthStencil = inputs[m_PinDepthStencil.m_uiInputIndex];
  auto pColorOutput = outputs[m_PinColorOutput.m_uiOutputIndex];

  if (pGBuffer0 == nullptr || pGBuffer1 == nullptr || pGBuffer2 == nullptr ||
      pDepthStencil == nullptr || pColorOutput == nullptr)
    return;

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  // Bind clustered lighting data
  auto pClusteredData = GetPipeline()->GetFrameDataProvider<ezClusteredDataProvider>()->GetData(renderViewContext);
  pClusteredData->BindResources(renderViewContext.m_pRenderContext);

  // Bind SSR data (from previous frame, if available)
  {
    auto pSSRData = GetPipeline()->GetFrameDataProvider<ezSSRDataProvider>()->GetData(renderViewContext);
    if (pSSRData != nullptr && !pSSRData->m_hSSRTexture.IsInvalidated())
    {
      pSSRData->BindResources(renderViewContext.m_pRenderContext);
    }
  }

  // Bind screen-space shadow data (if available)
  {
    auto pSSSProvider = GetPipeline()->GetFrameDataProvider<ezScreenSpaceShadowDataProvider>();
    if (pSSSProvider != nullptr)
    {
      auto pSSSData = pSSSProvider->GetData(renderViewContext);
      if (pSSSData != nullptr)
      {
        pSSSData->BindResources(renderViewContext.m_pRenderContext);
      }
    }
  }

  // Update constant buffer
  {
    const ezGALTexture* pGBuffer0Tex = pDevice->GetTexture(pGBuffer0->m_TextureHandle);
    ezDeferredLightingConstants* cb = ezRenderContext::GetConstantBufferData<ezDeferredLightingConstants>(m_hConstantBuffer);
    cb->InverseGBufferSize.x = 1.0f / (float)pGBuffer0Tex->GetDescription().m_uiWidth;
    cb->InverseGBufferSize.y = 1.0f / (float)pGBuffer0Tex->GetDescription().m_uiHeight;
  }

  // Set up fullscreen render target
  ezGALRenderingSetup renderingSetup;
  renderingSetup.SetColorTarget(0, pDevice->GetDefaultRenderTargetView(pColorOutput->m_TextureHandle));

  auto pCommandEncoder = renderViewContext.m_pRenderContext->BeginRenderingScope(renderViewContext, renderingSetup, "Deferred Lighting", renderViewContext.m_pCamera->IsStereoscopic());

  // Constant buffer is in BG_FRAME (set 0)
  ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
  bindGroup.BindBuffer("ezDeferredLightingConstants", m_hConstantBuffer);

  // GBuffer textures, SceneDepth, and SSAOTexture are in BG_RENDER_PASS (set 1)
  ezBindGroupBuilder& bindGroupRenderPass = renderViewContext.m_pRenderContext->GetBindGroup(EZ_GAL_BIND_GROUP_RENDER_PASS);
  bindGroupRenderPass.BindTexture("GBuffer0Texture", pGBuffer0->m_TextureHandle);
  bindGroupRenderPass.BindTexture("GBuffer1Texture", pGBuffer1->m_TextureHandle);
  bindGroupRenderPass.BindTexture("GBuffer2Texture", pGBuffer2->m_TextureHandle);
  bindGroupRenderPass.BindTexture("SceneDepth", pDepthStencil->m_TextureHandle);

  if (inputs[m_PinSSAO.m_uiInputIndex])
  {
    bindGroupRenderPass.BindTexture("SSAOTexture", inputs[m_PinSSAO.m_uiInputIndex]->m_TextureHandle);
  }

  // Set shadow quality permutation
  {
    extern ezCVarInt cvar_RenderingShadowsQuality;
    if (cvar_RenderingShadowsQuality == 0)
      renderViewContext.m_pRenderContext->SetShaderPermutationVariable("SHADOW_QUALITY", "SHADOW_QUALITY_LOW");
    else if (cvar_RenderingShadowsQuality >= 2)
      renderViewContext.m_pRenderContext->SetShaderPermutationVariable("SHADOW_QUALITY", "SHADOW_QUALITY_HIGH");
    else
      renderViewContext.m_pRenderContext->SetShaderPermutationVariable("SHADOW_QUALITY", "SHADOW_QUALITY_MEDIUM");
  }

  renderViewContext.m_pRenderContext->BindShader(m_hShader);
  renderViewContext.m_pRenderContext->BindNullMeshBuffer(ezGALPrimitiveTopology::Triangles, 1);
  renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_DeferredLightingPass);
