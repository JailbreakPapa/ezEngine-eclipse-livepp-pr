#pragma once

#include <RazorCore/DOM/RazorElement.h>
#include <RazorCore/RazorCoreDLL.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Color.h>
#include <Foundation/Math/Rect.h>

/// Types of paint nodes in the retained paint tree.
struct ezRazorPaintNodeType
{
  using StorageType = ezUInt8;

  enum Enum : ezUInt8
  {
    Container,  ///< Groups children, applies clip/opacity. No geometry of its own.
    Background, ///< Fills a rect with color, optional rounded corners.
    Border,     ///< Strokes the border edges of a rect.
    Text,       ///< Glyph runs from Skribidi text layout.
    Image,      ///< Textured quad from image resource.

    ENUM_COUNT,
    Default = Container
  };
};

/// A node in the retained paint tree.
///
/// Maps 1:1 with a DOM element, but carries only the visual information
/// needed for rendering. The tree is rebuilt when styles or layout change.
/// Nodes are stored in a flat array in paint order (pre-order DFS).
struct EZ_RAZORCORE_DLL ezRazorPaintNode
{
  ezEnum<ezRazorPaintNodeType> m_Type;
  ezRazorElementId m_ElementId;

  /// Layout rect in absolute viewport pixels.
  ezRectFloat m_Rect;

  /// Visual properties for this node.
  ezColor m_BackgroundColor = ezColor::MakeZero();
  ezColor m_BorderColor = ezColor::MakeZero();
  float m_fBorderRadius[4] = {}; ///< TL, TR, BR, BL in pixels.
  float m_fBorderWidth[4] = {};  ///< T, R, B, L in pixels.
  float m_fOpacity = 1.0f;

  /// Clip rect in absolute viewport pixels. Invalid means no custom clip.
  ezRectFloat m_ClipRect;
  bool m_bHasClip = false;

  /// Z-order for stable sorting.
  ezInt32 m_iZIndex = 0;

  /// For text nodes: index range into the shared glyph quad array.
  ezUInt32 m_uiGlyphStart = 0;
  ezUInt32 m_uiGlyphCount = 0;

  /// For image nodes: resource index in the renderer's texture table.
  ezUInt32 m_uiImageResourceIndex = 0;
};
