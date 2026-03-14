#include <EnginePluginAssets/EnginePluginAssetsPCH.h>

#include <EnginePluginAssets/AnimationGraphAsset/AnimationGraphContext.h>
#include <EnginePluginAssets/AnimationGraphAsset/AnimationGraphView.h>
#include <RendererCore/RenderWorld/RenderWorld.h>

ezAnimationGraphViewContext::ezAnimationGraphViewContext(ezAnimationGraphContext* pContext)
  : ezEngineProcessViewContext(pContext)
{
  m_pContext = pContext;

  m_Camera.SetCameraMode(ezCameraMode::PerspectiveFixedFovX, 45.0f, 0.1f, 1000.0f);
  m_Camera.LookAt(ezVec3(1, 1, 1), ezVec3::MakeZero(), ezVec3(0.0f, 0.0f, 1.0f));
}

ezAnimationGraphViewContext::~ezAnimationGraphViewContext() = default;

bool ezAnimationGraphViewContext::UpdateThumbnailCamera(const ezBoundingBoxSphere& bounds)
{
  return !FocusCameraOnObject(m_Camera, bounds, 45.0f, -ezVec3(5, -2, 3));
}

ezViewHandle ezAnimationGraphViewContext::CreateView()
{
  ezView* pView = nullptr;
  ezRenderWorld::CreateView("Animation Graph Editor - View", pView);
  pView->SetCameraUsageHint(ezCameraUsageHint::EditorView);

  pView->SetRenderPipelineResource(CreateDefaultRenderPipeline());

  ezEngineProcessDocumentContext* pDocumentContext = GetDocumentContext();
  pView->SetWorld(pDocumentContext->GetWorld());
  pView->SetCamera(&m_Camera);
  return pView->GetHandle();
}

void ezAnimationGraphViewContext::SetCamera(const ezViewRedrawMsgToEngine* pMsg)
{
  if (m_pContext->m_bDisplayGrid)
  {
    ezEngineProcessViewContext::DrawSimpleGrid();
  }

  ezEngineProcessViewContext::SetCamera(pMsg);
}
