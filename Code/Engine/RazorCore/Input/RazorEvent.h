#pragma once

#include <RazorCore/RazorCoreDLL.h>
#include <RazorCore/DOM/RazorElement.h>

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Math/Vec2.h>

/// Event propagation phase.
enum class ezRazorEventPhase
{
  None,        ///< Event is not currently dispatching
  Capture,     ///< Traveling down from root to target
  Target,      ///< At the target element
  Bubble       ///< Traveling up from target to root
};

/// Common event types used by Razor UI.
struct EZ_RAZORCORE_DLL ezRazorEventType
{
  static const ezHashedString Click;
  static const ezHashedString DblClick;
  static const ezHashedString MouseDown;
  static const ezHashedString MouseUp;
  static const ezHashedString MouseMove;
  static const ezHashedString MouseEnter;
  static const ezHashedString MouseLeave;
  static const ezHashedString MouseOver;
  static const ezHashedString MouseOut;
  static const ezHashedString Wheel;
  static const ezHashedString KeyDown;
  static const ezHashedString KeyUp;
  static const ezHashedString Focus;
  static const ezHashedString Blur;
  static const ezHashedString FocusIn;
  static const ezHashedString FocusOut;
  static const ezHashedString Input;
  static const ezHashedString Change;
  static const ezHashedString Submit;
  static const ezHashedString Scroll;
  static const ezHashedString TouchStart;
  static const ezHashedString TouchMove;
  static const ezHashedString TouchEnd;
  static const ezHashedString TouchCancel;
  static const ezHashedString GamepadButtonDown;
  static const ezHashedString GamepadButtonUp;
  static const ezHashedString GamepadAxis;
};

/// Base class for all Razor UI events.
///
/// Events propagate through the DOM tree in capture and bubble phases.
/// Handlers can stop propagation or prevent default behavior.
class EZ_RAZORCORE_DLL ezRazorEvent : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorEvent, ezReflectedClass);

public:
  ezRazorEvent();
  explicit ezRazorEvent(ezHashedString sType, bool bBubbles = true, bool bCancelable = true);
  virtual ~ezRazorEvent() = default;

  /// The event type name (e.g., "click", "keydown").
  ezHashedString GetType() const { return m_sType; }

  /// The element that the event was originally dispatched to.
  ezRazorElementId GetTarget() const { return m_Target; }
  void SetTarget(ezRazorElementId target) { m_Target = target; }

  /// The element whose handler is currently being invoked.
  ezRazorElementId GetCurrentTarget() const { return m_CurrentTarget; }
  void SetCurrentTarget(ezRazorElementId current) { m_CurrentTarget = current; }

  /// Current propagation phase.
  ezRazorEventPhase GetPhase() const { return m_Phase; }
  void SetPhase(ezRazorEventPhase phase) { m_Phase = phase; }

  /// Whether the event bubbles up through ancestors.
  bool GetBubbles() const { return m_bBubbles; }

  /// Whether the event's default action can be prevented.
  bool GetCancelable() const { return m_bCancelable; }

  /// Stops the event from propagating to other elements.
  void StopPropagation() { m_bPropagationStopped = true; }

  /// Stops the event from propagating and prevents other listeners on the current element.
  void StopImmediatePropagation() { m_bImmediatePropagationStopped = true; m_bPropagationStopped = true; }

  /// Cancels the default action for this event.
  void PreventDefault() { if (m_bCancelable) m_bDefaultPrevented = true; }

  bool IsPropagationStopped() const { return m_bPropagationStopped; }
  bool IsImmediatePropagationStopped() const { return m_bImmediatePropagationStopped; }
  bool IsDefaultPrevented() const { return m_bDefaultPrevented; }

  /// Timestamp when the event was created.
  ezTime GetTimestamp() const { return m_Timestamp; }

protected:
  ezHashedString m_sType;
  ezRazorElementId m_Target;
  ezRazorElementId m_CurrentTarget;
  ezRazorEventPhase m_Phase = ezRazorEventPhase::None;
  ezTime m_Timestamp;

  bool m_bBubbles = true;
  bool m_bCancelable = true;
  bool m_bPropagationStopped = false;
  bool m_bImmediatePropagationStopped = false;
  bool m_bDefaultPrevented = false;
};

/// Modifier key state shared by mouse and keyboard events.
struct EZ_RAZORCORE_DLL ezRazorModifierState
{
  EZ_DECLARE_POD_TYPE();

  bool m_bCtrl = false;
  bool m_bShift = false;
  bool m_bAlt = false;
  bool m_bMeta = false;  ///< Windows/Command key
};

/// Mouse button identifiers.
enum class ezRazorMouseButton : ezUInt8
{
  None = 0,
  Left = 1,
  Middle = 2,
  Right = 3,
  X1 = 4,
  X2 = 5
};

/// Mouse-related events (click, move, enter, leave, etc.)
class EZ_RAZORCORE_DLL ezRazorMouseEvent : public ezRazorEvent
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorMouseEvent, ezRazorEvent);

public:
  ezRazorMouseEvent();
  ezRazorMouseEvent(ezHashedString sType, const ezVec2& vClientPos, ezRazorMouseButton button = ezRazorMouseButton::None);

  /// Position relative to the document viewport.
  ezVec2 GetClientPosition() const { return m_vClientPosition; }
  void SetClientPosition(const ezVec2& vPos) { m_vClientPosition = vPos; }

  /// Position relative to the target element.
  ezVec2 GetOffsetPosition() const { return m_vOffsetPosition; }
  void SetOffsetPosition(const ezVec2& vPos) { m_vOffsetPosition = vPos; }

  /// Which mouse button triggered the event (for click events).
  ezRazorMouseButton GetButton() const { return m_Button; }
  void SetButton(ezRazorMouseButton button) { m_Button = button; }

  /// Bitmask of currently pressed buttons.
  ezUInt8 GetButtons() const { return m_uiButtons; }
  void SetButtons(ezUInt8 buttons) { m_uiButtons = buttons; }

  /// Modifier key state.
  const ezRazorModifierState& GetModifiers() const { return m_Modifiers; }
  void SetModifiers(const ezRazorModifierState& mods) { m_Modifiers = mods; }

  /// The element the mouse moved from (for enter/leave events).
  ezRazorElementId GetRelatedTarget() const { return m_RelatedTarget; }
  void SetRelatedTarget(ezRazorElementId related) { m_RelatedTarget = related; }

private:
  ezVec2 m_vClientPosition = ezVec2::MakeZero();
  ezVec2 m_vOffsetPosition = ezVec2::MakeZero();
  ezRazorMouseButton m_Button = ezRazorMouseButton::None;
  ezUInt8 m_uiButtons = 0;
  ezRazorModifierState m_Modifiers;
  ezRazorElementId m_RelatedTarget;
};

/// Wheel/scroll events.
class EZ_RAZORCORE_DLL ezRazorWheelEvent : public ezRazorMouseEvent
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorWheelEvent, ezRazorMouseEvent);

public:
  ezRazorWheelEvent();
  ezRazorWheelEvent(const ezVec2& vClientPos, const ezVec2& vDelta);

  /// Scroll delta in pixels (negative = up/left, positive = down/right).
  ezVec2 GetDelta() const { return m_vDelta; }
  void SetDelta(const ezVec2& vDelta) { m_vDelta = vDelta; }

private:
  ezVec2 m_vDelta = ezVec2::MakeZero();
};

/// Keyboard events.
class EZ_RAZORCORE_DLL ezRazorKeyboardEvent : public ezRazorEvent
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorKeyboardEvent, ezRazorEvent);

public:
  ezRazorKeyboardEvent();
  ezRazorKeyboardEvent(ezHashedString sType, ezUInt32 uiKeyCode);

  /// Platform-independent key code.
  ezUInt32 GetKeyCode() const { return m_uiKeyCode; }
  void SetKeyCode(ezUInt32 code) { m_uiKeyCode = code; }

  /// The character produced by the key, if any.
  ezUInt32 GetCharacter() const { return m_uiCharacter; }
  void SetCharacter(ezUInt32 ch) { m_uiCharacter = ch; }

  /// Whether this is a key repeat.
  bool IsRepeat() const { return m_bRepeat; }
  void SetRepeat(bool repeat) { m_bRepeat = repeat; }

  /// Modifier key state.
  const ezRazorModifierState& GetModifiers() const { return m_Modifiers; }
  void SetModifiers(const ezRazorModifierState& mods) { m_Modifiers = mods; }

private:
  ezUInt32 m_uiKeyCode = 0;
  ezUInt32 m_uiCharacter = 0;
  bool m_bRepeat = false;
  ezRazorModifierState m_Modifiers;
};

/// Focus-related events.
class EZ_RAZORCORE_DLL ezRazorFocusEvent : public ezRazorEvent
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorFocusEvent, ezRazorEvent);

public:
  ezRazorFocusEvent();
  ezRazorFocusEvent(ezHashedString sType, ezRazorElementId relatedTarget);

  /// The element losing focus (for focus events) or gaining focus (for blur events).
  ezRazorElementId GetRelatedTarget() const { return m_RelatedTarget; }
  void SetRelatedTarget(ezRazorElementId related) { m_RelatedTarget = related; }

private:
  ezRazorElementId m_RelatedTarget;
};

/// Touch point information.
struct EZ_RAZORCORE_DLL ezRazorTouchPoint
{
  EZ_DECLARE_POD_TYPE();

  ezInt32 m_iIdentifier = 0;      ///< Unique touch ID for tracking
  ezVec2 m_vClientPosition;       ///< Position relative to viewport
  ezVec2 m_vScreenPosition;       ///< Position in screen coordinates
  float m_fRadiusX = 1.0f;        ///< Contact area X radius
  float m_fRadiusY = 1.0f;        ///< Contact area Y radius
  float m_fForce = 0.0f;          ///< Pressure (0.0 to 1.0)
  ezRazorElementId m_Target;      ///< Element the touch started on
};

/// Touch events for mobile/touch-screen input.
class EZ_RAZORCORE_DLL ezRazorTouchEvent : public ezRazorEvent
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorTouchEvent, ezRazorEvent);

public:
  ezRazorTouchEvent();
  ezRazorTouchEvent(ezHashedString sType);

  /// All active touch points.
  ezArrayPtr<const ezRazorTouchPoint> GetTouches() const { return m_Touches; }
  void AddTouch(const ezRazorTouchPoint& touch) { m_Touches.PushBack(touch); }

  /// Touch points that changed in this event.
  ezArrayPtr<const ezRazorTouchPoint> GetChangedTouches() const { return m_ChangedTouches; }
  void AddChangedTouch(const ezRazorTouchPoint& touch) { m_ChangedTouches.PushBack(touch); }

  /// Modifier key state.
  const ezRazorModifierState& GetModifiers() const { return m_Modifiers; }
  void SetModifiers(const ezRazorModifierState& mods) { m_Modifiers = mods; }

private:
  ezHybridArray<ezRazorTouchPoint, 2> m_Touches;
  ezHybridArray<ezRazorTouchPoint, 2> m_ChangedTouches;
  ezRazorModifierState m_Modifiers;
};

/// Gamepad button identifiers (Xbox-style layout).
enum class ezRazorGamepadButton : ezUInt8
{
  A = 0,
  B = 1,
  X = 2,
  Y = 3,
  LeftBumper = 4,
  RightBumper = 5,
  Back = 6,
  Start = 7,
  LeftStick = 8,
  RightStick = 9,
  DPadUp = 10,
  DPadDown = 11,
  DPadLeft = 12,
  DPadRight = 13,
  Guide = 14,

  Count
};

/// Gamepad button events.
class EZ_RAZORCORE_DLL ezRazorGamepadButtonEvent : public ezRazorEvent
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorGamepadButtonEvent, ezRazorEvent);

public:
  ezRazorGamepadButtonEvent();
  ezRazorGamepadButtonEvent(ezHashedString sType, ezUInt8 uiGamepadIndex, ezRazorGamepadButton button);

  ezUInt8 GetGamepadIndex() const { return m_uiGamepadIndex; }
  ezRazorGamepadButton GetButton() const { return m_Button; }
  
  /// Value 0.0-1.0 for analog buttons like triggers.
  float GetValue() const { return m_fValue; }
  void SetValue(float value) { m_fValue = value; }

private:
  ezUInt8 m_uiGamepadIndex = 0;
  ezRazorGamepadButton m_Button = ezRazorGamepadButton::A;
  float m_fValue = 0.0f;
};

/// Gamepad axis identifiers.
enum class ezRazorGamepadAxis : ezUInt8
{
  LeftStickX = 0,
  LeftStickY = 1,
  RightStickX = 2,
  RightStickY = 3,
  LeftTrigger = 4,
  RightTrigger = 5,

  Count
};

/// Gamepad axis/stick movement events.
class EZ_RAZORCORE_DLL ezRazorGamepadAxisEvent : public ezRazorEvent
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorGamepadAxisEvent, ezRazorEvent);

public:
  ezRazorGamepadAxisEvent();
  ezRazorGamepadAxisEvent(ezUInt8 uiGamepadIndex, ezRazorGamepadAxis axis, float fValue);

  ezUInt8 GetGamepadIndex() const { return m_uiGamepadIndex; }
  ezRazorGamepadAxis GetAxis() const { return m_Axis; }
  
  /// Axis value (-1.0 to 1.0 for sticks, 0.0 to 1.0 for triggers).
  float GetValue() const { return m_fValue; }

private:
  ezUInt8 m_uiGamepadIndex = 0;
  ezRazorGamepadAxis m_Axis = ezRazorGamepadAxis::LeftStickX;
  float m_fValue = 0.0f;
};
