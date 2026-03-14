#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorPluginAssets/AnimationGraphAsset/AnimGraphParametersPanel.moc.h>
#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphAsset.h>
#include <Foundation/Reflection/Reflection.h>
#include <RendererCore/AnimationSystem/AnimGraph/Nodes/Blackboard/BlackboardAnimNodes.h>

#include <QHeaderView>
#include <QTreeWidget>
#include <QVBoxLayout>

ezQtAnimGraphParametersPanel::ezQtAnimGraphParametersPanel(QWidget* pParent, const ezAnimationGraphNodeManager* pManager)
  : QWidget(pParent)
  , m_pManager(pManager)
{
  QVBoxLayout* pLayout = new QVBoxLayout(this);
  pLayout->setContentsMargins(0, 0, 0, 0);
  pLayout->setSpacing(0);

  m_pTree = new QTreeWidget(this);
  m_pTree->setHeaderLabels({"Name", "Type", "Refs"});
  m_pTree->setRootIsDecorated(false);
  m_pTree->setAlternatingRowColors(true);
  m_pTree->header()->setStretchLastSection(false);
  m_pTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
  m_pTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  m_pTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

  pLayout->addWidget(m_pTree);
  setLayout(pLayout);

  // Listen for structural changes to rebuild the list
  m_pManager->GetDocument()->GetObjectManager()->m_StructureEvents.AddEventHandler(
    ezMakeDelegate(&ezQtAnimGraphParametersPanel::StructureEventHandler, this));

  RebuildParameterList();
}

ezQtAnimGraphParametersPanel::~ezQtAnimGraphParametersPanel()
{
  m_pManager->GetDocument()->GetObjectManager()->m_StructureEvents.RemoveEventHandler(
    ezMakeDelegate(&ezQtAnimGraphParametersPanel::StructureEventHandler, this));
}

void ezQtAnimGraphParametersPanel::StructureEventHandler(const ezDocumentObjectStructureEvent& e)
{
  // Rebuild on any structural change (node added/removed/moved)
  switch (e.m_EventType)
  {
    case ezDocumentObjectStructureEvent::Type::AfterObjectAdded:
    case ezDocumentObjectStructureEvent::Type::AfterObjectRemoved:
    case ezDocumentObjectStructureEvent::Type::AfterObjectMoved:
      RebuildParameterList();
      break;
    default:
      break;
  }
}

struct ParameterInfo
{
  ezString m_sType;
  ezUInt32 m_uiRefCount = 0;
};

void ezQtAnimGraphParametersPanel::RebuildParameterList()
{
  m_pTree->clear();

  // Collect all blackboard references across all nodes in the document
  ezMap<ezString, ParameterInfo> parameters;

  for (auto pChild : m_pManager->GetRootObject()->GetChildren())
  {
    if (!m_pManager->InternalIsNode(pChild))
      continue;

    const ezRTTI* pType = pChild->GetTypeAccessor().GetType();

    // Check if this node type has a "BlackboardEntry" property
    if (pType->FindPropertyByName("BlackboardEntry") == nullptr)
      continue;

    ezVariant val = pChild->GetTypeAccessor().GetValue("BlackboardEntry");
    if (!val.IsA<ezString>())
      continue;

    ezString sEntry = val.Get<ezString>();
    if (sEntry.IsEmpty())
      continue;

    auto& info = parameters[sEntry];
    info.m_uiRefCount++;

    // Infer type from the node class
    if (pType->IsDerivedFrom<ezSetBlackboardNumberAnimNode>() ||
        pType->IsDerivedFrom<ezGetBlackboardNumberAnimNode>() ||
        pType->IsDerivedFrom<ezCompareBlackboardNumberAnimNode>())
    {
      info.m_sType = "Number";
    }
    else if (pType->IsDerivedFrom<ezSetBlackboardBoolAnimNode>() ||
             pType->IsDerivedFrom<ezGetBlackboardBoolAnimNode>() ||
             pType->IsDerivedFrom<ezCheckBlackboardBoolAnimNode>())
    {
      info.m_sType = "Bool";
    }
    else if (pType->IsDerivedFrom<ezOnBlackboardValueChangedAnimNode>())
    {
      if (info.m_sType.IsEmpty())
        info.m_sType = "Any";
    }
  }

  // Populate the tree widget
  for (auto it = parameters.GetIterator(); it.IsValid(); ++it)
  {
    QTreeWidgetItem* pItem = new QTreeWidgetItem(m_pTree);
    pItem->setText(0, it.Key().GetData());
    pItem->setText(1, it.Value().m_sType.GetData());
    pItem->setText(2, QString::number(it.Value().m_uiRefCount));
    pItem->setTextAlignment(2, Qt::AlignCenter);
  }
}
