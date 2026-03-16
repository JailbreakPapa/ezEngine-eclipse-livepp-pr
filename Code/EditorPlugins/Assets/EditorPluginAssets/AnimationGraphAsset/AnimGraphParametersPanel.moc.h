#pragma once

#include <Foundation/Basics.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

#include <QWidget>

class QTreeWidget;
class QTreeWidgetItem;
class ezAnimationGraphNodeManager;
class ezDocument;

/// Side panel listing all blackboard parameters referenced across the animation graph.
///
/// Scans all nodes across all scopes for blackboard references and displays a table showing
/// each parameter's name, inferred type (Bool/Number), reference count, and an editable value.
/// Editing a value sends a message to the engine process to update the blackboard for preview.
class ezQtAnimGraphParametersPanel : public QWidget
{
  Q_OBJECT

public:
  explicit ezQtAnimGraphParametersPanel(QWidget* pParent, const ezAnimationGraphNodeManager* pManager);
  ~ezQtAnimGraphParametersPanel();

  /// Rebuilds the parameter table by scanning all nodes in the document.
  void RebuildParameterList();

private:
  void StructureEventHandler(const ezDocumentObjectStructureEvent& e);
  void SendBlackboardValue(const QString& name, const QString& type, const QString& value);

  const ezAnimationGraphNodeManager* m_pManager = nullptr;
  QTreeWidget* m_pTree = nullptr;
};
