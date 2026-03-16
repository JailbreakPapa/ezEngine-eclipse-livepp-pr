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

#include <QTimer>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>

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

    // Add status bar below the 3D viewport
    m_pAnimStatusLabel = new QLabel("No animation playing");
    m_pAnimStatusLabel->setStyleSheet("QLabel { background-color: #2d2d30; color: #dcdcdc; padding: 4px 8px; font-family: monospace; font-size: 11px; }");
    m_pAnimStatusLabel->setFixedHeight(24);
    pContainer->layout()->addWidget(m_pAnimStatusLabel);

    m_pDockManager->addDockWidget(ads::BottomDockWidgetArea, pContainer);
  }

  // Animation Clips panel
  {
    ezQtDocumentPanel* pClipPanel = new ezQtDocumentPanel(GetContainerWindow()->GetDockManager(), this, pDocument);
    pClipPanel->setObjectName("AnimGraphClipPanel");
    pClipPanel->setWindowTitle("Animation Clips");

    m_pClipList = new QListWidget(pClipPanel);
    m_pClipList->setAlternatingRowColors(true);
    connect(m_pClipList, &QListWidget::itemDoubleClicked, this, &ezQtAnimationGraphAssetDocumentWindow::OnClipDoubleClicked);

    pClipPanel->setWidget(m_pClipList);

    m_pDockManager->addDockWidgetTab(ads::RightDockWidgetArea, pClipPanel);

    UpdateClipList();
  }

  GetDocument()->GetSelectionManager()->m_Events.AddEventHandler(ezMakeDelegate(&ezQtAnimationGraphAssetDocumentWindow::SelectionEventHandler, this));

  FinishWindowCreation();

  SelectionEventHandler(ezSelectionManagerEvent());

  // Frame the graph view after nodes have been painted and connections refreshed.
  // The scope filter defers connection refresh, so we defer framing after that.
  QTimer::singleShot(0, [this]()
    {
      QTimer::singleShot(0, [this]()
        {
          if (m_pView)
            m_pView->FrameContent();
        });
    });
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

  if (GetEditorEngineConnection() == nullptr)
    return;

  auto* pDoc = GetAnimGraphDocument();

  {
    ezSimpleDocumentConfigMsgToEngine msg;
    msg.m_sWhatToDo = "PreviewMesh";
    msg.m_sPayload = pDoc->GetProperties()->m_sPreviewMesh;
    GetDocument()->SendMessageToEngine(&msg);
  }

  {
    ezSimpleDocumentConfigMsgToEngine msg;
    msg.m_sWhatToDo = "RenderBones";
    msg.m_PayloadValue = pDoc->GetRenderBones();
    pDoc->SendMessageToEngine(&msg);
  }

  {
    ezSimpleDocumentConfigMsgToEngine msg;
    msg.m_sWhatToDo = "RenderMesh";
    msg.m_PayloadValue = pDoc->GetRenderPreviewMesh();
    pDoc->SendMessageToEngine(&msg);
  }

  {
    ezSimpleDocumentConfigMsgToEngine msg;
    msg.m_sWhatToDo = "SimulationSpeed";

    if (pDoc->GetCommonAssetUiState(ezCommonAssetUiState::Pause) != 0.0)
      msg.m_PayloadValue = 0.0;
    else
      msg.m_PayloadValue = pDoc->GetCommonAssetUiState(ezCommonAssetUiState::SimulationSpeed);

    GetEditorEngineConnection()->SendMessage(&msg);
  }

  for (auto pView : m_ViewWidgets)
  {
    pView->SetEnablePicking(false);
    pView->UpdateCameraInterpolation();
    pView->SyncToEngine();
  }

  QueryObjectBBox();
}

ezAnimationGraphAssetDocument* ezQtAnimationGraphAssetDocumentWindow::GetAnimGraphDocument()
{
  return static_cast<ezAnimationGraphAssetDocument*>(GetDocument());
}

void ezQtAnimationGraphAssetDocumentWindow::QueryObjectBBox(ezInt32 iPurpose)
{
  ezQuerySelectionBBoxMsgToEngine msg;
  msg.m_uiViewID = 0xFFFFFFFF;
  msg.m_iPurpose = iPurpose;
  GetDocument()->SendMessageToEngine(&msg);
}

void ezQtAnimationGraphAssetDocumentWindow::ProcessMessageEventHandler(const ezEditorEngineDocumentMsg* pMsg)
{
  if (pMsg->GetDynamicRTTI()->IsDerivedFrom<ezQuerySelectionBBoxResultMsgToEditor>())
  {
    const ezQuerySelectionBBoxResultMsgToEditor* pMessage = static_cast<const ezQuerySelectionBBoxResultMsgToEditor*>(pMsg);

    if (pMessage->m_vCenter.IsValid() && pMessage->m_vHalfExtents.IsValid())
    {
      m_pViewWidget->SetOrbitVolume(pMessage->m_vCenter, pMessage->m_vHalfExtents.CompMax(ezVec3(0.1f)));
    }
    else
    {
      QueryObjectBBox(pMessage->m_iPurpose);
    }

    return;
  }

  if (auto pSimpleMsg = ezDynamicCast<const ezSimpleDocumentConfigMsgToEditor*>(pMsg))
  {
    if (pSimpleMsg->m_sWhatToDo == "ActiveNodes")
    {
      UpdateNodeHighlighting(pSimpleMsg->m_sPayload);
      return;
    }
    else if (pSimpleMsg->m_sWhatToDo == "StateMachineState")
    {
      UpdateStateMachineStatus(pSimpleMsg->m_sPayload, pSimpleMsg->m_PayloadValue.ConvertTo<double>());
      return;
    }
  }

  ezQtEngineDocumentWindow::ProcessMessageEventHandler(pMsg);
}

void ezQtAnimationGraphAssetDocumentWindow::CommonAssetUiEventHandler(const ezCommonAssetUiState& e)
{
  ezQtEngineDocumentWindow::CommonAssetUiEventHandler(e);

  if (e.m_State == ezCommonAssetUiState::Restart)
  {
    // Send restart message to the engine to reset the animation controller
    ezSimpleDocumentConfigMsgToEngine msg;
    msg.m_sWhatToDo = "Restart";
    GetDocument()->SendMessageToEngine(&msg);
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

  // Frame the view after node paint and connection refresh have completed.
  // The scope filter defers connection refresh via QTimer::singleShot(0),
  // so we defer framing one more event loop iteration after that.
  if (m_pView)
  {
    QTimer::singleShot(0, [this]()
      {
        QTimer::singleShot(0, [this]()
          {
            if (m_pView)
              m_pView->FrameContent();
          });
      });
  }
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

void ezQtAnimationGraphAssetDocumentWindow::UpdateNodeHighlighting(const ezString& sActiveDebugIndices)
{
  auto* pDoc = GetAnimGraphDocument();
  const auto& mapping = pDoc->GetDebugIndexToGuid();

  if (mapping.IsEmpty())
    return;

  ezSet<ezUuid> activeGuids;

  // Parse comma-separated debug indices
  ezStringBuilder sIndices = sActiveDebugIndices;
  ezHybridArray<ezStringView, 32> parts;
  sIndices.Split(false, parts, ",");

  for (const auto& part : parts)
  {
    ezUInt32 uiIdx = 0;
    if (ezConversionUtils::StringToUInt(part, uiIdx).Succeeded())
    {
      if (uiIdx < mapping.GetCount())
      {
        activeGuids.Insert(mapping[uiIdx]);
      }
    }
  }

  m_pScene->SetNodeActivity(activeGuids);
}

void ezQtAnimationGraphAssetDocumentWindow::UpdateStateMachineStatus(const ezString& sStateName, double fTimeInState)
{
  if (m_pAnimStatusLabel == nullptr)
    return;

  if (sStateName.IsEmpty())
  {
    m_pAnimStatusLabel->setText("No state active");
  }
  else
  {
    m_pAnimStatusLabel->setText(
      QString("State: %1  |  Time: %2s")
        .arg(sStateName.GetData())
        .arg(fTimeInState, 0, 'f', 2));
  }
}

void ezQtAnimationGraphAssetDocumentWindow::UpdateClipList()
{
  if (m_pClipList == nullptr)
    return;

  m_pClipList->clear();

  auto* pDoc = GetAnimGraphDocument();
  const auto& clips = pDoc->GetProperties()->m_AnimationClipMapping;

  for (const auto& clip : clips)
  {
    const char* szName = clip.GetClipName();
    if (ezStringUtils::IsNullOrEmpty(szName))
      continue;

    QListWidgetItem* pItem = new QListWidgetItem(szName, m_pClipList);
    // Store the clip resource path as user data
    ezStringView sClipRes = clip.m_hClip.IsValid() ? clip.m_hClip.GetResourceID() : ezStringView();
    pItem->setData(Qt::UserRole, QString::fromUtf8(sClipRes.GetStartPointer(), (int)sClipRes.GetElementCount()));
    pItem->setToolTip(QString::fromUtf8(sClipRes.GetStartPointer(), (int)sClipRes.GetElementCount()));
  }
}

void ezQtAnimationGraphAssetDocumentWindow::OnClipDoubleClicked(QListWidgetItem* pItem)
{
  if (pItem == nullptr)
    return;

  QString sClipResource = pItem->data(Qt::UserRole).toString();

  ezSimpleDocumentConfigMsgToEngine msg;
  msg.m_sWhatToDo = "PlayClip";
  msg.m_sPayload = sClipResource.toUtf8().data();
  GetDocument()->SendMessageToEngine(&msg);
}
