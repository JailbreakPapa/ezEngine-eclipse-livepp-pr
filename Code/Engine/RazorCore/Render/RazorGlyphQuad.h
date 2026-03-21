#pragma once

#include <RazorCore/RazorCoreDLL.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Color.h>
#include <Foundation/Math/Rect.h>
#include <Foundation/Math/Vec2.h>

/// A single positioned glyph quad ready for GPU rendering.
///
/// Produced by the text shaping/layout pipeline (Skribidi).
/// Each quad maps to one glyph in a font atlas texture.
struct EZ_RAZORCORE_DLL ezRazorGlyphQuad
{
  EZ_DECLARE_POD_TYPE();

  /// Screen-space rectangle in pixels.
  ezRectFloat m_Rect;

  /// UV coordinates in the font atlas texture.
  ezVec2 m_vUVTopLeft = ezVec2::MakeZero();
  ezVec2 m_vUVBottomRight = ezVec2::MakeZero();

  /// Glyph color (inherits from computed text color).
  ezColor m_Color = ezColor::White;

  /// Which atlas texture this glyph is in (for multi-page atlases).
  ezUInt8 m_uiAtlasIndex = 0;

  /// Whether this is an SDF glyph (vs alpha mask).
  bool m_bIsSDF = false;

  /// Whether this is a color glyph (emoji).
  bool m_bIsColor = false;
};

/// A collection of pre-shaped glyph quads for a frame.
///
/// The paint generator appends glyph quads for each text node.
/// Paint nodes reference ranges in this shared array.
class EZ_RAZORCORE_DLL ezRazorGlyphBuffer
{
public:
  void Clear() { m_Quads.Clear(); }
  void Reserve(ezUInt32 uiCount) { m_Quads.Reserve(uiCount); }

  ezUInt32 GetCount() const { return m_Quads.GetCount(); }

  /// Appends a glyph quad and returns its index.
  ezUInt32 AddGlyph(const ezRazorGlyphQuad& quad)
  {
    m_Quads.PushBack(quad);
    return m_Quads.GetCount() - 1;
  }

  const ezRazorGlyphQuad& GetGlyph(ezUInt32 uiIndex) const { return m_Quads[uiIndex]; }
  const ezDynamicArray<ezRazorGlyphQuad>& GetQuads() const { return m_Quads; }

private:
  ezDynamicArray<ezRazorGlyphQuad> m_Quads;
};
