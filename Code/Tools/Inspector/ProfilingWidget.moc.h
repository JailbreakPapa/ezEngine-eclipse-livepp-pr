#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Time/Time.h>
#include <Inspector/ui_ProfilingWidget.h>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <ads/DockWidget.h>

class ezQtProfilingWidget : public ads::CDockWidget, public Ui_ProfilingWidget
{
public:
  Q_OBJECT

public:
  ezQtProfilingWidget(ads::CDockManager* pDockManager, QWidget* pParent = nullptr);

  static ezQtProfilingWidget* s_pWidget;

  static void ProcessTelemetry(void* pUnused);

  void ResetStats();
  void UpdateStats();

private Q_SLOTS:
  void on_ButtonCapture_clicked();
  void on_ComboZoom_currentIndexChanged(int index);
  void on_CheckShowGPU_stateChanged(int state);
  void on_CheckShowFrames_stateChanged(int state);
  void on_SpinFrame_valueChanged(int value);
  void on_ButtonPrevFrame_clicked();
  void on_ButtonNextFrame_clicked();

protected:
  bool eventFilter(QObject* pObject, QEvent* pEvent) override;

private:
  struct ScopeData
  {
    ezString m_sName;
    ezString m_sFunctionName;
    ezTime m_BeginTime;
    ezTime m_EndTime;
    ezInt32 m_iDepth = 0;
    QColor m_Color;
  };

  struct ThreadData
  {
    ezUInt64 m_uiThreadId = 0;
    ezString m_sName;
    ezDynamicArray<ScopeData> m_Scopes;
  };

  struct FrameData
  {
    ezTime m_BeginTime;
    ezTime m_EndTime;
    ezUInt64 m_uiFrameIndex = 0;
  };

  void RenderBars();
  void ComputeScopeDepths(ezDynamicArray<ScopeData>& scopes);
  QColor GetColorForName(const char* szName);
  void UpdateFrameNavigation();
  void ShowScopeTooltip(const ScopeData* pScope, const QPointF& pos);

  QGraphicsScene m_Scene;
  ezDynamicArray<ThreadData> m_ThreadData;
  ezDynamicArray<ScopeData> m_GPUScopes;
  ezDynamicArray<FrameData> m_Frames;

  double m_fPixelsPerMs = 10.0;
  ezTime m_ViewStartTime;
  ezTime m_ViewEndTime;
  ezTime m_TotalDuration;
  ezInt32 m_iSelectedFrame = -1;
  bool m_bShowGPU = true;
  bool m_bShowFrames = true;
  bool m_bDataReceived = false;

  const ScopeData* m_pHoveredScope = nullptr;
};
