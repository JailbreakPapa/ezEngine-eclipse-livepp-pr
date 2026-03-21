#pragma once

#include <RazorPlugin/Components/RazorCanvasComponentBase.h>

#include <RazorCore/Render/RazorDrawCommand.h>
#include <RazorCore/Render/RazorPaintGenerator.h>

class ezRazorGeometryBuilder;

using ezRazorCanvas2DComponentManager = ezComponentManagerSimple<class ezRazorCanvas2DComponent, ezComponentUpdateType::Always, ezBlockStorageType::Compact, ezWorldUpdatePhase::PostTransform>;

/// Renders a Razor UI document as a 2D screen-space overlay.
///
/// Each frame, generates draw commands from the document's DOM/style/layout,
/// converts them to GPU-ready vertex/index data, and submits to the render pipeline.
class EZ_RAZORPLUGIN_DLL ezRazorCanvas2DComponent : public ezRazorCanvasComponentBase
{
  EZ_DECLARE_COMPONENT_TYPE(ezRazorCanvas2DComponent, ezRazorCanvasComponentBase, ezRazorCanvas2DComponentManager);

public:
  ezRazorCanvas2DComponent();
  ~ezRazorCanvas2DComponent();

  ezRazorCanvas2DComponent& operator=(ezRazorCanvas2DComponent&& rhs);

  virtual void OnActivated() override;
  void Update() final override;

  void SetOffset(const ezVec2I32& vOffset);                // [ property ]
  const ezVec2I32& GetOffset() const { return m_vOffset; } // [ property ]

  void SetSize(const ezVec2U32& vSize);                    // [ property ]
  const ezVec2U32& GetSize() const { return m_vSize; }     // [ property ]

  void SetPassInput(bool bPassInput);                      // [ property ]
  bool GetPassInput() const { return m_bPassInput; }       // [ property ]

  virtual void SerializeComponent(ezWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(ezWorldReader& inout_stream) override;

protected:
  void OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const override;

  ezVec2I32 m_vOffset = ezVec2I32::MakeZero();
  bool m_bPassInput = true;

  ezRazorPaintGenerator m_PaintGenerator;
  mutable ezRazorRenderBatch m_RenderBatch; ///< Cached, rebuilt on invalidation.
  mutable bool m_bPaintDirty = true;
};
