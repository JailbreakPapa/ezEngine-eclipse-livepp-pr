#pragma once

#include <EditorEngineProcessFramework/EngineProcess/EngineProcessViewContext.h>
#include <EnginePluginRazor/EnginePluginRazorDLL.h>

class ezRazorDocumentContext;

/// View context for rendering Razor UI document previews.
class EZ_ENGINEPLUGINRAZOR_DLL ezRazorViewContext : public ezEngineProcessViewContext
{
public:
  ezRazorViewContext(ezRazorDocumentContext* pContext);
  ~ezRazorViewContext();

protected:
  virtual void HandleViewMessage(const ezEditorEngineViewMsg* pMsg) override;
  virtual ezViewHandle CreateView() override;
  virtual void SetCamera(const ezViewRedrawMsgToEngine* pMsg) override;

private:
  ezRazorDocumentContext* m_pDocumentContext = nullptr;
  ezViewHandle m_hView;
};
