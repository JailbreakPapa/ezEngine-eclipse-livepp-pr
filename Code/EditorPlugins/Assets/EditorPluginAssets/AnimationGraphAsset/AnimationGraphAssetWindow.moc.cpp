#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorFramework/IPC/EngineProcessConnection.h>
#include <EditorFramework/Assets/AssetStatusIndicator.moc.h>
#include <EditorFramework/DocumentWindow/EngineViewWidget.moc.h>
#include <EditorFramework/DocumentWindow/OrbitCamViewWidget.moc.h>
#include <EditorFramework/InputContexts/EditorInputContext.h>
#include <EditorPluginAssets/AnimationGraphAsset/AnimGraphParametersPanel.moc.h>
#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphAsset.h>
#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphAssetScene.moc.h>
#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphAssetWindow.moc.h>
#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphBreadcrumb.moc.h>
#include <GuiFoundation/ActionViews/MenuBarActionMapView.moc.h>
#include <GuiFoundation/ActionViews/ToolBarActionMapView.moc.h>
#include <GuiFoundation/DockPanels/DocumentPanel.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyGridWidget.moc.h>
#include <GuiFoundation/VisualGraph/View.moc.h>

#include <QVBoxLayout>

ezQtAnimationGraphAssetDocumentWindow::ezQtAnimationGraphAssetDocumentWindow(ezAssetDocument* pDocument)
  : ezQtEngineDocumentWindow(pDocument)
{
  SetTargetFramerate(25);

  // Menu Bar
  {
    ezQtMenuBarActionMapView* pMenuBar = static_cast<ezQtMenuBarActionMapView*>(menuBar());
    ezActionContext context;
    context.m_sMapping = "AnimationGraphAssetMenuBar";
    context.m_pDocument = pDocument;
    context.m_pWindow = this;
    pMenuBar->SetActionContext(context);
  }

  // Tool Bar
  {
    ezQtToolBarActionMapView* pToolBar = new ezQtToolBarActionMapView("Toolbar", this);
    ezActionContext context;
    context.m_sMapping = "AnimationGraphAssetToolBar";
    context.m_pDocument = pDocument;
    context.m_pWindow = this;
    pToolBar->SetActionContext(context);
    pToolBar->setObjectName("AnimationGraphAssetWindowToolBar");
    addToolBar(pToolBar);
  }

  // Central Widget: breadcrumb + graph view
  {
    m_pScene = new ezQtAnimationGraphAssetScene(this);
    m_pScene->InitScene(static_cast<const ezVisualGraphObjectManager*>(pDocument->GetObjectManager()));

    m_pView = new ezQtVisualGraphView(this);
    m_pView->SetScene(m_pScene);

    m_pBreadcrumb = new ezQtAnimGraphBreadcrumb(this);

    // Container widget with vertical layout: breadcrumb on top, graph view below
    QWidget* pCentralContainer = new QWidget(this);
    QVBoxLayout* pLayout = new QVBoxLayout(pCentralContainer);
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->setSpacing(0);
    pLayout->addWidget(m_pBreadcrumb);
    pLayout->addWidget(m_pView, 1);
    pCentralContainer->setLayout(pLayout);

    ezQtDocumentPanel* pCentral = new ezQtDocumentPanel(GetContainerWindow()->GetDockManager(), this, pDocument);
    pCentral->setObjectName("ezQtDocumentPanel");
    pCentral->setWindowTitle("Anim Graph");
    pCentral->setWidget(pCentralContainer);

    m_pDockManager->setCentralWidget(pCentral);

    // Wire breadcrumb navigation
    connect(m_pBreadcrumb, &ezQtAnimGraphBreadcrumb::NavigateToScope, this, &ezQtAnimationGraphAssetDocumentWindow::OnNavigateToScope);
    connect(m_pScene, &ezQtAnimationGraphAssetScene::ScopeChanged, this, &ezQtAnimationGraphAssetDocumentWindow::OnScopeChanged);

    // Initialize breadcrumb with root level
    m_pBreadcrumb->UpdateFromNodeManager(
      static_cast<const ezAnimationGraphNodeManager*>(pDocument->GetObjectManager()));
  }

  // Properties panel
  {
    ezQtDocumentPanel* pPropertyPanel = new ezQtDocumentPanel(GetContainerWindow()->GetDockManager(), this, pDocument);
    pPropertyPanel->setObjectName("AnimationGraphAssetDockWidget");
    pPropertyPanel->setWindowTitle("Properties");
    pPropertyPanel->show();

    ezQtPropertyGridWidget* pPropertyGrid = new ezQtPropertyGridWidget(pPropertyPanel, pDocument);

    QWidget* pWidget = new QWidget();
    pWidget->setObjectName("Group");
    pWidget->setLayout(new QVBoxLayout());
    pWidget->setContentsMargins(0, 0, 0, 0);

    pWidget->layout()->setContentsMargins(0, 0, 0, 0);
    pWidget->layout()->addWidget(new ezQtAssetStatusIndicator(pDocument));
    pWidget->layout()->addWidget(pPropertyGrid);

    pPropertyPanel->setWidget(pWidget, ads::CDockWidget::ForceNoScrollArea);

    m_pDockManager->addDockWidgetTab(ads::RightDockWidgetArea, pPropertyPanel);
  }

  // Parameters panel
  {
    ezQtDocumentPanel* pParamsPanel = new ezQtDocumentPanel(GetContainerWindow()->GetDockManager(), this, pDocument);
    pParamsPanel->setObjectName("AnimGraphParametersDockWidget");
    pParamsPanel->setWindowTitle("Parameters");

    m_pParametersPanel = new ezQtAnimGraphParametersPanel(
      pParamsPanel, static_cast<const ezAnimationGraphNodeManager*>(pDocument->GetObjectManager()));

    pParamsPanel->setWidget(m_pParametersPanel);

    m_pDockManager->addDockWidgetTab(ads::RightDockWidgetArea, pParamsPanel);
  }

  // 3D Preview viewport
  {
    m_ViewConfig.m_Camera.LookAt(ezVec3(-1.6f, 0, 0), ezVec3(0, 0, 0), ezVec3(0, 0, 1));
    m_ViewConfig.ApplyPerspectiveSetting(90);

    m_pViewWidget = new ezQtOrbitCamViewWidget(this, &m_ViewConfig);
    m_pViewWidget->ConfigureRelative(ezVec3(0), ezVec3(5.0f), ezVec3(-2, 0, 0.5f), 1.0f);
    AddViewWidget(m_pViewWidget);

    ezQtViewWidgetContainer* pContainer = new ezQtViewWidgetContainer(
      GetContainerWindow()->GetDockManager(), this, m_pViewWidget, "AnimationGraphAssetViewToolBar");
    pContainer->setObjectName("AnimGraphPreviewDockWidget");

    m_pDockManager->addDockWidget(ads::BottomDockWidgetArea, pContainer);
  }

  GetDocument()->GetSelectionManager()->m_Events.AddEventHandler(ezMakeDelegate(&ezQtAnimationGraphAssetDocumentWindow::SelectionEventHandler, this));

  FinishWindowCreation();

  SelectionEventHandler(ezSelectionManagerEvent());
}

ezQtAnimationGraphAssetDocumentWindow::~ezQtAnimationGraphAssetDocumentWindow()
{
  if (GetDocument() != nullptr)
  {
    GetDocument()->GetSelectionManager()->m_Events.RemoveEventHandler(ezMakeDelegate(&ezQtAnimationGraphAssetDocumentWindow::SelectionEventHandler, this));
  }
}

void ezQtAnimationGraphAssetDocumentWindow::InternalRedraw()
{
  ezEditorInputContext::UpdateActiveInputContext();
  SendRedrawMsg();
  ezQtEngineDocumentWindow::InternalRedraw();
}

void ezQtAnimationGraphAssetDocumentWindow::SendRedrawMsg()
{
  if (ezEditorEngineProcessConnection::GetSingleton()->IsProcessCrashed())
    return;

  for (auto pView : m_ViewWidgets)
  {
    pView->SetEnablePicking(false);
    pView->UpdateCameraInterpolation();
    pView->SyncToEngine();
  }
}

void ezQtAnimationGraphAssetDocumentWindow::OnNavigateToScope(const ezUuid& scopeGuid)
{
  m_pScene->NavigateToScope(scopeGuid);
}

void ezQtAnimationGraphAssetDocumentWindow::OnScopeChanged(const ezUuid& newScope)
{
  m_pBreadcrumb->UpdateFromNodeManager(
    static_cast<const ezAnimationGraphNodeManager*>(GetDocument()->GetObjectManager()));
}

void ezQtAnimationGraphAssetDocumentWindow::SelectionEventHandler(const ezSelectionManagerEvent& e)
{
  if (GetDocument()->GetSelectionManager()->IsSelectionEmpty())
  {
    // delayed execution
    QTimer::singleShot(1, [this]()
      {
      // Check again if the selection is empty. This could have changed due to the delayed execution.
      if (GetDocument()->GetSelectionManager()->IsSelectionEmpty())
      {
        GetDocument()->GetSelectionManager()->SetSelection(((ezAnimationGraphAssetDocument*)GetDocument())->GetPropertyObject());
      } });
  }
}
