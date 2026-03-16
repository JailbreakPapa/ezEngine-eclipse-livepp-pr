#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorFramework/Assets/AssetDocument.h>
#include <EditorPluginAssets/AnimationGraphAsset/AnimGraphParametersPanel.moc.h>
#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphAsset.h>
#include <EditorEngineProcessFramework/EngineProcess/EngineProcessMessages.h>
#include <Foundation/Reflection/Reflection.h>
#include <RendererCore/AnimationSystem/AnimGraph/Nodes/Blackboard/BlackboardAnimNodes.h>

#include <QCheckBox>
#include <QDoubleSpinBox>
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
  m_pTree->setHeaderLabels({"Name", "Type", "Value", "Refs"});
  m_pTree->setRootIsDecorated(false);
  m_pTree->setAlternatingRowColors(true);
  m_pTree->header()->setStretchLastSection(false);
  m_pTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
  m_pTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  m_pTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  m_pTree->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

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

void ezQtAnimGraphParametersPanel::SendBlackboardValue(const QString& name, const QString& type, const QString& value)
{
  ezSimpleDocumentConfigMsgToEngine msg;
  msg.m_sWhatToDo = "SetBlackboardValue";

  // Format: "name|type|value"
  ezStringBuilder payload;
  payload.SetFormat("{}|{}|{}", name.toUtf8().data(), type.toUtf8().data(), value.toUtf8().data());
  msg.m_sPayload = payload;

  auto* pAssetDoc = static_cast<ezAssetDocument*>(const_cast<ezDocument*>(m_pManager->GetDocument()));
  pAssetDoc->SendMessageToEngine(&msg);
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

  // Build registration payload and populate tree with type-aware widgets
  ezStringBuilder registrationPayload;
  bool bFirst = true;

  for (auto it = parameters.GetIterator(); it.IsValid(); ++it)
  {
    QTreeWidgetItem* pItem = new QTreeWidgetItem(m_pTree);
    pItem->setText(0, it.Key().GetData());
    pItem->setText(1, it.Value().m_sType.GetData());

    // Embed a type-aware editor widget in the Value column
    const QString sName = QString::fromUtf8(it.Key().GetData());
    const QString sType = QString::fromUtf8(it.Value().m_sType.GetData());

    if (it.Value().m_sType == "Bool")
    {
      QCheckBox* pCheckBox = new QCheckBox(m_pTree);
      pCheckBox->setChecked(false);
      m_pTree->setItemWidget(pItem, 2, pCheckBox);

      connect(pCheckBox, &QCheckBox::toggled, this, [this, sName, sType](bool bChecked)
        { SendBlackboardValue(sName, sType, bChecked ? "true" : "false"); });
    }
    else // Number or Any
    {
      QDoubleSpinBox* pSpinBox = new QDoubleSpinBox(m_pTree);
      pSpinBox->setDecimals(3);
      pSpinBox->setRange(-1000000.0, 1000000.0);
      pSpinBox->setSingleStep(0.1);
      pSpinBox->setValue(0.0);
      m_pTree->setItemWidget(pItem, 2, pSpinBox);

      connect(pSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
        [this, sName, sType](double fValue)
        { SendBlackboardValue(sName, sType, QString::number(fValue)); });
    }

    pItem->setText(3, QString::number(it.Value().m_uiRefCount));
    pItem->setTextAlignment(3, Qt::AlignCenter);

    if (!bFirst)
      registrationPayload.Append(";");
    registrationPayload.AppendFormat("{}|{}", it.Key(), it.Value().m_sType);
    bFirst = false;
  }

  // Send all parameters to engine to pre-register on blackboard
  if (!registrationPayload.IsEmpty())
  {
    ezSimpleDocumentConfigMsgToEngine msg;
    msg.m_sWhatToDo = "RegisterBlackboardParams";
    msg.m_sPayload = registrationPayload;

    auto* pAssetDoc = static_cast<ezAssetDocument*>(const_cast<ezDocument*>(m_pManager->GetDocument()));
    pAssetDoc->SendMessageToEngine(&msg);
  }
}
