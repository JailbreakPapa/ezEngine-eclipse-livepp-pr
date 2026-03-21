#pragma once

#include <EditorEngineProcessFramework/EngineProcess/ViewRenderSettings.h>
#include <EditorFramework/DocumentWindow/EngineDocumentWindow.moc.h>
#include <EditorPluginRazor/EditorPluginRazorDLL.h>
#include <SharedPluginRazor/Common/Messages.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class ezQtEngineViewWidget;
class ezRazorDocumentAsset;
class ezQtRazorHierarchyPanel;
class ezQtRazorStylePanel;

/// Asset document window for Razor UI documents.
///
/// Provides a preview-centered workflow with hierarchy panel, style inspector,
/// and live preview of the UI document in the engine process.
class ezQtRazorAssetDocumentWindow : public ezQtEngineDocumentWindow
{
  Q_OBJECT

public:
  ezQtRazorAssetDocumentWindow(ezAssetDocument* pDocument);
  ~ezQtRazorAssetDocumentWindow();

  ezRazorDocumentAsset* GetRazorDocument();

  /// Request hierarchy refresh from the engine process.
  void RequestHierarchyUpdate();

  /// Select an element in the preview and update panels.
  void SelectElement(ezUInt32 uiElementId);

  /// Get the currently selected element ID.
  ezUInt32 GetSelectedElementId() const { return m_uiSelectedElementId; }

protected:
  virtual void InternalRedraw() override;
  virtual void ProcessMessageEventHandler(const ezEditorEngineDocumentMsg* pMsg) override;

public Q_SLOTS:
  void OnHotReloadClicked();

private:
  void SendRedrawMsg();
  void OnHierarchyReceived(const ezRazorHierarchyMsgToEditor* pMsg);
  void OnComputedStyleReceived(const ezRazorComputedStyleMsgToEditor* pMsg);
  void OnLayoutReceived(const ezRazorLayoutMsgToEditor* pMsg);
  void OnElementSelectedFromEngine(const ezRazorElementSelectedMsgToEditor* pMsg);
  void OnDiagnosticReceived(const ezRazorDiagnosticMsgToEditor* pMsg);

  ezEngineViewConfig m_ViewConfig;
  ezQtEngineViewWidget* m_pViewWidget = nullptr;
  ezRazorDocumentAsset* m_pAssetDoc = nullptr;

  // Panels
  ezQtRazorHierarchyPanel* m_pHierarchyPanel = nullptr;
  ezQtRazorStylePanel* m_pStylePanel = nullptr;

  ezUInt32 m_uiSelectedElementId = 0;
};
