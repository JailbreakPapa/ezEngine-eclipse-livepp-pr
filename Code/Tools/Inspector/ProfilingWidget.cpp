#include <Inspector/InspectorPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Math/Color.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <Inspector/ProfilingWidget.moc.h>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QMouseEvent>
#include <QScrollBar>
#include <QToolTip>

ezQtProfilingWidget* ezQtProfilingWidget::s_pWidget = nullptr;

ezQtProfilingWidget::ezQtProfilingWidget(ads::CDockManager* pDockManager, QWidget* pParent)
  : ads::CDockWidget(pDockManager, "Profiling Widget", pParent)
{
  s_pWidget = this;

  setupUi(this);
  setWidget(ProfilingWidgetFrame);

  setIcon(QIcon(":/Icons/Icons/Time.svg"));

  ProfileView->setScene(&m_Scene);
  ProfileView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  ProfileView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  ProfileView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  ProfileView->setMouseTracking(true);
  ProfileView->viewport()->installEventFilter(this);

  ComboZoom->setCurrentIndex(3); // Default to 16.7ms

  ResetStats();
}

void ezQtProfilingWidget::ResetStats()
{
  m_ThreadData.Clear();
  m_GPUScopes.Clear();
  m_Frames.Clear();
  m_Scene.clear();
  m_bDataReceived = false;
  m_iSelectedFrame = -1;
  m_pHoveredScope = nullptr;

  SpinFrame->setMaximum(0);
  SpinFrame->setValue(0);
  LabelFrameTime->setText("-- ms");
  LabelHoverInfo->setText("Click Capture to get profiling data");
}

void ezQtProfilingWidget::UpdateStats()
{
  if (!isVisible())
    return;

  if (!ezTelemetry::IsConnectedToServer())
  {
    ButtonCapture->setEnabled(false);
    return;
  }

  ButtonCapture->setEnabled(true);
}

void ezQtProfilingWidget::ProcessTelemetry(void* pUnused)
{
  if (s_pWidget == nullptr)
    return;

  ezTelemetryMessage Msg;

  while (ezTelemetry::RetrieveMessage('PROF', Msg) == EZ_SUCCESS)
  {
    if (Msg.GetMessageID() == 'DATA')
    {
      s_pWidget->m_ThreadData.Clear();
      s_pWidget->m_GPUScopes.Clear();
      s_pWidget->m_Frames.Clear();

      ezUInt64 uiFrameCount;
      Msg.GetReader() >> uiFrameCount;

      // Read thread data
      ezUInt32 uiThreadCount;
      Msg.GetReader() >> uiThreadCount;

      for (ezUInt32 t = 0; t < uiThreadCount; ++t)
      {
        ThreadData& thread = s_pWidget->m_ThreadData.ExpandAndGetRef();

        Msg.GetReader() >> thread.m_uiThreadId;
        Msg.GetReader() >> thread.m_sName;

        ezUInt32 uiScopeCount;
        Msg.GetReader() >> uiScopeCount;

        thread.m_Scopes.Reserve(uiScopeCount);

        for (ezUInt32 s = 0; s < uiScopeCount; ++s)
        {
          ScopeData& scope = thread.m_Scopes.ExpandAndGetRef();

          Msg.GetReader() >> scope.m_BeginTime;
          Msg.GetReader() >> scope.m_EndTime;
          Msg.GetReader() >> scope.m_sName;
          Msg.GetReader() >> scope.m_sFunctionName;

          scope.m_Color = s_pWidget->GetColorForName(scope.m_sName.GetData());
        }

        // Compute depths for this thread
        s_pWidget->ComputeScopeDepths(thread.m_Scopes);
      }

      // Read GPU scopes
      ezUInt32 uiGPUScopeCount;
      Msg.GetReader() >> uiGPUScopeCount;

      s_pWidget->m_GPUScopes.Reserve(uiGPUScopeCount);

      for (ezUInt32 s = 0; s < uiGPUScopeCount; ++s)
      {
        ScopeData& scope = s_pWidget->m_GPUScopes.ExpandAndGetRef();

        Msg.GetReader() >> scope.m_BeginTime;
        Msg.GetReader() >> scope.m_EndTime;
        Msg.GetReader() >> scope.m_sName;

        scope.m_Color = s_pWidget->GetColorForName(scope.m_sName.GetData());
      }

      s_pWidget->ComputeScopeDepths(s_pWidget->m_GPUScopes);

      // Read frame times
      ezUInt32 uiFrameStartCount;
      Msg.GetReader() >> uiFrameStartCount;

      for (ezUInt32 f = 0; f + 1 < uiFrameStartCount; ++f)
      {
        ezTime frameStart, frameEnd;
        Msg.GetReader() >> frameStart;

        // Peek at next frame start to get end time
        if (f + 1 < uiFrameStartCount)
        {
          FrameData& frame = s_pWidget->m_Frames.ExpandAndGetRef();
          frame.m_BeginTime = frameStart;
          frame.m_uiFrameIndex = f;
        }
      }

      // Read the last frame start
      if (uiFrameStartCount > 0)
      {
        ezTime lastFrameStart;
        Msg.GetReader() >> lastFrameStart;

        if (!s_pWidget->m_Frames.IsEmpty())
        {
          // Set end time of previous frames
          for (ezUInt32 i = 0; i + 1 < s_pWidget->m_Frames.GetCount(); ++i)
          {
            s_pWidget->m_Frames[i].m_EndTime = s_pWidget->m_Frames[i + 1].m_BeginTime;
          }
          s_pWidget->m_Frames.PeekBack().m_EndTime = lastFrameStart;
        }
      }

      s_pWidget->m_bDataReceived = true;
      s_pWidget->UpdateFrameNavigation();
      s_pWidget->RenderBars();
    }
  }
}

void ezQtProfilingWidget::on_ButtonCapture_clicked()
{
  LabelHoverInfo->setText("Waiting for profiling data...");

  ezTelemetryMessage Msg;
  Msg.SetMessageID('PROF', ' REQ');
  ezTelemetry::SendToServer(Msg);
}

void ezQtProfilingWidget::on_ComboZoom_currentIndexChanged(int index)
{
  const double zoomLevels[] = {1.0, 5.0, 10.0, 16.67, 33.33, 100.0};
  m_fPixelsPerMs = 800.0 / zoomLevels[index]; // 800 pixels for full zoom range

  if (m_bDataReceived)
    RenderBars();
}

void ezQtProfilingWidget::on_CheckShowGPU_stateChanged(int state)
{
  m_bShowGPU = (state == Qt::Checked);
  if (m_bDataReceived)
    RenderBars();
}

void ezQtProfilingWidget::on_CheckShowFrames_stateChanged(int state)
{
  m_bShowFrames = (state == Qt::Checked);
  if (m_bDataReceived)
    RenderBars();
}

void ezQtProfilingWidget::on_SpinFrame_valueChanged(int value)
{
  m_iSelectedFrame = value;
  if (m_bDataReceived)
    RenderBars();
}

void ezQtProfilingWidget::on_ButtonPrevFrame_clicked()
{
  if (SpinFrame->value() > 0)
    SpinFrame->setValue(SpinFrame->value() - 1);
}

void ezQtProfilingWidget::on_ButtonNextFrame_clicked()
{
  if (SpinFrame->value() < SpinFrame->maximum())
    SpinFrame->setValue(SpinFrame->value() + 1);
}

void ezQtProfilingWidget::ComputeScopeDepths(ezDynamicArray<ScopeData>& scopes)
{
  if (scopes.IsEmpty())
    return;

  // Sort by begin time, then by duration (longer first)
  scopes.Sort([](const ScopeData& a, const ScopeData& b) {
    if (a.m_BeginTime != b.m_BeginTime)
      return a.m_BeginTime < b.m_BeginTime;
    return (a.m_EndTime - a.m_BeginTime) > (b.m_EndTime - b.m_BeginTime);
  });

  // Track active scopes to determine depth
  ezHybridArray<ezUInt32, 32> activeStack;

  for (ezUInt32 i = 0; i < scopes.GetCount(); ++i)
  {
    ScopeData& scope = scopes[i];

    // Pop scopes that have ended
    while (!activeStack.IsEmpty())
    {
      const ScopeData& top = scopes[activeStack.PeekBack()];
      if (top.m_EndTime <= scope.m_BeginTime)
      {
        activeStack.PopBack();
      }
      else
      {
        break;
      }
    }

    scope.m_iDepth = static_cast<ezInt32>(activeStack.GetCount());
    activeStack.PushBack(i);
  }
}

QColor ezQtProfilingWidget::GetColorForName(const char* szName)
{
  // Hash the name for consistent colors
  ezUInt32 hash = ezHashingUtils::xxHash32String(szName);

  // Generate HSV color with good saturation and value
  float hue = (hash % 360) / 360.0f;
  float saturation = 0.5f + (((hash >> 8) % 50) / 100.0f);
  float value = 0.6f + (((hash >> 16) % 40) / 100.0f);

  QColor color;
  color.setHsvF(hue, saturation, value);
  return color;
}

void ezQtProfilingWidget::UpdateFrameNavigation()
{
  if (m_Frames.IsEmpty())
  {
    SpinFrame->setMaximum(0);
    LabelFrameTime->setText("-- ms");
    return;
  }

  SpinFrame->setMaximum(static_cast<int>(m_Frames.GetCount()) - 1);

  if (m_iSelectedFrame < 0 || m_iSelectedFrame >= (ezInt32)m_Frames.GetCount())
  {
    m_iSelectedFrame = 0;
    SpinFrame->setValue(0);
  }

  const FrameData& frame = m_Frames[m_iSelectedFrame];
  double frameTimeMs = (frame.m_EndTime - frame.m_BeginTime).GetMilliseconds();

  ezStringBuilder s;
  s.SetFormat("{0} ms", ezArgF(frameTimeMs, 2));
  LabelFrameTime->setText(s.GetData());
}

void ezQtProfilingWidget::RenderBars()
{
  m_Scene.clear();

  if (!m_bDataReceived)
    return;

  const double barHeight = 18.0;
  const double trackSpacing = 8.0;
  const double labelWidth = 120.0;

  double currentY = 10.0;

  // Determine time range based on selected frame
  ezTime viewStart = ezTime::MakeZero();
  ezTime viewEnd = ezTime::MakeFromMilliseconds(16.67);

  if (m_iSelectedFrame >= 0 && m_iSelectedFrame < (ezInt32)m_Frames.GetCount())
  {
    const FrameData& frame = m_Frames[m_iSelectedFrame];
    viewStart = frame.m_BeginTime;
    viewEnd = frame.m_EndTime;
  }

  m_ViewStartTime = viewStart;
  m_ViewEndTime = viewEnd;
  m_TotalDuration = viewEnd - viewStart;

  // Draw time ruler
  {
    QGraphicsTextItem* ruler = m_Scene.addText("0 ms");
    ruler->setDefaultTextColor(QColor(180, 180, 180));
    ruler->setPos(labelWidth, 0);

    double totalMs = m_TotalDuration.GetMilliseconds();
    ezStringBuilder s;
    s.SetFormat("{0} ms", ezArgF(totalMs, 2));
    QGraphicsTextItem* rulerEnd = m_Scene.addText(s.GetData());
    rulerEnd->setDefaultTextColor(QColor(180, 180, 180));
    rulerEnd->setPos(labelWidth + totalMs * m_fPixelsPerMs - 50, 0);

    currentY += 20;
  }

  // Render CPU thread tracks
  for (const ThreadData& thread : m_ThreadData)
  {
    // Track label
    QGraphicsTextItem* label = m_Scene.addText(thread.m_sName.GetData());
    label->setDefaultTextColor(QColor(200, 200, 200));
    label->setPos(0, currentY);

    int maxDepth = 0;

    // Render scopes
    for (const ScopeData& scope : thread.m_Scopes)
    {
      // Filter to current frame time range
      if (scope.m_EndTime < viewStart || scope.m_BeginTime > viewEnd)
        continue;

      double x = labelWidth + (scope.m_BeginTime - viewStart).GetMilliseconds() * m_fPixelsPerMs;
      double width = (scope.m_EndTime - scope.m_BeginTime).GetMilliseconds() * m_fPixelsPerMs;
      double y = currentY + scope.m_iDepth * (barHeight + 2);

      if (width < 1.0)
        width = 1.0;

      QGraphicsRectItem* rect = m_Scene.addRect(x, y, width, barHeight);
      rect->setBrush(scope.m_Color);
      rect->setPen(QPen(scope.m_Color.darker(120)));
      rect->setData(0, QVariant::fromValue(reinterpret_cast<quintptr>(&scope)));

      // Add text label if bar is wide enough
      if (width > 40)
      {
        QGraphicsTextItem* text = m_Scene.addText(scope.m_sName.GetData());
        text->setPos(x + 2, y - 2);
        text->setDefaultTextColor(Qt::white);

        QFont font = text->font();
        font.setPointSize(8);
        text->setFont(font);
      }

      maxDepth = ezMath::Max(maxDepth, scope.m_iDepth);
    }

    currentY += (maxDepth + 1) * (barHeight + 2) + trackSpacing;
  }

  // Render GPU track
  if (m_bShowGPU && !m_GPUScopes.IsEmpty())
  {
    QGraphicsTextItem* label = m_Scene.addText("GPU");
    label->setDefaultTextColor(QColor(100, 200, 100));
    label->setPos(0, currentY);

    int maxDepth = 0;

    for (const ScopeData& scope : m_GPUScopes)
    {
      if (scope.m_EndTime < viewStart || scope.m_BeginTime > viewEnd)
        continue;

      double x = labelWidth + (scope.m_BeginTime - viewStart).GetMilliseconds() * m_fPixelsPerMs;
      double width = (scope.m_EndTime - scope.m_BeginTime).GetMilliseconds() * m_fPixelsPerMs;
      double y = currentY + scope.m_iDepth * (barHeight + 2);

      if (width < 1.0)
        width = 1.0;

      QGraphicsRectItem* rect = m_Scene.addRect(x, y, width, barHeight);
      rect->setBrush(scope.m_Color);
      rect->setPen(QPen(scope.m_Color.darker(120)));
      rect->setData(0, QVariant::fromValue(reinterpret_cast<quintptr>(&scope)));

      if (width > 40)
      {
        QGraphicsTextItem* text = m_Scene.addText(scope.m_sName.GetData());
        text->setPos(x + 2, y - 2);
        text->setDefaultTextColor(Qt::white);

        QFont font = text->font();
        font.setPointSize(8);
        text->setFont(font);
      }

      maxDepth = ezMath::Max(maxDepth, scope.m_iDepth);
    }

    currentY += (maxDepth + 1) * (barHeight + 2) + trackSpacing;
  }

  // Render frame markers
  if (m_bShowFrames)
  {
    QGraphicsTextItem* label = m_Scene.addText("Frames");
    label->setDefaultTextColor(QColor(200, 200, 100));
    label->setPos(0, currentY);

    for (ezUInt32 i = 0; i < m_Frames.GetCount(); ++i)
    {
      const FrameData& frame = m_Frames[i];

      if (frame.m_EndTime < viewStart || frame.m_BeginTime > viewEnd)
        continue;

      double x = labelWidth + (frame.m_BeginTime - viewStart).GetMilliseconds() * m_fPixelsPerMs;

      // Draw vertical line at frame start
      QPen framePen(QColor(200, 200, 100, 128));
      framePen.setStyle(Qt::DashLine);
      m_Scene.addLine(x, 20, x, currentY, framePen);

      // Frame number label
      ezStringBuilder s;
      s.SetFormat("{0}", frame.m_uiFrameIndex);
      QGraphicsTextItem* frameLabel = m_Scene.addText(s.GetData());
      frameLabel->setDefaultTextColor(QColor(200, 200, 100));
      frameLabel->setPos(x + 2, currentY);

      QFont font = frameLabel->font();
      font.setPointSize(7);
      frameLabel->setFont(font);
    }

    currentY += barHeight + trackSpacing;
  }

  // Set scene rect
  double totalWidth = labelWidth + m_TotalDuration.GetMilliseconds() * m_fPixelsPerMs + 50;
  m_Scene.setSceneRect(0, 0, totalWidth, currentY + 20);

  LabelHoverInfo->setText("Hover over a bar to see details");
}

bool ezQtProfilingWidget::eventFilter(QObject* pObject, QEvent* pEvent)
{
  if (pObject == ProfileView->viewport())
  {
    if (pEvent->type() == QEvent::MouseMove)
    {
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(pEvent);
      QPointF scenePos = ProfileView->mapToScene(mouseEvent->pos());

      QGraphicsItem* item = m_Scene.itemAt(scenePos, ProfileView->transform());

      if (item && item->type() == QGraphicsRectItem::Type)
      {
        QGraphicsRectItem* rectItem = static_cast<QGraphicsRectItem*>(item);
        quintptr ptr = rectItem->data(0).value<quintptr>();

        if (ptr != 0)
        {
          const ScopeData* pScope = reinterpret_cast<const ScopeData*>(ptr);
          ShowScopeTooltip(pScope, scenePos);
          return false;
        }
      }

      LabelHoverInfo->setText("Hover over a bar to see details");
    }
  }

  return ads::CDockWidget::eventFilter(pObject, pEvent);
}

void ezQtProfilingWidget::ShowScopeTooltip(const ScopeData* pScope, const QPointF& pos)
{
  if (pScope == nullptr)
    return;

  double durationMs = (pScope->m_EndTime - pScope->m_BeginTime).GetMilliseconds();
  double framePercent = 0.0;

  if (m_TotalDuration.GetMilliseconds() > 0)
  {
    framePercent = (durationMs / m_TotalDuration.GetMilliseconds()) * 100.0;
  }

  ezStringBuilder s;
  s.SetFormat("{0} - {1} ms ({2}%)", pScope->m_sName, ezArgF(durationMs, 3), ezArgF(framePercent, 1));

  if (!pScope->m_sFunctionName.IsEmpty())
  {
    s.AppendFormat(" | {0}", pScope->m_sFunctionName);
  }

  LabelHoverInfo->setText(s.GetData());
}
