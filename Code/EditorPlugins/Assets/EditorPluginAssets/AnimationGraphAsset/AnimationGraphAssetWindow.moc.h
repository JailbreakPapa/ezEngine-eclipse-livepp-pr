#pragma once

#include <EditorEngineProcessFramework/EngineProcess/ViewRenderSettings.h>
#include <EditorFramework/DocumentWindow/EngineDocumentWindow.moc.h>
#include <Foundation/Basics.h>
#include <Foundation/Types/Uuid.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class ezQtAnimationGraphAssetScene;
class ezQtAnimGraphBreadcrumb;
class ezQtAnimGraphParametersPanel;
class ezQtOrbitCamViewWidget;
class ezQtVisualGraphView;

class ezQtAnimationGraphAssetDocumentWindow : public ezQtEngineDocumentWindow
{
  Q_OBJECT

public:
  ezQtAnimationGraphAssetDocumentWindow(ezAssetDocument* pDocument);
  ~ezQtAnimationGraphAssetDocumentWindow();

protected:
  virtual void InternalRedraw() override;

private Q_SLOTS:
  void OnNavigateToScope(const ezUuid& scopeGuid);
  void OnScopeChanged(const ezUuid& newScope);

private:
  void SendRedrawMsg();
  void SelectionEventHandler(const ezSelectionManagerEvent& e);

  ezQtAnimationGraphAssetScene* m_pScene = nullptr;
  ezQtVisualGraphView* m_pView = nullptr;
  ezQtAnimGraphBreadcrumb* m_pBreadcrumb = nullptr;
  ezQtAnimGraphParametersPanel* m_pParametersPanel = nullptr;

  ezEngineViewConfig m_ViewConfig;
  ezQtOrbitCamViewWidget* m_pViewWidget = nullptr;
};
