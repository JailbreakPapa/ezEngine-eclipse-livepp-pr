#include <EditorPluginRazor/EditorPluginRazorPCH.h>

#include <EditorPluginRazor/RazorAsset/RazorAssetWindow.moc.h>
#include <EditorPluginRazor/RazorAsset/RazorHierarchyPanel.moc.h>
#include <SharedPluginRazor/Common/Messages.h>

#include <QHeaderView>
#include <QVBoxLayout>

//////////////////////////////////////////////////////////////////////////
// ezQtRazorHierarchyTree
//////////////////////////////////////////////////////////////////////////

ezQtRazorHierarchyTree::ezQtRazorHierarchyTree(QWidget* pParent)
  : QTreeWidget(pParent)
{
  setHeaderHidden(true);
  setSelectionMode(QAbstractItemView::SingleSelection);
  setMouseTracking(true);

  connect(this, &QTreeWidget::itemSelectionChanged, this, &ezQtRazorHierarchyTree::OnItemSelectionChanged);
  connect(this, &QTreeWidget::itemEntered, this, &ezQtRazorHierarchyTree::OnItemEntered);
}

void ezQtRazorHierarchyTree::SetHierarchy(const ezDynamicArray<ezRazorElementInfo>& elements)
{
  m_bUpdating = true;
  clear();
  m_ElementToItem.Clear();

  BuildTree(elements);

  expandAll();
  m_bUpdating = false;
}

void ezQtRazorHierarchyTree::SetSelectedElement(ezUInt32 uiElementId)
{
  m_bUpdating = true;

  QTreeWidgetItem* pItem = nullptr;
  if (m_ElementToItem.TryGetValue(uiElementId, pItem))
  {
    setCurrentItem(pItem);
    scrollToItem(pItem);
  }
  else
  {
    clearSelection();
  }

  m_bUpdating = false;
}

void ezQtRazorHierarchyTree::OnItemSelectionChanged()
{
  if (m_bUpdating)
    return;

  QList<QTreeWidgetItem*> selected = selectedItems();
  if (selected.isEmpty())
  {
    Q_EMIT ElementSelected(0);
    return;
  }

  QTreeWidgetItem* pItem = selected.first();
  ezUInt32 elementId = pItem->data(0, Qt::UserRole).toUInt();
  Q_EMIT ElementSelected(elementId);
}

void ezQtRazorHierarchyTree::OnItemEntered(QTreeWidgetItem* pItem, int column)
{
  EZ_IGNORE_UNUSED(column);

  if (pItem)
  {
    ezUInt32 elementId = pItem->data(0, Qt::UserRole).toUInt();
    Q_EMIT ElementHovered(elementId);
  }
}

void ezQtRazorHierarchyTree::BuildTree(const ezDynamicArray<ezRazorElementInfo>& elements)
{
  // First pass: create all items
  ezHashTable<ezUInt32, ezRazorElementInfo> elementMap;
  for (const auto& elem : elements)
  {
    elementMap[elem.m_uiElementId] = elem;
  }

  // Build parent-to-children mapping
  ezHashTable<ezUInt32, ezDynamicArray<ezUInt32>> childrenMap;
  ezUInt32 rootId = 0;

  for (const auto& elem : elements)
  {
    if (elem.m_uiParentId == 0)
    {
      rootId = elem.m_uiElementId;
    }
    else
    {
      childrenMap[elem.m_uiParentId].PushBack(elem.m_uiElementId);
    }
  }

  // Recursive function to build tree
  std::function<void(ezUInt32, QTreeWidgetItem*)> buildSubtree;
  buildSubtree = [&](ezUInt32 elementId, QTreeWidgetItem* pParentItem) {
    ezRazorElementInfo* pInfo = nullptr;
    if (!elementMap.TryGetValue(elementId, pInfo))
      return;

    QTreeWidgetItem* pItem = CreateItemForElement(*pInfo);
    m_ElementToItem[elementId] = pItem;

    if (pParentItem)
    {
      pParentItem->addChild(pItem);
    }
    else
    {
      addTopLevelItem(pItem);
    }

    ezDynamicArray<ezUInt32>* pChildren = nullptr;
    if (childrenMap.TryGetValue(elementId, pChildren))
    {
      for (ezUInt32 childId : *pChildren)
      {
        buildSubtree(childId, pItem);
      }
    }
  };

  // Start building from root
  if (rootId != 0)
  {
    buildSubtree(rootId, nullptr);
  }
}

QTreeWidgetItem* ezQtRazorHierarchyTree::CreateItemForElement(const ezRazorElementInfo& element)
{
  QTreeWidgetItem* pItem = new QTreeWidgetItem();
  pItem->setText(0, FormatElementLabel(element));
  pItem->setData(0, Qt::UserRole, QVariant(element.m_uiElementId));

  // Set icon based on element type
  // TODO: Add icons for different element types

  return pItem;
}

QString ezQtRazorHierarchyTree::FormatElementLabel(const ezRazorElementInfo& element)
{
  QString label = QString::fromUtf8(element.m_sTagName.GetData());

  if (!element.m_sId.IsEmpty())
  {
    label += QString("#%1").arg(QString::fromUtf8(element.m_sId.GetData()));
  }

  for (const auto& cls : element.m_Classes)
  {
    label += QString(".%1").arg(QString::fromUtf8(cls.GetData()));
  }

  return label;
}

//////////////////////////////////////////////////////////////////////////
// ezQtRazorHierarchyPanel
//////////////////////////////////////////////////////////////////////////

ezQtRazorHierarchyPanel::ezQtRazorHierarchyPanel(ads::CDockManager* pManager, ezQtRazorAssetDocumentWindow* pWindow)
  : ads::CDockWidget(pManager, "Hierarchy", pWindow)
  , m_pWindow(pWindow)
{
  m_pTree = new ezQtRazorHierarchyTree(this);
  setWidget(m_pTree);

  connect(m_pTree, &ezQtRazorHierarchyTree::ElementSelected, this, &ezQtRazorHierarchyPanel::OnElementSelected);
  connect(m_pTree, &ezQtRazorHierarchyTree::ElementHovered, this, &ezQtRazorHierarchyPanel::OnElementHovered);
}

void ezQtRazorHierarchyPanel::SetHierarchy(const ezDynamicArray<ezRazorElementInfo>& elements)
{
  m_pTree->SetHierarchy(elements);
}

void ezQtRazorHierarchyPanel::SetSelectedElement(ezUInt32 uiElementId)
{
  m_pTree->SetSelectedElement(uiElementId);
}

void ezQtRazorHierarchyPanel::OnElementSelected(ezUInt32 uiElementId)
{
  if (m_pWindow)
  {
    m_pWindow->SelectElement(uiElementId);
  }
}

void ezQtRazorHierarchyPanel::OnElementHovered(ezUInt32 uiElementId)
{
  if (m_pWindow)
  {
    // Send highlight message to engine
    ezRazorHighlightElementMsgToEngine msg;
    msg.m_DocumentGuid = m_pWindow->GetDocument()->GetGuid();
    msg.m_uiElementId = uiElementId;
    msg.m_bHighlight = true;
    m_pWindow->GetEditorEngineConnection()->SendMessage(&msg);
  }
}

EZ_STATICLINK_FILE(EditorPluginRazor, EditorPluginRazor_RazorAsset_RazorHierarchyPanel);
