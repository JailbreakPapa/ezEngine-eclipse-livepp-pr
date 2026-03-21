#include <EditorPluginRazor/EditorPluginRazorPCH.h>

#include <EditorFramework/Assets/AssetStatusIndicator.moc.h>
#include <EditorFramework/DocumentWindow/EngineViewWidget.moc.h>
#include <EditorFramework/InputContexts/EditorInputContext.h>
#include <EditorPluginRazor/RazorAsset/RazorAsset.h>
#include <EditorPluginRazor/RazorAsset/RazorAssetWindow.moc.h>
#include <EditorPluginRazor/RazorAsset/RazorHierarchyPanel.moc.h>
#include <EditorPluginRazor/RazorAsset/RazorStylePanel.moc.h>
#include <GuiFoundation/ActionViews/MenuBarActionMapView.moc.h>
#include <GuiFoundation/ActionViews/ToolBarActionMapView.moc.h>
#include <GuiFoundation/ContainerWindow/ContainerWindow.moc.h>
#include <GuiFoundation/DockPanels/DocumentPanel.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyGridWidget.moc.h>
#include <QLayout>

ezQtRazorAssetDocumentWindow::ezQtRazorAssetDocumentWindow(ezAssetDocument* pDocument)
  : ezQtEngineDocumentWindow(pDocument)
{
  // Menu Bar
  {
    ezQtMenuBarActionMapView* pMenuBar = static_cast<ezQtMenuBarActionMapView*>(menuBar());
    ezActionContext context;
    context.m_sMapping = "RazorAssetMenuBar";
    context.m_pDocument = pDocument;
    context.m_pWindow = this;
    pMenuBar->SetActionContext(context);
  }

  // Tool Bar
  {
    ezQtToolBarActionMapView* pToolBar = new ezQtToolBarActionMapView("Toolbar", this);
    ezActionContext context;
    context.m_sMapping = "RazorAssetToolBar";
    context.m_pDocument = pDocument;
    context.m_pWindow = this;
    pToolBar->SetActionContext(context);
    pToolBar->setObjectName("RazorAssetWindowToolBar");
    addToolBar(pToolBar);
  }

  // Preview View
  ezQtViewWidgetContainer* pContainer = nullptr;
  {
    SetTargetFramerate(60);

    m_ViewConfig.m_Camera.LookAt(ezVec3(-1.6f, 0, 0), ezVec3(0, 0, 0), ezVec3(0, 0, 1));
    m_ViewConfig.ApplyPerspectiveSetting(90);

    m_pViewWidget = new ezQtEngineViewWidget(nullptr, this, &m_ViewConfig);
    AddViewWidget(m_pViewWidget);
    pContainer = new ezQtViewWidgetContainer(GetContainerWindow()->GetDockManager(), this, m_pViewWidget, nullptr);
    m_pDockManager->setCentralWidget(pContainer);
  }

  // Hierarchy Panel (Left)
  {
    m_pHierarchyPanel = new ezQtRazorHierarchyPanel(GetContainerWindow()->GetDockManager(), this);
    m_pHierarchyPanel->setObjectName("RazorHierarchyPanel");
    m_pHierarchyPanel->setWindowTitle("Hierarchy");
    m_pHierarchyPanel->show();
    m_pDockManager->addDockWidget(ads::LeftDockWidgetArea, m_pHierarchyPanel);
  }

  // Property Grid Panel (Right)
  {
    ezQtDocumentPanel* pPropertyPanel = new ezQtDocumentPanel(GetContainerWindow()->GetDockManager(), this, pDocument);
    pPropertyPanel->setObjectName("RazorPropertiesPanel");
    pPropertyPanel->setWindowTitle("Properties");
    pPropertyPanel->show();

    ezQtPropertyGridWidget* pPropertyGrid = new ezQtPropertyGridWidget(pPropertyPanel, pDocument);

    QWidget* pWidget = new QWidget();
    pWidget->setObjectName("Group");
    pWidget->setLayout(new QVBoxLayout());
    pWidget->setContentsMargins(0, 0, 0, 0);

    pWidget->layout()->setContentsMargins(0, 0, 0, 0);
    pWidget->layout()->addWidget(new ezQtAssetStatusIndicator(GetDocument()));
    pWidget->layout()->addWidget(pPropertyGrid);

    pPropertyPanel->setWidget(pWidget, ads::CDockWidget::ForceNoScrollArea);
    m_pDockManager->addDockWidget(ads::RightDockWidgetArea, pPropertyPanel);

    pDocument->GetSelectionManager()->SetSelection(pDocument->GetObjectManager()->GetRootObject()->GetChildren()[0]);
  }

  // Style Inspector Panel (Right, tabbed)
  {
    m_pStylePanel = new ezQtRazorStylePanel(GetContainerWindow()->GetDockManager(), this);
    m_pStylePanel->setObjectName("RazorStylePanel");
    m_pStylePanel->setWindowTitle("Styles");
    m_pStylePanel->show();
    m_pDockManager->addDockWidgetTab(ads::RightDockWidgetArea, m_pStylePanel);
  }

  m_pAssetDoc = static_cast<ezRazorDocumentAsset*>(pDocument);

  FinishWindowCreation();

  // Request initial hierarchy after setup
  RequestHierarchyUpdate();
}

ezQtRazorAssetDocumentWindow::~ezQtRazorAssetDocumentWindow() = default;

ezRazorDocumentAsset* ezQtRazorAssetDocumentWindow::GetRazorDocument()
{
  return m_pAssetDoc;
}

void ezQtRazorAssetDocumentWindow::RequestHierarchyUpdate()
{
  ezRazorGetHierarchyMsgToEngine msg;
  msg.m_DocumentGuid = GetDocument()->GetGuid();
  GetEditorEngineConnection()->SendMessage(&msg);
}

void ezQtRazorAssetDocumentWindow::SelectElement(ezUInt32 uiElementId)
{
  m_uiSelectedElementId = uiElementId;

  // Notify engine of selection
  {
    ezRazorSelectElementMsgToEngine msg;
    msg.m_DocumentGuid = GetDocument()->GetGuid();
    msg.m_uiElementId = uiElementId;
    GetEditorEngineConnection()->SendMessage(&msg);
  }

  // Request computed style for selected element
  if (uiElementId != 0)
  {
    ezRazorGetComputedStyleMsgToEngine styleMsg;
    styleMsg.m_DocumentGuid = GetDocument()->GetGuid();
    styleMsg.m_uiElementId = uiElementId;
    GetEditorEngineConnection()->SendMessage(&styleMsg);

    ezRazorGetLayoutMsgToEngine layoutMsg;
    layoutMsg.m_DocumentGuid = GetDocument()->GetGuid();
    layoutMsg.m_uiElementId = uiElementId;
    GetEditorEngineConnection()->SendMessage(&layoutMsg);
  }
}

void ezQtRazorAssetDocumentWindow::InternalRedraw()
{
  ezEditorInputContext::UpdateActiveInputContext();
  SendRedrawMsg();
  ezQtEngineDocumentWindow::InternalRedraw();
}

void ezQtRazorAssetDocumentWindow::ProcessMessageEventHandler(const ezEditorEngineDocumentMsg* pMsg)
{
  if (pMsg->GetDynamicRTTI()->IsDerivedFrom<ezRazorHierarchyMsgToEditor>())
  {
    OnHierarchyReceived(static_cast<const ezRazorHierarchyMsgToEditor*>(pMsg));
  }
  else if (pMsg->GetDynamicRTTI()->IsDerivedFrom<ezRazorComputedStyleMsgToEditor>())
  {
    OnComputedStyleReceived(static_cast<const ezRazorComputedStyleMsgToEditor*>(pMsg));
  }
  else if (pMsg->GetDynamicRTTI()->IsDerivedFrom<ezRazorLayoutMsgToEditor>())
  {
    OnLayoutReceived(static_cast<const ezRazorLayoutMsgToEditor*>(pMsg));
  }
  else if (pMsg->GetDynamicRTTI()->IsDerivedFrom<ezRazorElementSelectedMsgToEditor>())
  {
    OnElementSelectedFromEngine(static_cast<const ezRazorElementSelectedMsgToEditor*>(pMsg));
  }
  else if (pMsg->GetDynamicRTTI()->IsDerivedFrom<ezRazorDiagnosticMsgToEditor>())
  {
    OnDiagnosticReceived(static_cast<const ezRazorDiagnosticMsgToEditor*>(pMsg));
  }
  else
  {
    ezQtEngineDocumentWindow::ProcessMessageEventHandler(pMsg);
  }
}

void ezQtRazorAssetDocumentWindow::OnHotReloadClicked()
{
  ezRazorHotReloadMsgToEngine msg;
  msg.m_DocumentGuid = GetDocument()->GetGuid();
  msg.m_bReloadXml = true;
  msg.m_bReloadCss = true;
  msg.m_bReloadScripts = true;
  GetEditorEngineConnection()->SendMessage(&msg);

  // Request hierarchy refresh after reload
  RequestHierarchyUpdate();
}

void ezQtRazorAssetDocumentWindow::SendRedrawMsg()
{
  if (ezEditorEngineProcessConnection::GetSingleton()->IsProcessCrashed())
    return;

  for (auto pView : m_ViewWidgets)
  {
    pView->SetEnablePicking(true);
    pView->UpdateCameraInterpolation();
    pView->SyncToEngine();
  }
}

void ezQtRazorAssetDocumentWindow::OnHierarchyReceived(const ezRazorHierarchyMsgToEditor* pMsg)
{
  if (m_pHierarchyPanel)
  {
    m_pHierarchyPanel->SetHierarchy(pMsg->m_Elements);
  }
}

void ezQtRazorAssetDocumentWindow::OnComputedStyleReceived(const ezRazorComputedStyleMsgToEditor* pMsg)
{
  if (m_pStylePanel && pMsg->m_uiElementId == m_uiSelectedElementId)
  {
    m_pStylePanel->SetComputedStyle(pMsg->m_Properties);
  }
}

void ezQtRazorAssetDocumentWindow::OnLayoutReceived(const ezRazorLayoutMsgToEditor* pMsg)
{
  if (m_pStylePanel && pMsg->m_Layout.m_uiElementId == m_uiSelectedElementId)
  {
    m_pStylePanel->SetLayoutInfo(pMsg->m_Layout);
  }
}

void ezQtRazorAssetDocumentWindow::OnElementSelectedFromEngine(const ezRazorElementSelectedMsgToEditor* pMsg)
{
  m_uiSelectedElementId = pMsg->m_uiElementId;

  if (m_pHierarchyPanel)
  {
    m_pHierarchyPanel->SetSelectedElement(pMsg->m_uiElementId);
  }

  // Request style/layout info for newly selected element
  if (pMsg->m_uiElementId != 0)
  {
    ezRazorGetComputedStyleMsgToEngine styleMsg;
    styleMsg.m_DocumentGuid = GetDocument()->GetGuid();
    styleMsg.m_uiElementId = pMsg->m_uiElementId;
    GetEditorEngineConnection()->SendMessage(&styleMsg);

    ezRazorGetLayoutMsgToEngine layoutMsg;
    layoutMsg.m_DocumentGuid = GetDocument()->GetGuid();
    layoutMsg.m_uiElementId = pMsg->m_uiElementId;
    GetEditorEngineConnection()->SendMessage(&layoutMsg);
  }
}

void ezQtRazorAssetDocumentWindow::OnDiagnosticReceived(const ezRazorDiagnosticMsgToEditor* pMsg)
{
  // TODO: Display diagnostics in a diagnostics panel or log
  switch (pMsg->m_Severity.GetValue())
  {
    case ezRazorDiagnosticSeverity::Info:
      ezLog::Info("[Razor] {}", pMsg->m_sMessage);
      break;
    case ezRazorDiagnosticSeverity::Warning:
      ezLog::Warning("[Razor] {}", pMsg->m_sMessage);
      break;
    case ezRazorDiagnosticSeverity::Error:
      ezLog::Error("[Razor] {}", pMsg->m_sMessage);
      break;
  }
}

EZ_STATICLINK_FILE(EditorPluginRazor, EditorPluginRazor_RazorAsset_RazorAssetWindow);
