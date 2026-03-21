#pragma once

#include <RazorCore/DOM/RazorElement.h>
#include <RazorCore/RazorCoreDLL.h>
#include <RazorCore/Style/RazorComputedStyle.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Rect.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/UniquePtr.h>

class ezRazorStyleSheet;
class ezRazorLayoutAdapter;
class ezRazorYogaLayoutAdapter;
class ezStreamWriter;
class ezStreamReader;

/// A live instance of a Razor UI document.
///
/// Owns the element tree plus parallel arrays for computed styles and layout rectangles.
/// Created from a compiled template at runtime — the document itself never parses XML/CSS.
/// The element pool uses a flat array with free-list recycling.
class EZ_RAZORCORE_DLL ezRazorDocument
{
  EZ_DISALLOW_COPY_AND_ASSIGN(ezRazorDocument);

public:
  ezRazorDocument();
  ~ezRazorDocument();

  /// Creates a new element and returns its ID. The element is not yet attached to the tree.
  ezRazorElementId CreateElement(ezStringView sTag);

  /// Removes an element and all its descendants from the document.
  void DestroyElement(ezRazorElementId id);

  /// Appends a child element to a parent. The child must not already be in the tree.
  void AppendChild(ezRazorElementId parent, ezRazorElementId child);

  /// Returns a pointer to the element. Null if the ID is invalid or has been destroyed.
  const ezRazorElement* GetElement(ezRazorElementId id) const;
  ezRazorElement* GetElement(ezRazorElementId id);

  /// The root element of the document. Invalid until the first element is created.
  ezRazorElementId GetRootElement() const { return m_RootElement; }
  void SetRootElement(ezRazorElementId id) { m_RootElement = id; }

  /// Adds a stylesheet to be applied during style resolution. Order matters for specificity.
  void AddStyleSheet(const ezRazorStyleSheet* pStyleSheet);

  /// Triggers style resolution and layout computation for dirty subtrees.
  /// Returns true if layout was actually recomputed.
  bool UpdateLayout(const ezVec2& vViewportSize);

  /// Returns a monotonically increasing version that increments when layout changes.
  ezUInt64 GetLayoutVersion() const { return m_uiLayoutVersion; }

  /// Returns the computed style for an element after the last UpdateLayout pass.
  const ezRazorComputedStyle* GetComputedStyle(ezRazorElementId id) const;

  /// Returns the final layout rectangle in pixels for an element.
  const ezRectFloat* GetLayoutRect(ezRazorElementId id) const;

  /// Marks the entire style and layout tree as dirty, forcing a full recalculation on the next update.
  void InvalidateAll();

  ezUInt32 GetElementCount() const { return m_uiElementCount; }

  /// Serializes the document's element tree to a stream.
  ezResult Serialize(ezStreamWriter& inout_stream) const;

  /// Deserializes a document's element tree from a stream.
  ezResult Deserialize(ezStreamReader& inout_stream);

private:
  ezRazorElementId AllocateElement();
  void FreeElement(ezRazorElementId id);

  ezDynamicArray<ezRazorElement> m_Elements;
  ezDynamicArray<ezRazorComputedStyle> m_ComputedStyles;
  ezDynamicArray<ezRectFloat> m_LayoutRects;
  ezDynamicArray<ezUInt32> m_FreeList;

  ezRazorElementId m_RootElement;
  ezUInt32 m_uiElementCount = 0;

  ezHybridArray<const ezRazorStyleSheet*, 4> m_StyleSheets;
  ezUniquePtr<ezRazorYogaLayoutAdapter> m_pYogaAdapter;

  bool m_bStyleDirty = true;
  bool m_bLayoutDirty = true;
  ezUInt64 m_uiLayoutVersion = 0;
};
