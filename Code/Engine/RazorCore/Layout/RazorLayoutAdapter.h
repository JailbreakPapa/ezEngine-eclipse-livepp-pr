#pragma once

#include <RazorCore/RazorCoreDLL.h>
#include <RazorCore/Style/RazorComputedStyle.h>

#include <Foundation/Math/Rect.h>
#include <Foundation/Math/Vec2.h>

class ezRazorDocument;
struct ezRazorElementId;

/// Abstract layout adapter interface.
///
/// Sits between the Razor style/DOM layer and the concrete layout backend (Yoga in v1).
/// This abstraction keeps Yoga types out of Razor's public API so the backend can evolve
/// without breaking downstream code.
class EZ_RAZORCORE_DLL ezRazorLayoutAdapter
{
public:
  virtual ~ezRazorLayoutAdapter() = default;

  /// Prepare/create internal layout nodes for every element in the document.
  virtual void SyncFromDocument(const ezRazorDocument& document) = 0;

  /// Apply computed styles to layout nodes (flex direction, sizing, margins, etc.).
  virtual void ApplyStyles(const ezRazorDocument& document) = 0;

  /// Execute the layout pass and write results into the provided rect array.
  virtual void ComputeLayout(const ezVec2& vViewportSize, ezDynamicArray<ezRectFloat>& out_layoutRects) = 0;

  /// Invalidate layout for a specific subtree root. Pass an invalid ID to invalidate everything.
  virtual void Invalidate(ezRazorElementId subtreeRoot) = 0;
};
