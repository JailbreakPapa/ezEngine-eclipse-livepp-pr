#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Types/Uuid.h>

#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>

class ezAnimationGraphNodeManager;

/// Breadcrumb navigation widget for hierarchical AnimGraph editing.
///
/// Displays a clickable path showing the current position in the graph hierarchy,
/// e.g., "Root > Locomotion SM > Walk State". Clicking any segment navigates to that scope.
class ezQtAnimGraphBreadcrumb : public QWidget
{
  Q_OBJECT

public:
  explicit ezQtAnimGraphBreadcrumb(QWidget* pParent = nullptr);
  ~ezQtAnimGraphBreadcrumb();

  void UpdateFromNodeManager(const ezAnimationGraphNodeManager* pManager);

Q_SIGNALS:
  void NavigateToScope(const ezUuid& scopeGuid);

private:
  void OnButtonClicked();

  QHBoxLayout* m_pLayout = nullptr;
};
