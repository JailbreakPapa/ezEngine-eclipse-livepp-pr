#pragma once

#include <EditorEngineProcessFramework/Gizmos/GizmoHandle.h>
#include <EditorFramework/EditorFrameworkDLL.h>
#include <EditorFramework/Visualizers/VisualizerAdapter.h>

class ezRectLightVisualizerAdapter : public ezVisualizerAdapter
{
public:
  ezRectLightVisualizerAdapter();
  ~ezRectLightVisualizerAdapter();

protected:
  virtual void Finalize() override;
  virtual void Update() override;

  virtual void UpdateGizmoTransform() override;

  float m_fScale;
  float m_fWidth;
  float m_fHeight;
  ezEngineGizmoHandle m_hRangeGizmo;
  ezEngineGizmoHandle m_hRectGizmo;
};
