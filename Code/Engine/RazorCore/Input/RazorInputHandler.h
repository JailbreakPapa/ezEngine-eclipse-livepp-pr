#pragma once

#include <RazorCore/RazorCoreDLL.h>
#include <RazorCore/Input/RazorEvent.h>
#include <RazorCore/Input/RazorEventDispatcher.h>
#include <RazorCore/Input/RazorFocus.h>
#include <RazorCore/Input/RazorHitTest.h>

#include <Foundation/Math/Vec2.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Types/UniquePtr.h>

class ezRazorDocument;

/// Configuration for the input handler.
struct EZ_RAZORCORE_DLL ezRazorInputConfig
{
  float m_fDoubleClickTime = 0.3f;        ///< Maximum time between clicks for double-click (seconds)
  float m_fDoubleClickDistance = 4.0f;    ///< Maximum distance between clicks for double-click (pixels)
  float m_fLongPressTime = 0.5f;          ///< Time for long press detection (seconds)
  float m_fDragThreshold = 4.0f;          ///< Minimum distance to start a drag (pixels)
  float m_fGamepadDeadzone = 0.2f;        ///< Analog stick deadzone
  float m_fGamepadRepeatDelay = 0.5f;     ///< Initial delay before repeating (seconds)
  float m_fGamepadRepeatRate = 0.1f;      ///< Rate of repeated navigation (seconds)
};

/// Central input processing for a Razor document.
///
/// Receives raw input from the platform and converts it to Razor events,
/// handling:
/// - Mouse/touch hit testing and event generation
/// - Keyboard event routing to focused element
/// - Gamepad navigation and button events
/// - Click, double-click, and drag detection
/// - Pointer capture
/// - Focus management
class EZ_RAZORCORE_DLL ezRazorInputHandler
{
  EZ_DISALLOW_COPY_AND_ASSIGN(ezRazorInputHandler);

public:
  ezRazorInputHandler();
  ~ezRazorInputHandler();

  /// Initialize with a document.
  void SetDocument(ezRazorDocument* pDocument);

  /// Gets the event dispatcher for registering handlers.
  ezRazorEventDispatcher* GetEventDispatcher() { return &m_EventDispatcher; }

  /// Gets the focus manager.
  ezRazorFocusManager* GetFocusManager() { return &m_FocusManager; }

  /// Gets the pointer capture state.
  ezRazorPointerCapture* GetPointerCapture() { return &m_PointerCapture; }

  /// Gets the hit tester.
  ezRazorHitTester* GetHitTester() { return &m_HitTester; }

  /// Configuration settings.
  ezRazorInputConfig& GetConfig() { return m_Config; }
  const ezRazorInputConfig& GetConfig() const { return m_Config; }

  // --- Mouse Input ---

  /// Process mouse movement. Returns the element now under the cursor.
  ezRazorElementId OnMouseMove(const ezVec2& vPosition, const ezRazorModifierState& modifiers);

  /// Process mouse button press.
  void OnMouseDown(ezRazorMouseButton button, const ezVec2& vPosition, const ezRazorModifierState& modifiers);

  /// Process mouse button release.
  void OnMouseUp(ezRazorMouseButton button, const ezVec2& vPosition, const ezRazorModifierState& modifiers);

  /// Process mouse wheel.
  void OnMouseWheel(const ezVec2& vDelta, const ezVec2& vPosition, const ezRazorModifierState& modifiers);

  /// Called when the mouse leaves the viewport.
  void OnMouseLeave();

  // --- Keyboard Input ---

  /// Process key press.
  void OnKeyDown(ezUInt32 uiKeyCode, ezUInt32 uiCharacter, bool bRepeat, const ezRazorModifierState& modifiers);

  /// Process key release.
  void OnKeyUp(ezUInt32 uiKeyCode, const ezRazorModifierState& modifiers);

  // --- Touch Input ---

  /// Process touch start.
  void OnTouchStart(ezArrayPtr<const ezRazorTouchPoint> touches);

  /// Process touch move.
  void OnTouchMove(ezArrayPtr<const ezRazorTouchPoint> touches);

  /// Process touch end.
  void OnTouchEnd(ezArrayPtr<const ezRazorTouchPoint> touches);

  /// Process touch cancel.
  void OnTouchCancel(ezArrayPtr<const ezRazorTouchPoint> touches);

  // --- Gamepad Input ---

  /// Process gamepad button press.
  void OnGamepadButtonDown(ezUInt8 uiGamepadIndex, ezRazorGamepadButton button, float fValue);

  /// Process gamepad button release.
  void OnGamepadButtonUp(ezUInt8 uiGamepadIndex, ezRazorGamepadButton button);

  /// Process gamepad axis change.
  void OnGamepadAxis(ezUInt8 uiGamepadIndex, ezRazorGamepadAxis axis, float fValue);

  // --- Frame Update ---

  /// Call once per frame for time-based input handling (long press, repeat, etc.)
  void Update(ezTime deltaTime);

  // --- State Queries ---

  /// Returns the element currently under the pointer.
  ezRazorElementId GetHoveredElement() const { return m_HoverTracker.GetHoveredElement(); }

  /// Returns the mouse position in viewport coordinates.
  ezVec2 GetMousePosition() const { return m_vMousePosition; }

  /// Returns true if a mouse button is currently pressed.
  bool IsMouseButtonDown(ezRazorMouseButton button) const;

private:
  void HandleClick(ezRazorElementId target, const ezVec2& vPosition, const ezRazorModifierState& modifiers);
  void HandleNavigation(ezUInt32 uiKeyCode, const ezRazorModifierState& modifiers);
  void HandleGamepadNavigation();

  ezRazorDocument* m_pDocument = nullptr;

  // Subsystems
  ezRazorEventDispatcher m_EventDispatcher;
  ezRazorFocusManager m_FocusManager;
  ezRazorPointerCapture m_PointerCapture;
  ezRazorHitTester m_HitTester;
  ezRazorHoverTracker m_HoverTracker;

  ezRazorInputConfig m_Config;

  // Mouse state
  ezVec2 m_vMousePosition = ezVec2::MakeZero();
  ezUInt8 m_uiMouseButtons = 0;  // Bitmask of pressed buttons
  ezRazorElementId m_MouseDownTarget;
  ezVec2 m_vMouseDownPosition;
  ezTime m_MouseDownTime;

  // Double-click detection
  ezTime m_LastClickTime;
  ezVec2 m_vLastClickPosition;
  ezRazorElementId m_LastClickTarget;

  // Touch state
  ezHybridArray<ezRazorTouchPoint, 2> m_ActiveTouches;

  // Gamepad state
  struct GamepadState
  {
    float m_fAxes[(int)ezRazorGamepadAxis::Count] = {};
    bool m_bButtons[(int)ezRazorGamepadButton::Count] = {};
    ezTime m_NextRepeatTime;
    ezRazorFocusDirection m_RepeatDirection = ezRazorFocusDirection::Forward;
    bool m_bRepeating = false;
  };
  GamepadState m_GamepadStates[4];
};
