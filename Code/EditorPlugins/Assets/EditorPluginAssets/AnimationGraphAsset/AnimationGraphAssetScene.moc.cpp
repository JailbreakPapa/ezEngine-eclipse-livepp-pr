#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphAsset.h>
#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphAssetScene.moc.h>
#include <GuiFoundation/VisualGraph/Connection.h>
#include <GuiFoundation/VisualGraph/Node.h>
#include <GuiFoundation/VisualGraph/Pin.h>
#include <SharedPluginAssets/AnimationGraphAsset/AnimGraphStateMachineTypes.h>
#include <ToolsFoundation/CommandHistory/CommandHistory.h>

#include <QGraphicsSceneMouseEvent>
#include <QTimer>

ezQtAnimationGraphAssetScene::ezQtAnimationGraphAssetScene(QObject* pParent)
  : ezQtVisualGraphScene(pParent)
{
}

ezQtAnimationGraphAssetScene::~ezQtAnimationGraphAssetScene() = default;

void ezQtAnimationGraphAssetScene::InitScene(const ezVisualGraphObjectManager* pManager)
{
  ezQtVisualGraphScene::InitScene(pManager);

  // Apply connection style and decoration flags for the initial scope
  UpdateConnectionStyleForScope();

  // Apply initial scope filter so only root-level nodes are visible
  ApplyScopeFilter();
}

void ezQtAnimationGraphAssetScene::NavigateToScope(const ezUuid& scopeGuid)
{
  m_CurrentScope = scopeGuid;

  // Update the node manager's view scope
  auto* pManager = const_cast<ezAnimationGraphNodeManager*>(
    static_cast<const ezAnimationGraphNodeManager*>(m_pManager));
  pManager->SetCurrentViewScope(scopeGuid);

  // Update connection rendering style
  UpdateConnectionStyleForScope();

  // Show/hide nodes and connections based on the new scope
  ApplyScopeFilter();

  Q_EMIT ScopeChanged(scopeGuid);
}

void ezQtAnimationGraphAssetScene::UpdateConnectionStyleForScope()
{
  auto* pManager = static_cast<const ezAnimationGraphNodeManager*>(m_pManager);

  if (pManager->IsCurrentScopeStateMachine())
  {
    SetConnectionStyle(ezQtVisualGraphScene::ConnectionStyle::StraightLine);
    SetConnectionDecorationFlags(
      ezQtVisualGraphScene::ConnectionDecorationFlags::DirectionArrows |
      ezQtVisualGraphScene::ConnectionDecorationFlags::DrawDebugging);
  }
  else
  {
    SetConnectionStyle(ezQtVisualGraphScene::ConnectionStyle::BezierCurve);
    SetConnectionDecorationFlags(ezQtVisualGraphScene::ConnectionDecorationFlags::DrawDebugging);
  }
}

void ezQtAnimationGraphAssetScene::ApplyScopeFilter()
{
  auto* pManager = static_cast<const ezAnimationGraphNodeManager*>(m_pManager);

  // Filter nodes: only show those belonging to the current scope
  for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
  {
    const ezDocumentObject* pDocObj = it.Key();
    ezQtVisualGraphNode* pQtNode = it.Value();

    ezUuid nodeScope = pManager->GetNodeScope(pDocObj);
    pQtNode->setVisible(nodeScope == m_CurrentScope);
  }

  // Filter connections: only show those where both endpoints are in the current scope
  for (auto it = m_Connections.GetIterator(); it.IsValid(); ++it)
  {
    const ezDocumentObject* pConnectionObj = it.Key();
    ezQtVisualGraphConnection* pQtConnection = it.Value();

    pQtConnection->setVisible(IsConnectionInCurrentScope(pConnectionObj));
  }

  // Force visible nodes to run their deferred layout update immediately,
  // then refresh all connection positions. This must happen because:
  // 1. Nodes have a deferred UpdateGeometry() that runs in paint() which repositions pins.
  // 2. Qt does not fire ItemScenePositionHasChanged when items become visible.
  // By calling ResetFlags + update + UpdateConnections, we ensure pins are at their
  // final positions before connections draw.
  for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
  {
    ezQtVisualGraphNode* pQtNode = it.Value();
    if (!pQtNode->isVisible())
      continue;

    // Force an immediate layout recalculation
    pQtNode->ResetFlags();
    pQtNode->update();
  }

  // Defer connection refresh to after the event loop processes the paint events,
  // ensuring pin positions are finalized by the deferred UpdateGeometry in paint().
  QTimer::singleShot(0, [this]()
    {
      for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
      {
        ezQtVisualGraphNode* pQtNode = it.Value();
        if (!pQtNode->isVisible())
          continue;

        for (ezQtVisualGraphPin* pPin : pQtNode->GetInputPins())
          pPin->UpdateConnections();
        for (ezQtVisualGraphPin* pPin : pQtNode->GetOutputPins())
          pPin->UpdateConnections();
      }
    });
}

bool ezQtAnimationGraphAssetScene::IsConnectionInCurrentScope(const ezDocumentObject* pConnectionObj) const
{
  auto* pManager = static_cast<const ezAnimationGraphNodeManager*>(m_pManager);

  const ezVisualGraphConnection* pConnection = pManager->GetConnectionIfExists(pConnectionObj);
  if (pConnection == nullptr)
    return false;

  const ezDocumentObject* pSourceNode = pConnection->GetSourcePin().GetParent();
  const ezDocumentObject* pTargetNode = pConnection->GetTargetPin().GetParent();

  ezUuid sourceScope = pManager->GetNodeScope(pSourceNode);
  ezUuid targetScope = pManager->GetNodeScope(pTargetNode);

  return sourceScope == m_CurrentScope && targetScope == m_CurrentScope;
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

void ezQtAnimationGraphAssetScene::SetNodeActivity(const ezSet<ezUuid>& activeNodes)
{
  // Skip all work if the active set hasn't changed since last frame
  if (activeNodes == m_PreviousActiveNodes)
  {
    // Still need to repaint active connections for the animated dots
    for (auto it = m_Connections.GetIterator(); it.IsValid(); ++it)
    {
      ezQtVisualGraphConnection* pQtCon = it.Value();
      if (pQtCon->m_bIsActive && pQtCon->isVisible())
        pQtCon->update();
    }
    return;
  }

  m_PreviousActiveNodes = activeNodes;

  // Count how many visible nodes are active vs inactive.
  // In blend trees, all nodes are evaluated every frame, so all are "active".
  // Activity feedback is only useful when there's a real distinction.
  ezUInt32 uiVisibleCount = 0;
  ezUInt32 uiActiveCount = 0;
  for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
  {
    if (!it.Value()->isVisible())
      continue;
    uiVisibleCount++;
    if (activeNodes.Contains(it.Key()->GetGuid()))
      uiActiveCount++;
  }

  // If all visible nodes are active (blend tree) or none are, skip activity visuals
  const bool bShowActivity = uiActiveCount > 0 && uiActiveCount < uiVisibleCount;

  for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
  {
    ezQtVisualGraphNode* pQtNode = it.Value();
    if (!pQtNode->isVisible())
      continue;

    if (bShowActivity)
    {
      const bool bActive = activeNodes.Contains(it.Key()->GetGuid());
      pQtNode->SetActive(bActive);
    }
    else
    {
      pQtNode->SetActive(true); // Reset all to full visibility
    }
  }

  for (auto it = m_Connections.GetIterator(); it.IsValid(); ++it)
  {
    ezQtVisualGraphConnection* pQtCon = it.Value();
    if (!pQtCon->isVisible())
      continue;

    const ezVisualGraphConnection* pCon = pQtCon->GetConnection();
    if (pCon == nullptr)
      continue;

    bool bActive = false;
    if (bShowActivity)
    {
      const ezDocumentObject* pSrcNode = pCon->GetSourcePin().GetParent();
      const ezDocumentObject* pDstNode = pCon->GetTargetPin().GetParent();
      bActive = activeNodes.Contains(pSrcNode->GetGuid()) && activeNodes.Contains(pDstNode->GetGuid());
    }

    const bool bChanged = (pQtCon->m_bIsActive != bActive);
    pQtCon->m_bIsActive = bActive;

    if (bChanged || bActive)
      pQtCon->update();
  }
}
