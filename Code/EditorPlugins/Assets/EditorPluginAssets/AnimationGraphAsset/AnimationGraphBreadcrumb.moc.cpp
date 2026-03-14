#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphAsset.h>
#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphBreadcrumb.moc.h>

#include <QLabel>

ezQtAnimGraphBreadcrumb::ezQtAnimGraphBreadcrumb(QWidget* pParent)
  : QWidget(pParent)
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(4, 2, 4, 2);
  m_pLayout->setSpacing(0);

  setLayout(m_pLayout);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setFixedHeight(28);
}

ezQtAnimGraphBreadcrumb::~ezQtAnimGraphBreadcrumb() = default;

void ezQtAnimGraphBreadcrumb::UpdateFromNodeManager(const ezAnimationGraphNodeManager* pManager)
{
  // Clear existing buttons
  while (m_pLayout->count() > 0)
  {
    QLayoutItem* pItem = m_pLayout->takeAt(0);
    if (pItem->widget())
      delete pItem->widget();
    delete pItem;
  }

  // Build breadcrumb path
  ezDynamicArray<ezAnimGraphBreadcrumbEntry> path;
  pManager->GetBreadcrumbPath(path);

  for (ezUInt32 i = 0; i < path.GetCount(); ++i)
  {
    if (i > 0)
    {
      QLabel* pSeparator = new QLabel(" > ", this);
      pSeparator->setStyleSheet("color: #888888; font-size: 11px;");
      m_pLayout->addWidget(pSeparator);
    }

    QPushButton* pButton = new QPushButton(path[i].m_sDisplayName.GetData(), this);
    pButton->setFlat(true);
    pButton->setCursor(Qt::PointingHandCursor);
    pButton->setProperty("scopeGuid", QVariant::fromValue(
      QByteArray(reinterpret_cast<const char*>(&path[i].m_Guid), sizeof(ezUuid))));

    // Highlight the last (current) entry
    if (i == path.GetCount() - 1)
    {
      pButton->setStyleSheet("QPushButton { font-weight: bold; font-size: 11px; padding: 2px 4px; }");
      pButton->setEnabled(false);
    }
    else
    {
      pButton->setStyleSheet("QPushButton { font-size: 11px; padding: 2px 4px; color: #4488CC; } QPushButton:hover { text-decoration: underline; }");
      connect(pButton, &QPushButton::clicked, this, &ezQtAnimGraphBreadcrumb::OnButtonClicked);
    }

    m_pLayout->addWidget(pButton);
  }

  // Add stretch at the end
  m_pLayout->addStretch();
}

void ezQtAnimGraphBreadcrumb::OnButtonClicked()
{
  QPushButton* pButton = qobject_cast<QPushButton*>(sender());
  if (!pButton)
    return;

  QByteArray data = pButton->property("scopeGuid").toByteArray();
  if (data.size() == sizeof(ezUuid))
  {
    ezUuid scopeGuid;
    ezMemoryUtils::Copy(&scopeGuid, reinterpret_cast<const ezUuid*>(data.constData()), 1);
    Q_EMIT NavigateToScope(scopeGuid);
  }
}
