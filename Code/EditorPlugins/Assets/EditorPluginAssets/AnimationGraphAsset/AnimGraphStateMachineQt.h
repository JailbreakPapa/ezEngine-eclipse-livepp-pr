#pragma once

#include <Foundation/Basics.h>
#include <GuiFoundation/VisualGraph/Connection.h>
#include <GuiFoundation/VisualGraph/Node.h>
#include <GuiFoundation/VisualGraph/Pin.h>

/// Renders an AnimGraph state as a large rounded rectangle with state name.
class ezQtAnimGraphStateNode : public ezQtVisualGraphNode
{
public:
  ezQtAnimGraphStateNode();

  virtual void InitNode(const ezVisualGraphObjectManager* pManager, const ezDocumentObject* pObject) override;
  virtual void UpdateGeometry() override;
  virtual void UpdateState() override;
  virtual void ExtendContextMenu(QMenu& ref_menu) override;

protected:
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
  bool m_bIsInitialState = false;
  void UpdateHeaderColor();
};

/// Renders the "Any State" pseudo-node with a distinct orange header.
class ezQtAnimGraphAnyStateNode : public ezQtVisualGraphNode
{
public:
  ezQtAnimGraphAnyStateNode();

  virtual void InitNode(const ezVisualGraphObjectManager* pManager, const ezDocumentObject* pObject) override;
  virtual void UpdateGeometry() override;
  virtual void UpdateState() override;

protected:
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
};

/// Simplified rectangular pin for state nodes.
class ezQtAnimGraphStatePin : public ezQtVisualGraphPin
{
public:
  ezQtAnimGraphStatePin();

  virtual void SetPin(const ezVisualGraphPin& pin) override;
};

/// Transition connection with optional condition summary label.
class ezQtAnimGraphTransitionConnectionQt : public ezQtVisualGraphConnection
{
public:
  ezQtAnimGraphTransitionConnectionQt();
};
