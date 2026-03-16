#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Types/Uuid.h>
#include <GuiFoundation/VisualGraph/Scene.moc.h>

class ezAnimationGraphNodeManager;
class ezQtVisualGraphNode;
class ezQtVisualGraphConnection;

/// Qt scene for animation graph asset editing with hierarchical scope support.
///
/// Extends the base visual graph scene to support navigating into state machine nodes
/// and states via double-click, switching between state machine view (states as boxes,
/// straight-line transitions) and blend tree view (standard node graph with bezier curves).
/// Only nodes and connections belonging to the current scope are visible.
class ezQtAnimationGraphAssetScene : public ezQtVisualGraphScene
{
  Q_OBJECT

public:
  ezQtAnimationGraphAssetScene(QObject* pParent = nullptr);
  ~ezQtAnimationGraphAssetScene();

  virtual void InitScene(const ezVisualGraphObjectManager* pManager) override;

  /// Changes the currently displayed scope. Hides/shows nodes and connections accordingly.
  void NavigateToScope(const ezUuid& scopeGuid);

  /// Executes the command to set a state node as the initial state in its state machine.
  void SetInitialState(ezQtVisualGraphNode* pNode);

  /// Returns the currently displayed scope UUID. Invalid = root.
  const ezUuid& GetCurrentScope() const { return m_CurrentScope; }

  /// Updates which nodes and connections are visually highlighted as active.
  void SetNodeActivity(const ezSet<ezUuid>& activeNodes);

Q_SIGNALS:
  /// Emitted whenever the scope changes, so the window can update the breadcrumb.
  void ScopeChanged(const ezUuid& newScope);

protected:
  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
  void UpdateConnectionStyleForScope();

  /// Hides nodes/connections not in the current scope, shows those that are.
  void ApplyScopeFilter();

  /// Returns true if the given connection object connects two nodes that are both in the current scope.
  bool IsConnectionInCurrentScope(const ezDocumentObject* pConnectionObj) const;

  ezUuid m_CurrentScope;
  ezSet<ezUuid> m_PreviousActiveNodes; ///< Cached for change detection to avoid redundant repaints
};
