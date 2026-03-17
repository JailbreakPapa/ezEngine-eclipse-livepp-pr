#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <Inspector/ui_RendererWidget.h>
#include <QTreeWidgetItem>
#include <ads/DockWidget.h>

class ezQtRendererWidget : public ads::CDockWidget, public Ui_RendererWidget
{
public:
  Q_OBJECT

public:
  ezQtRendererWidget(ads::CDockManager* pDockManager, QWidget* pParent = nullptr);

  static ezQtRendererWidget* s_pWidget;

  static void ProcessTelemetry(void* pUnused);

  void ResetStats();
  void UpdateStats();

private Q_SLOTS:
  void on_ButtonRefresh_clicked();
  void on_ButtonCapture_clicked();
  void on_ButtonSave_clicked();
  void on_TreePasses_itemSelectionChanged();
  void on_ComboChannel_currentIndexChanged(int index);
  void on_SliderExposure_valueChanged(int value);
  void on_ComboDepthMode_currentIndexChanged(int index);
  void on_CheckNormalize_stateChanged(int state);

private:
  struct OutputInfo
  {
    ezString m_sPinName;
    ezUInt32 m_uiWidth = 0;
    ezUInt32 m_uiHeight = 0;
    ezString m_sFormat;
    bool m_bHasTexture = false;
  };

  struct PassInfo
  {
    ezString m_sPassName;
    ezDynamicArray<OutputInfo> m_Outputs;
    QTreeWidgetItem* m_pTreeItem = nullptr;
  };

  struct ViewInfo
  {
    ezString m_sViewName;
    ezDynamicArray<PassInfo> m_Passes;
    QTreeWidgetItem* m_pTreeItem = nullptr;
  };

  struct TextureData
  {
    ezString m_sViewName;
    ezString m_sPassName;
    ezString m_sPinName;
    ezUInt32 m_uiWidth = 0;
    ezUInt32 m_uiHeight = 0;
    ezString m_sFormat;
    ezDynamicArray<ezUInt8> m_RawPixels;     // Original data from GPU
    ezDynamicArray<ezUInt8> m_DisplayPixels; // Processed RGBA8 for display
  };

  void PopulateTreeView();
  void UpdateTextureDisplay();
  void ProcessTextureForDisplay();
  OutputInfo* GetSelectedOutput();

  ezDynamicArray<ViewInfo> m_Views;
  TextureData m_CurrentTexture;
  QImage m_DisplayImage;

  // Display options
  int m_iChannelMode = 0;   // 0=RGB, 1=R, 2=G, 3=B, 4=A, 5=Luminance
  float m_fExposure = 0.0f; // EV (-5 to +5)
  int m_iDepthMode = 0;     // 0=Linear, 1=Logarithmic
  bool m_bNormalize = false;
};
