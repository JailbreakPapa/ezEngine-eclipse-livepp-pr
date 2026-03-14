#pragma once

#include <Foundation/Basics.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

#include <QWidget>

class QTreeWidget;
class ezAnimationGraphNodeManager;

/// Side panel listing all blackboard parameters referenced across the animation graph.
///
/// Scans all nodes across all scopes for blackboard references and displays a table showing
/// each parameter's name, inferred type (Bool/Number), and how many nodes reference it.
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

  const ezAnimationGraphNodeManager* m_pManager = nullptr;
  QTreeWidget* m_pTree = nullptr;
  ezUInt32 m_uiStructureEventSub = 0;
};
