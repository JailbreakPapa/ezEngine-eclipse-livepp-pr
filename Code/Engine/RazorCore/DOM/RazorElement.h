#pragma once

#include <RazorCore/RazorCoreDLL.h>

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/UniquePtr.h>

/// Identifies an element within a Razor document.
/// Zero is reserved as the invalid/null ID.
struct ezRazorElementId
{
  EZ_DECLARE_POD_TYPE();

  ezUInt32 m_uiIndex = 0;

  bool IsValid() const { return m_uiIndex != 0; }
  bool operator==(const ezRazorElementId& rhs) const { return m_uiIndex == rhs.m_uiIndex; }
  bool operator!=(const ezRazorElementId& rhs) const { return m_uiIndex != rhs.m_uiIndex; }
};

template <>
struct ezHashHelper<ezRazorElementId>
{
  EZ_ALWAYS_INLINE static ezUInt32 Hash(const ezRazorElementId& value) { return ezHashHelper<ezUInt32>::Hash(value.m_uiIndex); }
  EZ_ALWAYS_INLINE static bool Equal(const ezRazorElementId& a, const ezRazorElementId& b) { return a == b; }
};

/// A single element in the Razor DOM tree.
///
/// Elements hold their tag name, class list, inline style overrides, and structural
/// relationships (parent, first child, next sibling). The actual style resolution and
/// layout data live in parallel arrays managed by the owning document — an element by
/// itself is a lightweight node.
struct EZ_RAZORCORE_DLL ezRazorElement
{
  ezHashedString m_sTag;
  ezHashedString m_sId;
  ezHybridArray<ezHashedString, 2> m_Classes;

  ezRazorElementId m_Parent;
  ezRazorElementId m_FirstChild;
  ezRazorElementId m_NextSibling;

  /// Text content for text nodes. Empty for container elements.
  ezString m_sTextContent;
};
