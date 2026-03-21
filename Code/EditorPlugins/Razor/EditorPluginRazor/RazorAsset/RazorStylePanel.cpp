#include <EditorPluginRazor/EditorPluginRazorPCH.h>

#include <EditorPluginRazor/RazorAsset/RazorAssetWindow.moc.h>
#include <EditorPluginRazor/RazorAsset/RazorStylePanel.moc.h>

#include <QHeaderView>
#include <QPainter>
#include <QVBoxLayout>

//////////////////////////////////////////////////////////////////////////
// ezQtRazorStyleTable
//////////////////////////////////////////////////////////////////////////

ezQtRazorStyleTable::ezQtRazorStyleTable(QWidget* pParent)
  : QTableWidget(pParent)
{
  setColumnCount(3);
  setHorizontalHeaderLabels({"Property", "Value", "Source"});
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setSelectionMode(QAbstractItemView::SingleSelection);
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setAlternatingRowColors(true);

  horizontalHeader()->setStretchLastSection(true);
  horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  verticalHeader()->setVisible(false);
}

void ezQtRazorStyleTable::SetProperties(const ezDynamicArray<ezRazorStyleProperty>& properties)
{
  Clear();
  setRowCount(static_cast<int>(properties.GetCount()));

  int row = 0;
  for (const auto& prop : properties)
  {
    QTableWidgetItem* pNameItem = new QTableWidgetItem(QString::fromUtf8(prop.m_sName.GetData()));
    QTableWidgetItem* pValueItem = new QTableWidgetItem(QString::fromUtf8(prop.m_sValue.GetData()));
    QTableWidgetItem* pSourceItem = new QTableWidgetItem(QString::fromUtf8(prop.m_sSource.GetData()));

    // Style inherited properties differently
    if (prop.m_bInherited)
    {
      QColor inheritedColor(128, 128, 128);
      pNameItem->setForeground(inheritedColor);
      pValueItem->setForeground(inheritedColor);
      pSourceItem->setForeground(inheritedColor);
    }

    // Strike through overridden properties
    if (prop.m_bOverridden)
    {
      QFont font = pNameItem->font();
      font.setStrikeOut(true);
      pNameItem->setFont(font);
      pValueItem->setFont(font);
      pSourceItem->setFont(font);
    }

    setItem(row, 0, pNameItem);
    setItem(row, 1, pValueItem);
    setItem(row, 2, pSourceItem);
    ++row;
  }
}

void ezQtRazorStyleTable::Clear()
{
  setRowCount(0);
}

//////////////////////////////////////////////////////////////////////////
// ezQtRazorLayoutBoxWidget
//////////////////////////////////////////////////////////////////////////

ezQtRazorLayoutBoxWidget::ezQtRazorLayoutBoxWidget(QWidget* pParent)
  : QWidget(pParent)
{
  setMinimumSize(200, 150);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void ezQtRazorLayoutBoxWidget::SetLayoutInfo(const ezRazorLayoutInfo& layout)
{
  m_Layout = layout;
  m_bHasData = true;
  update();
}

void ezQtRazorLayoutBoxWidget::Clear()
{
  m_bHasData = false;
  update();
}

void ezQtRazorLayoutBoxWidget::paintEvent(QPaintEvent* pEvent)
{
  EZ_IGNORE_UNUSED(pEvent);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QRect widgetRect = rect().adjusted(10, 10, -10, -10);

  if (!m_bHasData)
  {
    painter.drawText(widgetRect, Qt::AlignCenter, "No element selected");
    return;
  }

  // Colors inspired by browser dev tools
  QColor marginColor(255, 200, 150, 180);
  QColor borderColor(255, 220, 120, 200);
  QColor paddingColor(180, 220, 160, 180);
  QColor contentColor(140, 180, 220, 200);

  // Scale factor to fit layout boxes within widget
  float layoutWidth = m_Layout.m_MarginBox.width;
  float layoutHeight = m_Layout.m_MarginBox.height;
  float scaleX = layoutWidth > 0 ? widgetRect.width() / layoutWidth : 1.0f;
  float scaleY = layoutHeight > 0 ? widgetRect.height() / layoutHeight : 1.0f;
  float scale = qMin(scaleX, scaleY) * 0.9f;

  auto toWidgetRect = [&](const ezRectFloat& r) -> QRectF {
    float cx = widgetRect.center().x();
    float cy = widgetRect.center().y();
    float w = r.width * scale;
    float h = r.height * scale;
    return QRectF(cx - w / 2, cy - h / 2, w, h);
  };

  // Draw boxes from outside in
  painter.setBrush(marginColor);
  painter.setPen(Qt::NoPen);
  painter.drawRect(toWidgetRect(m_Layout.m_MarginBox));

  painter.setBrush(borderColor);
  painter.drawRect(toWidgetRect(m_Layout.m_BorderBox));

  painter.setBrush(paddingColor);
  painter.drawRect(toWidgetRect(m_Layout.m_PaddingBox));

  painter.setBrush(contentColor);
  painter.drawRect(toWidgetRect(m_Layout.m_ContentBox));

  // Draw labels
  painter.setPen(Qt::black);
  QFont font = painter.font();
  font.setPointSize(8);
  painter.setFont(font);

  QRectF contentRect = toWidgetRect(m_Layout.m_ContentBox);
  QString sizeText = QString("%1 x %2")
                       .arg(static_cast<int>(m_Layout.m_ContentBox.width))
                       .arg(static_cast<int>(m_Layout.m_ContentBox.height));
  painter.drawText(contentRect, Qt::AlignCenter, sizeText);

  // Draw legend
  int legendY = widgetRect.bottom() - 20;
  int legendX = widgetRect.left();

  auto drawLegendItem = [&](int x, const QColor& color, const QString& label) {
    painter.setBrush(color);
    painter.drawRect(x, legendY, 12, 12);
    painter.drawText(x + 16, legendY + 10, label);
  };

  drawLegendItem(legendX, marginColor, "margin");
  drawLegendItem(legendX + 60, borderColor, "border");
  drawLegendItem(legendX + 120, paddingColor, "padding");
  drawLegendItem(legendX + 180, contentColor, "content");
}

//////////////////////////////////////////////////////////////////////////
// ezQtRazorStylePanel
//////////////////////////////////////////////////////////////////////////

ezQtRazorStylePanel::ezQtRazorStylePanel(ads::CDockManager* pManager, ezQtRazorAssetDocumentWindow* pWindow)
  : ads::CDockWidget(pManager, "Styles", pWindow)
  , m_pWindow(pWindow)
{
  QWidget* pContainer = new QWidget(this);
  QVBoxLayout* pLayout = new QVBoxLayout(pContainer);
  pLayout->setContentsMargins(0, 0, 0, 0);

  m_pTabWidget = new QTabWidget(pContainer);

  // Computed Style tab
  m_pStyleTable = new ezQtRazorStyleTable(m_pTabWidget);
  m_pTabWidget->addTab(m_pStyleTable, "Computed");

  // Layout tab
  QWidget* pLayoutTab = new QWidget(m_pTabWidget);
  QVBoxLayout* pLayoutTabLayout = new QVBoxLayout(pLayoutTab);
  m_pLayoutBox = new ezQtRazorLayoutBoxWidget(pLayoutTab);
  pLayoutTabLayout->addWidget(m_pLayoutBox);
  pLayoutTabLayout->addStretch();
  m_pTabWidget->addTab(pLayoutTab, "Layout");

  pLayout->addWidget(m_pTabWidget);
  setWidget(pContainer);
}

void ezQtRazorStylePanel::SetComputedStyle(const ezDynamicArray<ezRazorStyleProperty>& properties)
{
  m_pStyleTable->SetProperties(properties);
}

void ezQtRazorStylePanel::SetLayoutInfo(const ezRazorLayoutInfo& layout)
{
  m_pLayoutBox->SetLayoutInfo(layout);
}

void ezQtRazorStylePanel::Clear()
{
  m_pStyleTable->Clear();
  m_pLayoutBox->Clear();
}

EZ_STATICLINK_FILE(EditorPluginRazor, EditorPluginRazor_RazorAsset_RazorStylePanel);
