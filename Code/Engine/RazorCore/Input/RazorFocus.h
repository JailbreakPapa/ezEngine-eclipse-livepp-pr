#pragma once

#include <RazorCore/RazorCoreDLL.h>
#include <RazorCore/DOM/RazorElement.h>
#include <RazorCore/Input/RazorEvent.h>

#include <Foundation/Containers/DynamicArray.h>

class ezRazorDocument;
class ezRazorEventDispatcher;

/// Focus navigation direction for keyboard/gamepad navigation.
enum class ezRazorFocusDirection
{
  Forward,   ///< Tab forward
  Backward,  ///< Shift+Tab backward
  Up,
  Down,
  Left,
  Right
};

/// Manages focus state and navigation within a Razor document.
///
/// Handles:
/// - Current focus tracking
/// - Focus/blur event generation
/// - Keyboard navigation (Tab, Shift+Tab, arrows)
/// - Gamepad navigation (D-pad, stick)
/// - Focus rings and visual indicators
class EZ_RAZORCORE_DLL ezRazorFocusManager
{
  EZ_DISALLOW_COPY_AND_ASSIGN(ezRazorFocusManager);

public:
  ezRazorFocusManager();
  ~ezRazorFocusManager();

  /// Initialize with the document and event dispatcher.
  void Initialize(ezRazorDocument* pDocument, ezRazorEventDispatcher* pDispatcher);

  /// Returns the currently focused element.
  ezRazorElementId GetFocusedElement() const { return m_FocusedElement; }

  /// Sets focus to an element, dispatching focus/blur events.
  /// Returns true if focus changed.
  bool SetFocus(ezRazorElementId element);

  /// Clears focus from all elements.
  void ClearFocus();

  /// Moves focus in a direction. Returns the new focused element.
  ezRazorElementId MoveFocus(ezRazorFocusDirection direction);

  /// Returns true if the element can receive focus.
  bool IsFocusable(ezRazorElementId element) const;

  /// Returns true if the element is currently focused.
  bool HasFocus(ezRazorElementId element) const { return m_FocusedElement == element; }

  /// Returns all focusable elements in the document (in tab order).
  void GetFocusableElements(ezDynamicArray<ezRazorElementId>& out_elements) const;

  /// Gets the tab index of an element (-1 = not focusable, 0 = natural order, >0 = explicit order).
  ezInt32 GetTabIndex(ezRazorElementId element) const;

  /// Sets whether focus navigation wraps around when reaching the end.
  void SetWrapNavigation(bool bWrap) { m_bWrapNavigation = bWrap; }
  bool GetWrapNavigation() const { return m_bWrapNavigation; }

  /// Enables or disables focus ring rendering.
  void SetShowFocusRing(bool bShow) { m_bShowFocusRing = bShow; }
  bool GetShowFocusRing() const { return m_bShowFocusRing; }

private:
  ezRazorElementId FindNextFocusable(ezRazorFocusDirection direction) const;
  ezRazorElementId FindSpatialFocusable(ezRazorFocusDirection direction) const;
  void CollectFocusableElements(ezRazorElementId parent, ezDynamicArray<ezRazorElementId>& out_elements) const;
  ezInt32 GetElementTabIndex(ezRazorElementId element) const;

  void DispatchFocusEvents(ezRazorElementId oldFocus, ezRazorElementId newFocus);

  ezRazorDocument* m_pDocument = nullptr;
  ezRazorEventDispatcher* m_pDispatcher = nullptr;

  ezRazorElementId m_FocusedElement;
  bool m_bWrapNavigation = true;
  bool m_bShowFocusRing = true;
};

/// Manages pointer capture state.
///
/// When an element captures the pointer, all pointer events are directed
/// to that element until capture is released, even if the pointer moves
/// outside the element's bounds.
class EZ_RAZORCORE_DLL ezRazorPointerCapture
{
  EZ_DISALLOW_COPY_AND_ASSIGN(ezRazorPointerCapture);

public:
  ezRazorPointerCapture();
  ~ezRazorPointerCapture();

  /// Sets the element that captures all pointer events.
  void SetCapture(ezRazorElementId element);

  /// Releases pointer capture.
  void ReleaseCapture();

  /// Returns the capturing element, or invalid if none.
  ezRazorElementId GetCaptureElement() const { return m_CaptureElement; }

  /// Returns true if an element has capture.
  bool HasCapture() const { return m_CaptureElement.IsValid(); }

  /// Returns true if the specified element has capture.
  bool HasCapture(ezRazorElementId element) const { return m_CaptureElement == element; }

private:
  ezRazorElementId m_CaptureElement;
};
