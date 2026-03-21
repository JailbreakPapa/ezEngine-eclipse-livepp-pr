#pragma once

#include <RazorCore/RazorCoreDLL.h>
#include <RazorCore/DOM/RazorElement.h>

#include <Foundation/Math/Vec2.h>
#include <Foundation/Math/Rect.h>
#include <Foundation/Containers/DynamicArray.h>

class ezRazorDocument;

/// Result of a hit test query.
struct EZ_RAZORCORE_DLL ezRazorHitTestResult
{
  ezRazorElementId m_Element;           ///< The topmost element hit (invalid if nothing hit)
  ezVec2 m_vLocalPosition;              ///< Position relative to the hit element
  float m_fDistance = 0.0f;             ///< Distance for 3D hit tests (0 for 2D)
};

/// Options for hit testing.
struct EZ_RAZORCORE_DLL ezRazorHitTestOptions
{
  bool m_bIgnorePointerEvents = false;  ///< If true, respects pointer-events:none CSS
  bool m_bIncludeInvisible = false;     ///< If true, tests against visibility:hidden elements
  bool m_bIncludeClipped = true;        ///< If true, tests against overflow:hidden clipped areas
};

/// Performs hit testing against the document's layout tree.
///
/// Hit testing walks the visual tree in reverse paint order (front-to-back)
/// to find the topmost element under a point. Elements can be excluded
/// based on CSS pointer-events, visibility, and clipping.
class EZ_RAZORCORE_DLL ezRazorHitTester
{
  EZ_DISALLOW_COPY_AND_ASSIGN(ezRazorHitTester);

public:
  ezRazorHitTester();
  ~ezRazorHitTester();

  /// Sets the document to test against.
  void SetDocument(ezRazorDocument* pDocument) { m_pDocument = pDocument; }

  /// Performs a hit test at the given viewport coordinates.
  /// Returns the topmost element under the point, or an invalid element if nothing was hit.
  ezRazorHitTestResult HitTest(const ezVec2& vViewportPosition, const ezRazorHitTestOptions& options = {}) const;

  /// Performs a hit test and returns all elements under the point (front-to-back order).
  void HitTestAll(const ezVec2& vViewportPosition, ezDynamicArray<ezRazorHitTestResult>& out_results, const ezRazorHitTestOptions& options = {}) const;

  /// Tests if a point is inside an element's layout rectangle.
  bool IsPointInElement(const ezVec2& vViewportPosition, ezRazorElementId element) const;

  /// Returns the element path from root to the given element (for debugging).
  void GetElementPath(ezRazorElementId element, ezDynamicArray<ezRazorElementId>& out_path) const;

private:
  bool HitTestElement(
    ezRazorElementId element,
    const ezVec2& vViewportPosition,
    const ezRazorHitTestOptions& options,
    ezRazorHitTestResult& out_result) const;

  void HitTestElementRecursive(
    ezRazorElementId element,
    const ezVec2& vViewportPosition,
    const ezRazorHitTestOptions& options,
    ezDynamicArray<ezRazorHitTestResult>& out_results) const;

  bool ShouldTestElement(ezRazorElementId element, const ezRazorHitTestOptions& options) const;

  ezRazorDocument* m_pDocument = nullptr;
};

/// Tracks which element the pointer is currently over.
///
/// Maintains state for generating mouseenter/mouseleave and mouseover/mouseout
/// events when the pointer moves between elements.
class EZ_RAZORCORE_DLL ezRazorHoverTracker
{
  EZ_DISALLOW_COPY_AND_ASSIGN(ezRazorHoverTracker);

public:
  ezRazorHoverTracker();
  ~ezRazorHoverTracker();

  /// Sets the document and hit tester to use.
  void Initialize(ezRazorDocument* pDocument, ezRazorHitTester* pHitTester);

  /// Updates hover state based on new pointer position.
  /// Returns the element now under the pointer.
  ezRazorElementId UpdatePointerPosition(const ezVec2& vViewportPosition);

  /// Gets the element currently under the pointer.
  ezRazorElementId GetHoveredElement() const { return m_HoveredElement; }

  /// Gets elements that the pointer entered since the last update (for mouseenter events).
  ezArrayPtr<const ezRazorElementId> GetEnteredElements() const { return m_EnteredElements; }

  /// Gets elements that the pointer left since the last update (for mouseleave events).
  ezArrayPtr<const ezRazorElementId> GetLeftElements() const { return m_LeftElements; }

  /// Clears hover state (e.g., when pointer leaves the viewport).
  void ClearHover();

private:
  void ComputeAncestorPath(ezRazorElementId element, ezDynamicArray<ezRazorElementId>& out_path);

  ezRazorDocument* m_pDocument = nullptr;
  ezRazorHitTester* m_pHitTester = nullptr;

  ezRazorElementId m_HoveredElement;
  ezHybridArray<ezRazorElementId, 8> m_HoverPath;  // Current elements under pointer (leaf to root)

  ezHybridArray<ezRazorElementId, 4> m_EnteredElements;
  ezHybridArray<ezRazorElementId, 4> m_LeftElements;
};
