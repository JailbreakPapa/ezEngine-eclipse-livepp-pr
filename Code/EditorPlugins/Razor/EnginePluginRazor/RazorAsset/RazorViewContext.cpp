#include <EnginePluginRazor/EnginePluginRazorPCH.h>

#include <EditorEngineProcessFramework/EngineProcess/EngineProcessMessages.h>
#include <EnginePluginRazor/RazorAsset/RazorDocumentContext.h>
#include <EnginePluginRazor/RazorAsset/RazorViewContext.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderWorld/RenderWorld.h>

ezRazorViewContext::ezRazorViewContext(ezRazorDocumentContext* pContext)
  : ezEngineProcessViewContext(pContext)
  , m_pDocumentContext(pContext)
{
}

ezRazorViewContext::~ezRazorViewContext()
{
  if (!m_hView.IsInvalidated())
  {
    ezRenderWorld::DeleteView(m_hView);
  }
}

void ezRazorViewContext::HandleViewMessage(const ezEditorEngineViewMsg* pMsg)
{
  ezEngineProcessViewContext::HandleViewMessage(pMsg);
}

ezViewHandle ezRazorViewContext::CreateView()
{
  ezView* pView = nullptr;
  m_hView = ezRenderWorld::CreateView("Razor Editor - View", pView);

  pView->SetCameraUsageHint(ezCameraUsageHint::EditorView);
  pView->SetWorld(m_pDocumentContext->GetWorld());
  pView->SetRenderPipelineResource(CreateDefaultRenderPipeline());
  pView->SetCamera(&m_Camera);

  return m_hView;
}

void ezRazorViewContext::SetCamera(const ezViewRedrawMsgToEngine* pMsg)
{
  // For 2D UI, we use an orthographic camera looking at the canvas
  if (m_pDocumentContext->GetWorld() == nullptr)
    return;

  ezEngineProcessViewContext::SetCamera(pMsg);

  // Override with orthographic for UI preview
  const float fWidth = static_cast<float>(pMsg->m_uiWindowWidth);
  const float fHeight = static_cast<float>(pMsg->m_uiWindowHeight);

  if (fWidth > 0 && fHeight > 0)
  {
    // Set up orthographic camera for 2D UI
    m_Camera.SetCameraMode(ezCameraMode::OrthoFixedWidth, fWidth, 0.1f, 1000.0f);
    m_Camera.LookAt(ezVec3(fWidth / 2, fHeight / 2, -100), ezVec3(fWidth / 2, fHeight / 2, 0), ezVec3(0, 1, 0));
  }
}

EZ_STATICLINK_FILE(EnginePluginRazor, EnginePluginRazor_RazorAsset_RazorViewContext);
