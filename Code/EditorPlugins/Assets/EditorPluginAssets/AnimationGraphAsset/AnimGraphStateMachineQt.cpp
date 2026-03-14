#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorPluginAssets/AnimationGraphAsset/AnimGraphStateMachineQt.h>
#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphAsset.h>
#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphAssetScene.moc.h>
#include <Foundation/Math/ColorScheme.h>
#include <GuiFoundation/VisualGraph/Scene.moc.h>
#include <SharedPluginAssets/AnimationGraphAsset/AnimGraphStateMachineTypes.h>

#include <QMenu>
#include <QPainter>

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(EditorPluginAssets, AnimGraphStateMachine)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Core"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    ezQtVisualGraphScene::GetNodeFactory().RegisterCreator(
      ezGetStaticRTTI<ezAnimGraphStateNode>(),
      [](const ezRTTI* pRtti) -> ezQtVisualGraphNode* { return new ezQtAnimGraphStateNode(); });

    ezQtVisualGraphScene::GetNodeFactory().RegisterCreator(
      ezGetStaticRTTI<ezAnimGraphAnyStateNode>(),
      [](const ezRTTI* pRtti) -> ezQtVisualGraphNode* { return new ezQtAnimGraphAnyStateNode(); });

    ezQtVisualGraphScene::GetPinFactory().RegisterCreator(
      ezGetStaticRTTI<ezAnimationGraphNodePin>(),
      [](const ezRTTI* pRtti) -> ezQtVisualGraphPin* { return new ezQtAnimGraphStatePin(); });

    ezQtVisualGraphScene::GetConnectionFactory().RegisterCreator(
      ezGetStaticRTTI<ezAnimGraphTransitionConnection>(),
      [](const ezRTTI* pRtti) -> ezQtVisualGraphConnection* { return new ezQtAnimGraphTransitionConnectionQt(); });
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    ezQtVisualGraphScene::GetNodeFactory().UnregisterCreator(ezGetStaticRTTI<ezAnimGraphStateNode>());
    ezQtVisualGraphScene::GetNodeFactory().UnregisterCreator(ezGetStaticRTTI<ezAnimGraphAnyStateNode>());
    ezQtVisualGraphScene::GetPinFactory().UnregisterCreator(ezGetStaticRTTI<ezAnimationGraphNodePin>());
    ezQtVisualGraphScene::GetConnectionFactory().UnregisterCreator(ezGetStaticRTTI<ezAnimGraphTransitionConnection>());
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

//////////////////////////////////////////////////////////////////////////
// ezQtAnimGraphStateNode

ezQtAnimGraphStateNode::ezQtAnimGraphStateNode() = default;

void ezQtAnimGraphStateNode::InitNode(const ezVisualGraphObjectManager* pManager, const ezDocumentObject* pObject)
{
  ezQtVisualGraphNode::InitNode(pManager, pObject);
  UpdateHeaderColor();
}

void ezQtAnimGraphStateNode::UpdateHeaderColor()
{
  m_bIsInitialState = false;

  if (GetObject())
  {
    ezVariant val = GetObject()->GetTypeAccessor().GetValue("IsInitialState");
    if (val.IsA<bool>())
      m_bIsInitialState = val.Get<bool>();
  }

  if (m_bIsInitialState)
    m_HeaderColor = ezToQtColor(ezColorScheme::DarkUI(ezColorScheme::Teal));
  else
    m_HeaderColor = ezToQtColor(ezColorScheme::DarkUI(ezColorScheme::Blue));

  update();
}

void ezQtAnimGraphStateNode::UpdateGeometry()
{
  prepareGeometryChange();

  ezQtVisualGraphNode::UpdateGeometry();
}

void ezQtAnimGraphStateNode::UpdateState()
{
  UpdateHeaderColor();

  ezString sName;
  if (GetObject())
  {
    ezVariant val = GetObject()->GetTypeAccessor().GetValue("Name");
    if (val.IsA<ezString>())
      sName = val.Get<ezString>();
  }

  if (sName.IsEmpty())
    sName = "State";

  if (m_bIsInitialState)
  {
    ezStringBuilder sb = sName;
    sb.Append(" [Initial]");
    m_pTitleLabel->setPlainText(sb.GetData());
  }
  else
  {
    m_pTitleLabel->setPlainText(sName.GetData());
  }
}

void ezQtAnimGraphStateNode::ExtendContextMenu(QMenu& ref_menu)
{
  if (!m_bIsInitialState)
  {
    QAction* pAction = ref_menu.addAction("Set as Initial State");
    pAction->setEnabled(true);
    pAction->connect(pAction, &QAction::triggered,
      [this]()
      {
        auto* pScene = static_cast<ezQtAnimationGraphAssetScene*>(scene());
        pScene->SetInitialState(this);
      });
  }

  QAction* pEnter = ref_menu.addAction("Enter State");
  pEnter->setEnabled(true);
  pEnter->connect(pEnter, &QAction::triggered,
    [this]()
    {
      auto* pScene = static_cast<ezQtAnimationGraphAssetScene*>(scene());
      pScene->NavigateToScope(GetObject()->GetGuid());
    });
}

void ezQtAnimGraphStateNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  ezQtVisualGraphNode::paint(painter, option, widget);
}

//////////////////////////////////////////////////////////////////////////
// ezQtAnimGraphAnyStateNode

ezQtAnimGraphAnyStateNode::ezQtAnimGraphAnyStateNode() = default;

void ezQtAnimGraphAnyStateNode::InitNode(const ezVisualGraphObjectManager* pManager, const ezDocumentObject* pObject)
{
  ezQtVisualGraphNode::InitNode(pManager, pObject);
  m_HeaderColor = ezToQtColor(ezColorScheme::DarkUI(ezColorScheme::Orange));
}

void ezQtAnimGraphAnyStateNode::UpdateGeometry()
{
  prepareGeometryChange();
  ezQtVisualGraphNode::UpdateGeometry();
}

void ezQtAnimGraphAnyStateNode::UpdateState()
{
  m_pTitleLabel->setPlainText("Any State");
}

void ezQtAnimGraphAnyStateNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  ezQtVisualGraphNode::paint(painter, option, widget);
}

//////////////////////////////////////////////////////////////////////////
// ezQtAnimGraphStatePin

ezQtAnimGraphStatePin::ezQtAnimGraphStatePin() = default;

void ezQtAnimGraphStatePin::SetPin(const ezVisualGraphPin& pin)
{
  ezQtVisualGraphPin::SetPin(pin);

  // State pins use a simplified label
  if (pin.GetType() == ezVisualGraphPin::Type::Input)
  {
    m_pLabel->setPlainText("");
  }
  else
  {
    m_pLabel->setPlainText("     +     ");
    m_pLabel->setToolTip("Add Transition");
  }
}

//////////////////////////////////////////////////////////////////////////
// ezQtAnimGraphTransitionConnectionQt

ezQtAnimGraphTransitionConnectionQt::ezQtAnimGraphTransitionConnectionQt()
{
  setFlag(QGraphicsItem::ItemIsSelectable);
}
