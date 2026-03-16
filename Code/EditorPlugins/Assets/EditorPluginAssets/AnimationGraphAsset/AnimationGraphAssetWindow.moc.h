#pragma once

#include <EditorEngineProcessFramework/EngineProcess/ViewRenderSettings.h>
#include <EditorFramework/DocumentWindow/EngineDocumentWindow.moc.h>
#include <Foundation/Basics.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Types/Uuid.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class ezAnimationGraphAssetDocument;
class ezQtAnimationGraphAssetScene;
class ezQtAnimGraphBreadcrumb;
class ezQtAnimGraphParametersPanel;
class ezQtOrbitCamViewWidget;
class ezQtVisualGraphView;
class QLabel;
class QListWidget;
class QListWidgetItem;

class ezQtAnimationGraphAssetDocumentWindow : public ezQtEngineDocumentWindow
{
  Q_OBJECT

public:
  ezQtAnimationGraphAssetDocumentWindow(ezAssetDocument* pDocument);
  ~ezQtAnimationGraphAssetDocumentWindow();

protected:
  virtual void InternalRedraw() override;
  virtual void ProcessMessageEventHandler(const ezEditorEngineDocumentMsg* pMsg) override;
  virtual void CommonAssetUiEventHandler(const ezCommonAssetUiState& e) override;

private Q_SLOTS:
  void OnNavigateToScope(const ezUuid& scopeGuid);
  void OnScopeChanged(const ezUuid& newScope);
  void OnClipDoubleClicked(QListWidgetItem* pItem);

private:
  ezAnimationGraphAssetDocument* GetAnimGraphDocument();
  void SendRedrawMsg();
  void QueryObjectBBox(ezInt32 iPurpose = 0);
  void SelectionEventHandler(const ezSelectionManagerEvent& e);
  void UpdateNodeHighlighting(const ezString& sActiveDebugIndices);
  void UpdateStateMachineStatus(const ezString& sStateName, double fTimeInState);
  void UpdateClipList();

  ezQtAnimationGraphAssetScene* m_pScene = nullptr;
  ezQtVisualGraphView* m_pView = nullptr;
  ezQtAnimGraphBreadcrumb* m_pBreadcrumb = nullptr;
  ezQtAnimGraphParametersPanel* m_pParametersPanel = nullptr;

  ezEngineViewConfig m_ViewConfig;
  ezQtOrbitCamViewWidget* m_pViewWidget = nullptr;

  // Status bar for state machine info
  QLabel* m_pAnimStatusLabel = nullptr;

  // Clip list panel
  QListWidget* m_pClipList = nullptr;
};
