#pragma once

#include <EditorPluginRazor/EditorPluginRazorDLL.h>
#include <GuiFoundation/DockPanels/DocumentPanel.moc.h>
#include <SharedPluginRazor/Common/Messages.h>

#include <QTreeWidget>

class ezQtRazorAssetDocumentWindow;

/// Tree widget that displays the Razor DOM hierarchy.
class ezQtRazorHierarchyTree : public QTreeWidget
{
  Q_OBJECT

public:
  explicit ezQtRazorHierarchyTree(QWidget* pParent = nullptr);

  void SetHierarchy(const ezDynamicArray<ezRazorElementInfo>& elements);
  void SetSelectedElement(ezUInt32 uiElementId);

Q_SIGNALS:
  void ElementSelected(ezUInt32 uiElementId);
  void ElementHovered(ezUInt32 uiElementId);

private Q_SLOTS:
  void OnItemSelectionChanged();
  void OnItemEntered(QTreeWidgetItem* pItem, int column);

private:
  void BuildTree(const ezDynamicArray<ezRazorElementInfo>& elements);
  QTreeWidgetItem* CreateItemForElement(const ezRazorElementInfo& element);
  QString FormatElementLabel(const ezRazorElementInfo& element);

  ezHashTable<ezUInt32, QTreeWidgetItem*> m_ElementToItem;
  bool m_bUpdating = false;
};

/// Panel that hosts the DOM hierarchy tree.
class ezQtRazorHierarchyPanel : public ads::CDockWidget
{
  Q_OBJECT

public:
  ezQtRazorHierarchyPanel(ads::CDockManager* pManager, ezQtRazorAssetDocumentWindow* pWindow);

  void SetHierarchy(const ezDynamicArray<ezRazorElementInfo>& elements);
  void SetSelectedElement(ezUInt32 uiElementId);

private Q_SLOTS:
  void OnElementSelected(ezUInt32 uiElementId);
  void OnElementHovered(ezUInt32 uiElementId);

private:
  ezQtRazorAssetDocumentWindow* m_pWindow = nullptr;
  ezQtRazorHierarchyTree* m_pTree = nullptr;
};
