#pragma once

#include <EditorPluginRazor/EditorPluginRazorDLL.h>
#include <GuiFoundation/DockPanels/DocumentPanel.moc.h>
#include <SharedPluginRazor/Common/Messages.h>

#include <QTableWidget>
#include <QTabWidget>

class ezQtRazorAssetDocumentWindow;

/// Table widget displaying computed style properties.
class ezQtRazorStyleTable : public QTableWidget
{
  Q_OBJECT

public:
  explicit ezQtRazorStyleTable(QWidget* pParent = nullptr);

  void SetProperties(const ezDynamicArray<ezRazorStyleProperty>& properties);
  void Clear();
};

/// Widget displaying layout box model (margin/border/padding/content).
class ezQtRazorLayoutBoxWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ezQtRazorLayoutBoxWidget(QWidget* pParent = nullptr);

  void SetLayoutInfo(const ezRazorLayoutInfo& layout);
  void Clear();

protected:
  void paintEvent(QPaintEvent* pEvent) override;

private:
  ezRazorLayoutInfo m_Layout;
  bool m_bHasData = false;
};

/// Panel that displays computed styles and layout info for the selected element.
class ezQtRazorStylePanel : public ads::CDockWidget
{
  Q_OBJECT

public:
  ezQtRazorStylePanel(ads::CDockManager* pManager, ezQtRazorAssetDocumentWindow* pWindow);

  void SetComputedStyle(const ezDynamicArray<ezRazorStyleProperty>& properties);
  void SetLayoutInfo(const ezRazorLayoutInfo& layout);
  void Clear();

private:
  ezQtRazorAssetDocumentWindow* m_pWindow = nullptr;
  QTabWidget* m_pTabWidget = nullptr;
  ezQtRazorStyleTable* m_pStyleTable = nullptr;
  ezQtRazorLayoutBoxWidget* m_pLayoutBox = nullptr;
};
