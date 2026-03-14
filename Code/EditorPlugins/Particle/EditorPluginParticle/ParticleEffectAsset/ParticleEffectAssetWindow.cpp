#include <EditorPluginParticle/EditorPluginParticlePCH.h>

#include <EditorFramework/Assets/AssetStatusIndicator.moc.h>
#include <EditorFramework/DocumentWindow/OrbitCamViewWidget.moc.h>
#include <EditorFramework/InputContexts/EditorInputContext.h>
#include <EditorPluginParticle/ParticleEffectAsset/ParticleEffectAssetWindow.moc.h>
#include <EditorPluginParticle/ParticleEffectAsset/ParticleEffectGraphScene.moc.h>
#include <Foundation/Utilities/AssetFileHeader.h>
#include <GuiFoundation/ActionViews/MenuBarActionMapView.moc.h>
#include <GuiFoundation/ActionViews/ToolBarActionMapView.moc.h>
#include <GuiFoundation/DockPanels/DocumentPanel.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyGridWidget.moc.h>
#include <GuiFoundation/VisualGraph/View.moc.h>
#include <QBoxLayout>
#include <QSettings>
#include <SharedPluginAssets/Common/Messages.h>
#include <ToolsFoundation/Command/TreeCommands.h>

ezQtParticleEffectAssetDocumentWindow::ezQtParticleEffectAssetDocumentWindow(ezAssetDocument* pDocument)
  : ezQtEngineDocumentWindow(pDocument)
{
  GetDocument()->GetObjectManager()->m_PropertyEvents.AddEventHandler(ezMakeDelegate(&ezQtParticleEffectAssetDocumentWindow::PropertyEventHandler, this));
  GetDocument()->GetObjectManager()->m_StructureEvents.AddEventHandler(ezMakeDelegate(&ezQtParticleEffectAssetDocumentWindow::StructureEventHandler, this));

  // Menu Bar
  {
    ezQtMenuBarActionMapView* pMenuBar = static_cast<ezQtMenuBarActionMapView*>(menuBar());
    ezActionContext context;
    context.m_sMapping = "ParticleEffectAssetMenuBar";
    context.m_pDocument = pDocument;
    context.m_pWindow = this;
    pMenuBar->SetActionContext(context);
  }

  // Tool Bar
  {
    ezQtToolBarActionMapView* pToolBar = new ezQtToolBarActionMapView("Toolbar", this);
    ezActionContext context;
    context.m_sMapping = "ParticleEffectAssetToolBar";
    context.m_pDocument = pDocument;
    context.m_pWindow = this;
    pToolBar->SetActionContext(context);
    pToolBar->setObjectName("ParticleEffectAssetWindowToolBar");
    addToolBar(pToolBar);
  }

  // 3D View (central widget)
  {
    SetTargetFramerate(25);

    m_ViewConfig.m_Camera.LookAt(ezVec3(-1.6f, 0, 0), ezVec3(0, 0, 0), ezVec3(0, 0, 1));
    m_ViewConfig.ApplyPerspectiveSetting(90);

    m_pViewWidget = new ezQtOrbitCamViewWidget(this, &m_ViewConfig);
    m_pViewWidget->ConfigureRelative(ezVec3(0), ezVec3(5.0f), ezVec3(-2, 0, 0.5f), 1.0f);
    AddViewWidget(m_pViewWidget);
    ezQtViewWidgetContainer* pContainer = new ezQtViewWidgetContainer(GetContainerWindow()->GetDockManager(), this, m_pViewWidget, "ParticleEffectAssetViewToolBar");
    m_pDockManager->setCentralWidget(pContainer);
  }

  // Visual Graph Panel
  {
    m_pScene = new ezQtParticleEffectGraphScene(this);
    m_pScene->InitScene(static_cast<const ezVisualGraphObjectManager*>(pDocument->GetObjectManager()));

    m_pView = new ezQtVisualGraphView(this);
    m_pView->SetScene(m_pScene);

    ezQtDocumentPanel* pGraphPanel = new ezQtDocumentPanel(GetContainerWindow()->GetDockManager(), this, pDocument);
    pGraphPanel->setObjectName("ParticleEffectGraphView");
    pGraphPanel->setWindowTitle("Particle Graph");
    pGraphPanel->setWidget(m_pView);

    m_pDockManager->addDockWidget(ads::BottomDockWidgetArea, pGraphPanel);
  }

  // Property Grid Panel
  {
    ezQtDocumentPanel* pPropertyPanel = new ezQtDocumentPanel(GetContainerWindow()->GetDockManager(), this, pDocument);
    pPropertyPanel->setObjectName("ParticleEffectAssetDockWidget_Properties");
    pPropertyPanel->setWindowTitle("Properties");
    pPropertyPanel->show();

    ezQtPropertyGridWidget* pPropertyGrid = new ezQtPropertyGridWidget(pPropertyPanel, pDocument);

    QWidget* pWidget = new QWidget();
    pWidget->setObjectName("Group");
    pWidget->setLayout(new QVBoxLayout());
    pWidget->setContentsMargins(0, 0, 0, 0);

    pWidget->layout()->setContentsMargins(0, 0, 0, 0);
    pWidget->layout()->addWidget(new ezQtAssetStatusIndicator((ezAssetDocument*)GetDocument()));
    pWidget->layout()->addWidget(pPropertyGrid);

    pPropertyPanel->setWidget(pWidget, ads::CDockWidget::ForceNoScrollArea);

    m_pDockManager->addDockWidgetTab(ads::RightDockWidgetArea, pPropertyPanel);
  }

  m_pAssetDoc = static_cast<ezParticleEffectAssetDocument*>(pDocument);

  // Clear stale window layout from the old particle editor (before visual graph).
  // The base class queues SlotRestoreDocumentLayout which would hide our new panels
  // if the saved state doesn't include them.
  {
    QSettings settings;
    settings.beginGroup("DocumentWindowLayouts");
    QByteArray state = settings.value("ezQtParticleEffectAssetDocumentWindow").toByteArray();
    if (!state.isEmpty() && !state.contains("ParticleEffectGraphView"))
    {
      settings.remove("ezQtParticleEffectAssetDocumentWindow");
    }
    settings.endGroup();
  }

  FinishWindowCreation();

  GetParticleDocument()->m_Events.AddEventHandler(ezMakeDelegate(&ezQtParticleEffectAssetDocumentWindow::ParticleEventHandler, this));
}

ezQtParticleEffectAssetDocumentWindow::~ezQtParticleEffectAssetDocumentWindow()
{
  GetParticleDocument()->m_Events.RemoveEventHandler(ezMakeDelegate(&ezQtParticleEffectAssetDocumentWindow::ParticleEventHandler, this));

  RestoreResource();

  GetDocument()->GetObjectManager()->m_StructureEvents.RemoveEventHandler(ezMakeDelegate(&ezQtParticleEffectAssetDocumentWindow::StructureEventHandler, this));
  GetDocument()->GetObjectManager()->m_PropertyEvents.RemoveEventHandler(ezMakeDelegate(&ezQtParticleEffectAssetDocumentWindow::PropertyEventHandler, this));
}

ezParticleEffectAssetDocument* ezQtParticleEffectAssetDocumentWindow::GetParticleDocument()
{
  return static_cast<ezParticleEffectAssetDocument*>(GetDocument());
}

void ezQtParticleEffectAssetDocumentWindow::SendLiveResourcePreview()
{
  if (ezEditorEngineProcessConnection::GetSingleton()->IsProcessCrashed())
    return;

  ezResourceUpdateMsgToEngine msg;
  msg.m_sResourceType = "Particle Effect";

  ezStringBuilder tmp;
  msg.m_sResourceID = ezConversionUtils::ToString(GetDocument()->GetGuid(), tmp);

  ezContiguousMemoryStreamStorage streamStorage;
  ezMemoryStreamWriter memoryWriter(&streamStorage);

  // Write Path
  ezStringBuilder sAbsFilePath = GetParticleDocument()->GetDocumentPath();
  sAbsFilePath.ChangeFileExtension("ezParticleEffect");

  // Write Header
  memoryWriter << sAbsFilePath;
  const ezUInt64 uiHash = ezAssetCurator::GetSingleton()->GetAssetTransformHash(GetParticleDocument()->GetGuid());
  ezAssetFileHeader AssetHeader;
  AssetHeader.SetFileHashAndVersion(uiHash, GetParticleDocument()->GetAssetTypeVersion());
  AssetHeader.Write(memoryWriter).IgnoreResult();

  // Write Asset Data
  GetParticleDocument()->WriteResource(memoryWriter);
  msg.m_Data = ezArrayPtr<const ezUInt8>(streamStorage.GetData(), streamStorage.GetStorageSize32());

  ezEditorEngineProcessConnection::GetSingleton()->SendMessage(&msg);
}

void ezQtParticleEffectAssetDocumentWindow::PropertyEventHandler(const ezDocumentObjectPropertyEvent& e)
{
  m_bDoLiveResourceUpdate = true;
}

void ezQtParticleEffectAssetDocumentWindow::StructureEventHandler(const ezDocumentObjectStructureEvent& e)
{
  switch (e.m_EventType)
  {
    case ezDocumentObjectStructureEvent::Type::AfterObjectAdded:
    case ezDocumentObjectStructureEvent::Type::AfterObjectMoved2:
    case ezDocumentObjectStructureEvent::Type::AfterObjectRemoved:
      m_bDoLiveResourceUpdate = true;
      break;

    default:
      break;
  }
}

void ezQtParticleEffectAssetDocumentWindow::ParticleEventHandler(const ezParticleEffectAssetEvent& e)
{
  switch (e.m_Type)
  {
    case ezParticleEffectAssetEvent::RestartEffect:
    {
      ezEditorEngineRestartSimulationMsg msg;
      GetEditorEngineConnection()->SendMessage(&msg);
    }
    break;

    case ezParticleEffectAssetEvent::AutoRestartChanged:
    {
      ezEditorEngineLoopAnimationMsg msg;
      msg.m_bLoop = GetParticleDocument()->GetAutoRestart();
      GetEditorEngineConnection()->SendMessage(&msg);
    }
    break;

    default:
      break;
  }
}

void ezQtParticleEffectAssetDocumentWindow::InternalRedraw()
{
  ezEditorInputContext::UpdateActiveInputContext();
  SendRedrawMsg();
  ezQtEngineDocumentWindow::InternalRedraw();
}

void ezQtParticleEffectAssetDocumentWindow::SendRedrawMsg()
{
  if (ezEditorEngineProcessConnection::GetSingleton()->IsProcessCrashed())
    return;

  if (m_bDoLiveResourceUpdate)
  {
    SendLiveResourcePreview();
    m_bDoLiveResourceUpdate = false;
  }

  {
    ezSimulationSettingsMsgToEngine msg;
    msg.m_bSimulateWorld = !GetParticleDocument()->GetSimulationPaused();
    msg.m_fSimulationSpeed = GetParticleDocument()->GetSimulationSpeed();
    GetEditorEngineConnection()->SendMessage(&msg);
  }

  for (auto pView : m_ViewWidgets)
  {
    pView->SetEnablePicking(false);
    pView->UpdateCameraInterpolation();
    pView->SyncToEngine();
  }
}

void ezQtParticleEffectAssetDocumentWindow::RestoreResource()
{
  ezRestoreResourceMsgToEngine msg;
  msg.m_sResourceType = "Particle Effect";

  ezStringBuilder tmp;
  msg.m_sResourceID = ezConversionUtils::ToString(GetDocument()->GetGuid(), tmp);

  ezEditorEngineProcessConnection::GetSingleton()->SendMessage(&msg);
}
