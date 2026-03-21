#pragma once

#include <RazorCore/RazorCoreDLL.h>
#include <RazorCore/Render/RazorGlyphQuad.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Color.h>
#include <Foundation/Math/Rect.h>
#include <Foundation/Math/Vec2.h>
#include <Foundation/Strings/String.h>

/// Types of draw commands that the Razor paint phase generates.
struct ezRazorDrawCommandType
{
  using StorageType = ezUInt8;

  enum Enum : ezUInt8
  {
    FillRect,   ///< Solid or gradient-filled rounded rectangle.
    BorderRect, ///< Border stroke around a rounded rectangle.
    Shadow,     ///< Box shadow (inset or outset).
    Text,       ///< Pre-shaped glyph quad run from Skribidi.
    Image,      ///< Textured quad.
    PushClip,   ///< Push a clip rectangle onto the clip stack.
    PopClip,    ///< Pop the top clip rectangle.

    ENUM_COUNT,
    Default = FillRect
  };
};

/// Gradient fill direction for FillRect commands.
struct ezRazorGradientType
{
  using StorageType = ezUInt8;

  enum Enum : ezUInt8
  {
    None,       ///< Solid fill, use m_Color only.
    LinearH,    ///< Horizontal linear gradient.
    LinearV,    ///< Vertical linear gradient.
    LinearAngle,///< Linear gradient at m_fGradientAngle degrees.
    Radial,     ///< Radial gradient from center.

    Default = None
  };
};

/// A single draw command emitted from the Razor paint tree.
///
/// Flat command structure designed for efficient batching. The renderer doesn't
/// need to know about DOM elements or styles — it only sees geometry + material state.
struct EZ_RAZORCORE_DLL ezRazorDrawCommand
{
  ezEnum<ezRazorDrawCommandType> m_Type;

  /// Target rectangle in viewport pixels.
  ezRectFloat m_Rect;

  /// Primary fill color.
  ezColor m_Color;

  /// Corner radii: TL, TR, BR, BL in pixels.
  float m_fBorderRadius[4] = {};

  /// Border widths: T, R, B, L in pixels.
  float m_fBorderWidth[4] = {};
  ezColor m_BorderColor;

  /// Opacity multiplier (already pre-multiplied from style cascade).
  float m_fOpacity = 1.0f;

  // --- Gradient (FillRect only) ---
  ezEnum<ezRazorGradientType> m_GradientType;
  ezColor m_GradientEndColor;
  float m_fGradientAngle = 0.0f; ///< Degrees, for LinearAngle.

  // --- Shadow (Shadow commands) ---
  ezVec2 m_vShadowOffset = ezVec2::MakeZero();
  float m_fShadowBlur = 0.0f;
  float m_fShadowSpread = 0.0f;
  bool m_bShadowInset = false;

  // --- Text (Text commands) ---
  /// Index range into the shared ezRazorGlyphBuffer.
  ezUInt32 m_uiGlyphStart = 0;
  ezUInt32 m_uiGlyphCount = 0;

  // --- Image (Image commands) ---
  ezUInt32 m_uiImageResourceIndex = 0;

  // --- Clip (PushClip commands) ---
  ezRectFloat m_ClipRect;

  /// Z-order for stable sorting before submission.
  ezInt32 m_iSortOrder = 0;
};

/// A collected frame of draw commands ready for renderer consumption.
///
/// The paint generator fills this on invalidation. The renderer reads it,
/// batches by material/clip/texture state, and submits GPU work.
class EZ_RAZORCORE_DLL ezRazorRenderBatch
{
public:
  void Clear();
  void AddCommand(ezRazorDrawCommand&& cmd);

  const ezDynamicArray<ezRazorDrawCommand>& GetCommands() const { return m_Commands; }
  ezDynamicArray<ezRazorDrawCommand>& GetCommands() { return m_Commands; }

  /// Returns the shared glyph buffer for text rendering.
  const ezRazorGlyphBuffer& GetGlyphBuffer() const { return m_GlyphBuffer; }
  ezRazorGlyphBuffer& GetGlyphBuffer() { return m_GlyphBuffer; }

  void SortByOrder();

private:
  ezDynamicArray<ezRazorDrawCommand> m_Commands;
  ezRazorGlyphBuffer m_GlyphBuffer;
};
