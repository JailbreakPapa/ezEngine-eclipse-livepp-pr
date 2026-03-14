#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Types/Uuid.h>
#include <GuiFoundation/VisualGraph/Scene.moc.h>

class ezAnimationGraphNodeManager;
class ezQtVisualGraphNode;

/// Qt scene for animation graph asset editing with hierarchical scope support.
///
/// Extends the base visual graph scene to support navigating into state machine nodes
/// and states via double-click, switching between state machine view (states as boxes,
/// straight-line transitions) and blend tree view (standard node graph with bezier curves).
class ezQtAnimationGraphAssetScene : public ezQtVisualGraphScene
{
  Q_OBJECT

public:
  ezQtAnimationGraphAssetScene(QObject* pParent = nullptr);
  ~ezQtAnimationGraphAssetScene();

  /// Changes the currently displayed scope. Triggers a full scene rebuild.
  void NavigateToScope(const ezUuid& scopeGuid);

  /// Executes the command to set a state node as the initial state in its state machine.
  void SetInitialState(ezQtVisualGraphNode* pNode);

  /// Returns the currently displayed scope UUID. Invalid = root.
  const ezUuid& GetCurrentScope() const { return m_CurrentScope; }

Q_SIGNALS:
  /// Emitted whenever the scope changes, so the window can update the breadcrumb.
  void ScopeChanged(const ezUuid& newScope);

protected:
  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
  void UpdateConnectionStyleForScope();

  ezUuid m_CurrentScope;
};
