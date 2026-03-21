#pragma once

#include <EditorEngineProcessFramework/EngineProcess/EngineProcessDocumentContext.h>
#include <EnginePluginRazor/EnginePluginRazorDLL.h>
#include <RazorPlugin/Resources/RazorDocumentResource.h>
#include <SharedPluginRazor/Common/Messages.h>

class ezRazorCanvas2DComponent;

class EZ_ENGINEPLUGINRAZOR_DLL ezRazorDocumentContext : public ezEngineProcessDocumentContext
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorDocumentContext, ezEngineProcessDocumentContext);

public:
  ezRazorDocumentContext();
  ~ezRazorDocumentContext();

  const ezRazorDocumentResourceHandle& GetResource() const { return m_hMainResource; }

  /// Set element highlight overlay for preview.
  void SetHighlightedElement(ezUInt32 uiElementId);

  /// Set the currently selected element.
  void SetSelectedElement(ezUInt32 uiElementId);

  /// Get the canvas component.
  ezRazorCanvas2DComponent* GetCanvasComponent() const { return m_pCanvasComponent; }

protected:
  virtual void OnInitialize() override;

  virtual ezEngineProcessViewContext* CreateViewContext() override;
  virtual void DestroyViewContext(ezEngineProcessViewContext* pContext) override;
  virtual bool UpdateThumbnailViewContext(ezEngineProcessViewContext* pThumbnailViewContext) override;
  virtual void HandleMessage(const ezEditorEngineDocumentMsg* pMsg) override;

private:
  void HandleGetHierarchy(const ezRazorGetHierarchyMsgToEngine* pMsg);
  void HandleSelectElement(const ezRazorSelectElementMsgToEngine* pMsg);
  void HandleGetComputedStyle(const ezRazorGetComputedStyleMsgToEngine* pMsg);
  void HandleGetLayout(const ezRazorGetLayoutMsgToEngine* pMsg);
  void HandleHighlightElement(const ezRazorHighlightElementMsgToEngine* pMsg);
  void HandleHotReload(const ezRazorHotReloadMsgToEngine* pMsg);

  ezGameObject* m_pMainObject = nullptr;
  ezRazorCanvas2DComponent* m_pCanvasComponent = nullptr;
  ezRazorDocumentResourceHandle m_hMainResource;

  ezUInt32 m_uiSelectedElementId = 0;
  ezUInt32 m_uiHighlightedElementId = 0;
};
