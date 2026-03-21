#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorFramework/Assets/AssetStatusIndicator.moc.h>
#include <EditorPluginAssets/HairStrandAsset/HairStrandAsset.h>
#include <EditorPluginAssets/HairStrandAsset/HairStrandAssetWindow.moc.h>
#include <GuiFoundation/ActionViews/MenuBarActionMapView.moc.h>
#include <GuiFoundation/ActionViews/ToolBarActionMapView.moc.h>
#include <GuiFoundation/DockPanels/DocumentPanel.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyGridWidget.moc.h>

ezQtHairStrandAssetDocumentWindow::ezQtHairStrandAssetDocumentWindow(ezHairStrandAssetDocument* pDocument)
  : ezQtDocumentWindow(pDocument)
{
  // Menu Bar
  {
    ezQtMenuBarActionMapView* pMenuBar = static_cast<ezQtMenuBarActionMapView*>(menuBar());
    ezActionContext context;
    context.m_sMapping = "HairStrandAssetMenuBar";
    context.m_pDocument = pDocument;
    context.m_pWindow = this;
    pMenuBar->SetActionContext(context);
  }

  // Tool Bar
  {
    ezQtToolBarActionMapView* pToolBar = new ezQtToolBarActionMapView("Toolbar", this);
    ezActionContext context;
    context.m_sMapping = "HairStrandAssetToolBar";
    context.m_pDocument = pDocument;
    context.m_pWindow = this;
    pToolBar->SetActionContext(context);
    pToolBar->setObjectName("HairStrandAssetWindowToolBar");
    addToolBar(pToolBar);
  }

  // Property Panel
  {
    ezQtDocumentPanel* pPropertyPanel = new ezQtDocumentPanel(GetContainerWindow()->GetDockManager(), this, pDocument);
    pPropertyPanel->setObjectName("HairStrandAssetDockWidget");
    pPropertyPanel->setWindowTitle("Hair Strand Properties");
    pPropertyPanel->show();

    ezQtPropertyGridWidget* pPropertyGrid = new ezQtPropertyGridWidget(pPropertyPanel, pDocument);

    QWidget* pWidget = new QWidget();
    pWidget->setObjectName("Group");
    pWidget->setLayout(new QVBoxLayout());
    pWidget->setContentsMargins(0, 0, 0, 0);

    pWidget->layout()->setContentsMargins(0, 0, 0, 0);
    pWidget->layout()->addWidget(new ezQtAssetStatusIndicator(static_cast<ezAssetDocument*>(GetDocument())));
    pWidget->layout()->addWidget(pPropertyGrid);

    pPropertyPanel->setWidget(pWidget, ads::CDockWidget::ForceNoScrollArea);

    m_pDockManager->addDockWidgetTab(ads::CenterDockWidgetArea, pPropertyPanel);

    pDocument->GetSelectionManager()->SetSelection(pDocument->GetObjectManager()->GetRootObject()->GetChildren()[0]);
  }

  FinishWindowCreation();
}

ezQtHairStrandAssetDocumentWindow::~ezQtHairStrandAssetDocumentWindow() = default;
