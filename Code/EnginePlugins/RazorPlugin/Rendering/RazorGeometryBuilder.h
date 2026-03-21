#pragma once

#include <RazorPlugin/RazorPluginDLL.h>
#include <RazorPlugin/Rendering/RazorRenderData.h>

class ezRazorRenderBatch;
class ezRazorRenderData;

/// Converts Razor draw commands into GPU-ready vertex/index data and draw calls.
///
/// Processes an ezRazorRenderBatch and produces the flat vertex/index arrays
/// plus segmented draw calls (with mode, texture, and scissor state) stored
/// in an ezRazorRenderData. This is the bridge between RazorCore's paint system
/// and the GPU renderer.
class EZ_RAZORPLUGIN_DLL ezRazorGeometryBuilder
{
  EZ_DISALLOW_COPY_AND_ASSIGN(ezRazorGeometryBuilder);

public:
  ezRazorGeometryBuilder();
  ~ezRazorGeometryBuilder();

  /// Converts the batch commands into vertex/index data and draw calls.
  void Build(const ezRazorRenderBatch& batch, ezRazorRenderData& out_renderData);

private:
  /// Emits a filled rectangle (optionally with rounded corners).
  void EmitFilledRect(const ezRectFloat& rect, const ezColor& color, float fOpacity,
                      const float* fRadius, ezRazorRenderData& out);

  /// Emits a border rectangle.
  void EmitBorderRect(const ezRectFloat& rect, const ezColor& color, float fOpacity,
                      const float* fBorderWidth, const float* fRadius, ezRazorRenderData& out);

  /// Emits glyph quads for a text draw command.
  void EmitGlyphQuads(const struct ezRazorDrawCommand& cmd, const class ezRazorGlyphBuffer& glyphs,
                      ezRazorRenderData& out);

  /// Adds a quad (2 triangles, 4 vertices, 6 indices).
  void AddQuad(float x, float y, float w, float h,
               float u0, float v0, float u1, float v1,
               ezUInt32 uiColor, ezRazorRenderData& out);

  /// Pushes a clip rect onto the stack.
  void PushClip(const ezRectFloat& rect);
  void PopClip();

  /// Flushes the current draw call if the mode changed.
  void FlushDrawCallIfModeChanged(ezRazorDrawCall::Mode mode, ezRazorRenderData& out);

  /// Flushes the current pending draw call.
  void FlushCurrentDrawCall(ezRazorRenderData& out);

  /// Current clip rect (intersection of all active clips).
  ezRectFloat m_CurrentClip;
  ezHybridArray<ezRectFloat, 8> m_ClipStack;
  bool m_bHasClip = false;

  /// Tracks draw call state changes.
  ezRazorDrawCall::Mode m_CurrentMode = ezRazorDrawCall::Mode::Solid;
  ezUInt32 m_uiDrawCallStartIndex = 0;
};
