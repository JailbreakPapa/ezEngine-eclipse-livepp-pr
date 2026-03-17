#pragma once

#include <EditorEngineProcessFramework/Gizmos/GizmoHandle.h>
#include <EditorFramework/EditorFrameworkDLL.h>
#include <EditorFramework/Visualizers/VisualizerAdapter.h>

class ezTubeLightVisualizerAdapter : public ezVisualizerAdapter
{
public:
  ezTubeLightVisualizerAdapter();
  ~ezTubeLightVisualizerAdapter();

protected:
  virtual void Finalize() override;
  virtual void Update() override;

  virtual void UpdateGizmoTransform() override;

  float m_fScale;
  float m_fLength;
  float m_fRadius;
  ezEngineGizmoHandle m_hRangeGizmo;
};
