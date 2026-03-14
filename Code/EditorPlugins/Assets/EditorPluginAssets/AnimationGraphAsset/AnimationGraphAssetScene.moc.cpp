#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphAsset.h>
#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphAssetScene.moc.h>
#include <GuiFoundation/VisualGraph/Node.h>
#include <SharedPluginAssets/AnimationGraphAsset/AnimGraphStateMachineTypes.h>
#include <ToolsFoundation/CommandHistory/CommandHistory.h>

#include <QGraphicsSceneMouseEvent>

ezQtAnimationGraphAssetScene::ezQtAnimationGraphAssetScene(QObject* pParent)
  : ezQtVisualGraphScene(pParent)
{
}

ezQtAnimationGraphAssetScene::~ezQtAnimationGraphAssetScene() = default;

void ezQtAnimationGraphAssetScene::NavigateToScope(const ezUuid& scopeGuid)
{
  m_CurrentScope = scopeGuid;

  // Update the node manager's view scope
  auto* pManager = const_cast<ezAnimationGraphNodeManager*>(
    static_cast<const ezAnimationGraphNodeManager*>(m_pManager));
  pManager->SetCurrentViewScope(scopeGuid);

  // Update connection rendering style
  UpdateConnectionStyleForScope();

  // TODO: Rebuild the scene to show only nodes in the new scope.
  // This requires extending the base ezQtVisualGraphScene to support scope-filtered
  // node visibility, which is a deeper framework change tracked for follow-up.

  Q_EMIT ScopeChanged(scopeGuid);
}

void ezQtAnimationGraphAssetScene::UpdateConnectionStyleForScope()
{
  auto* pManager = static_cast<const ezAnimationGraphNodeManager*>(m_pManager);

  if (pManager->IsCurrentScopeStateMachine())
  {
    SetConnectionStyle(ezQtVisualGraphScene::ConnectionStyle::StraightLine);
    SetConnectionDecorationFlags(ezQtVisualGraphScene::ConnectionDecorationFlags::DirectionArrows);
  }
  else
  {
    SetConnectionStyle(ezQtVisualGraphScene::ConnectionStyle::BezierCurve);
    SetConnectionDecorationFlags(ezBitflags<ezQtVisualGraphScene::ConnectionDecorationFlags>());
  }
}

void ezQtAnimationGraphAssetScene::SetInitialState(ezQtVisualGraphNode* pNode)
{
  ezCommandHistory* pHistory = GetDocumentNodeManager()->GetDocument()->GetCommandHistory();
  pHistory->StartTransaction("Set Initial State");

  ezSetAnimGraphInitialStateCommand cmd;
  cmd.m_NewInitialStateObject = pNode->GetObject()->GetGuid();
  cmd.m_StateMachineScope = m_CurrentScope;

  ezStatus res = pHistory->AddCommand(cmd);

  if (res.Failed())
    pHistory->CancelTransaction();
  else
    pHistory->FinishTransaction();
}

void ezQtAnimationGraphAssetScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
  if (event->button() != Qt::LeftButton)
  {
    ezQtVisualGraphScene::mouseDoubleClickEvent(event);
    return;
  }

  QGraphicsItem* pItem = itemAt(event->scenePos(), QTransform());

  // Walk up the parent chain to find a Node item
  while (pItem != nullptr && pItem->type() != ezQtVisualGraphScene::Node)
  {
    pItem = pItem->parentItem();
  }

  if (pItem == nullptr || pItem->type() != ezQtVisualGraphScene::Node)
  {
    ezQtVisualGraphScene::mouseDoubleClickEvent(event);
    return;
  }

  auto* pNodeItem = static_cast<ezQtVisualGraphNode*>(pItem);
  const ezDocumentObject* pDocObj = pNodeItem->GetObject();

  auto* pManager = static_cast<const ezAnimationGraphNodeManager*>(m_pManager);

  if (pManager->IsStateMachineNode(pDocObj) || pManager->IsStateNode(pDocObj))
  {
    NavigateToScope(pDocObj->GetGuid());
    event->accept();
    return;
  }

  ezQtVisualGraphScene::mouseDoubleClickEvent(event);
}
