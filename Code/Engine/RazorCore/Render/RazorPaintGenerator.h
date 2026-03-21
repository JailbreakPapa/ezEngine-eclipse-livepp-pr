#pragma once

#include <RazorCore/RazorCoreDLL.h>
#include <RazorCore/DOM/RazorElement.h>
#include <RazorCore/Render/RazorDrawCommand.h>

class ezRazorDocument;

/// Generates draw commands from a Razor document's DOM, style, and layout state.
///
/// Walks the element tree in paint order (pre-order DFS with z-index sorting)
/// and emits FillRect, BorderRect, Shadow, Text, Image, and Clip commands into
/// an ezRazorRenderBatch. Text elements are shaped via Skribidi into glyph quads.
///
/// Call `Generate()` after `ezRazorDocument::UpdateLayout()` to produce a fresh
/// batch. The generator is stateless — it reads the document each time.
class EZ_RAZORCORE_DLL ezRazorPaintGenerator
{
  EZ_DISALLOW_COPY_AND_ASSIGN(ezRazorPaintGenerator);

public:
  ezRazorPaintGenerator();
  ~ezRazorPaintGenerator();

  /// Walks the document tree and fills the batch with draw commands.
  /// The batch is cleared before generation begins.
  void Generate(const ezRazorDocument& doc, ezRazorRenderBatch& out_batch);

private:
  void PaintElement(const ezRazorDocument& doc, ezRazorElementId id,
                    const ezRectFloat& parentClip, ezInt32 iBaseOrder,
                    ezRazorRenderBatch& out_batch);

  void EmitBackground(const ezRectFloat& rect, const struct ezRazorComputedStyle& style,
                      ezInt32 iOrder, ezRazorRenderBatch& out_batch);

  void EmitBorder(const ezRectFloat& rect, const struct ezRazorComputedStyle& style,
                  ezInt32 iOrder, ezRazorRenderBatch& out_batch);

  void EmitText(const ezRazorDocument& doc, ezRazorElementId id,
                const ezRectFloat& rect, const struct ezRazorComputedStyle& style,
                ezInt32 iOrder, ezRazorRenderBatch& out_batch);

  ezInt32 m_iGlobalOrder = 0;
};
