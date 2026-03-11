#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/ClusteredDataProvider.h>
#include <RendererCore/Lights/Implementation/VirtualShadowPool.h>
#include <RendererCore/Pipeline/Passes/VSMPageMarkingPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezVSMPageMarkingPass, 1, ezRTTIDefaultAllocator<ezVSMPageMarkingPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("DepthInput", m_PinDepthInput),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Lighting")
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezVSMPageMarkingPass::ezVSMPageMarkingPass()
  : ezRenderPipelinePass("VSMPageMarkingPass", true)
{
  m_hShaderPageMarking = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/VSMPageMarking.ezShader");
}

ezVSMPageMarkingPass::~ezVSMPageMarkingPass() = default;

bool ezVSMPageMarkingPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  if (!inputs[m_PinDepthInput.m_uiInputIndex])
  {
    ezLog::Error("No depth input connected to '{0}'!", GetName());
    return false;
  }

  return true;
}

void ezVSMPageMarkingPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  if (!ezVirtualShadowPool::IsEnabled())
    return;

  auto pDepthInput = inputs[m_PinDepthInput.m_uiInputIndex];
  if (pDepthInput == nullptr)
    return;

  EZ_PROFILE_SCOPE("VSM Page Marking");

  auto computeScope = renderViewContext.m_pRenderContext->BeginComputeScope(renderViewContext, "VSM PageMark");

  // Bind VSM resources
  ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
  bindGroup.BindBuffer("ezVSMConstants", ezVirtualShadowPool::GetConstantBuffer());
  bindGroup.BindTexture("SceneDepth", pDepthInput->m_TextureHandle);
  bindGroup.BindBuffer("PageRequestBuffer", ezVirtualShadowPool::GetPageRequestBuffer());

  // Bind clustered data for viewport info
  auto pClusteredData = GetPipeline()->GetFrameDataProvider<ezClusteredDataProvider>()->GetData(renderViewContext);
  pClusteredData->BindResources(renderViewContext.m_pRenderContext);

  renderViewContext.m_pRenderContext->BindShader(m_hShaderPageMarking);

  const ezRectFloat& viewport = renderViewContext.m_pViewData->m_ViewPortRect;
  ezUInt32 dispatchX = (static_cast<ezUInt32>(viewport.width) + 7) / 8;
  ezUInt32 dispatchY = (static_cast<ezUInt32>(viewport.height) + 7) / 8;
  renderViewContext.m_pRenderContext->Dispatch(dispatchX, dispatchY, 1).IgnoreResult();
}

void ezVSMPageMarkingPass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_VSMPageMarkingPass);
