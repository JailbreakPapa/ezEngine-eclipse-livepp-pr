#include <RazorCore/RazorCorePCH.h>

#include <RazorCore/Input/RazorEvent.h>

// Static event type strings
const ezHashedString ezRazorEventType::Click = ezMakeHashedString("click");
const ezHashedString ezRazorEventType::DblClick = ezMakeHashedString("dblclick");
const ezHashedString ezRazorEventType::MouseDown = ezMakeHashedString("mousedown");
const ezHashedString ezRazorEventType::MouseUp = ezMakeHashedString("mouseup");
const ezHashedString ezRazorEventType::MouseMove = ezMakeHashedString("mousemove");
const ezHashedString ezRazorEventType::MouseEnter = ezMakeHashedString("mouseenter");
const ezHashedString ezRazorEventType::MouseLeave = ezMakeHashedString("mouseleave");
const ezHashedString ezRazorEventType::MouseOver = ezMakeHashedString("mouseover");
const ezHashedString ezRazorEventType::MouseOut = ezMakeHashedString("mouseout");
const ezHashedString ezRazorEventType::Wheel = ezMakeHashedString("wheel");
const ezHashedString ezRazorEventType::KeyDown = ezMakeHashedString("keydown");
const ezHashedString ezRazorEventType::KeyUp = ezMakeHashedString("keyup");
const ezHashedString ezRazorEventType::Focus = ezMakeHashedString("focus");
const ezHashedString ezRazorEventType::Blur = ezMakeHashedString("blur");
const ezHashedString ezRazorEventType::FocusIn = ezMakeHashedString("focusin");
const ezHashedString ezRazorEventType::FocusOut = ezMakeHashedString("focusout");
const ezHashedString ezRazorEventType::Input = ezMakeHashedString("input");
const ezHashedString ezRazorEventType::Change = ezMakeHashedString("change");
const ezHashedString ezRazorEventType::Submit = ezMakeHashedString("submit");
const ezHashedString ezRazorEventType::Scroll = ezMakeHashedString("scroll");
const ezHashedString ezRazorEventType::TouchStart = ezMakeHashedString("touchstart");
const ezHashedString ezRazorEventType::TouchMove = ezMakeHashedString("touchmove");
const ezHashedString ezRazorEventType::TouchEnd = ezMakeHashedString("touchend");
const ezHashedString ezRazorEventType::TouchCancel = ezMakeHashedString("touchcancel");
const ezHashedString ezRazorEventType::GamepadButtonDown = ezMakeHashedString("gamepadbuttondown");
const ezHashedString ezRazorEventType::GamepadButtonUp = ezMakeHashedString("gamepadbuttonup");
const ezHashedString ezRazorEventType::GamepadAxis = ezMakeHashedString("gamepadaxis");

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorEvent, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorMouseEvent, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorWheelEvent, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorKeyboardEvent, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorFocusEvent, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorTouchEvent, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorGamepadButtonEvent, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorGamepadAxisEvent, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

// ezRazorEvent

ezRazorEvent::ezRazorEvent()
{
  m_Timestamp = ezTime::Now();
}

ezRazorEvent::ezRazorEvent(ezHashedString sType, bool bBubbles, bool bCancelable)
  : m_sType(sType)
  , m_bBubbles(bBubbles)
  , m_bCancelable(bCancelable)
{
  m_Timestamp = ezTime::Now();
}

// ezRazorMouseEvent

ezRazorMouseEvent::ezRazorMouseEvent()
  : ezRazorEvent()
{
}

ezRazorMouseEvent::ezRazorMouseEvent(ezHashedString sType, const ezVec2& vClientPos, ezRazorMouseButton button)
  : ezRazorEvent(sType, true, true)
  , m_vClientPosition(vClientPos)
  , m_Button(button)
{
}

// ezRazorWheelEvent

ezRazorWheelEvent::ezRazorWheelEvent()
  : ezRazorMouseEvent()
{
  m_sType = ezRazorEventType::Wheel;
}

ezRazorWheelEvent::ezRazorWheelEvent(const ezVec2& vClientPos, const ezVec2& vDelta)
  : ezRazorMouseEvent(ezRazorEventType::Wheel, vClientPos)
  , m_vDelta(vDelta)
{
}

// ezRazorKeyboardEvent

ezRazorKeyboardEvent::ezRazorKeyboardEvent()
  : ezRazorEvent()
{
}

ezRazorKeyboardEvent::ezRazorKeyboardEvent(ezHashedString sType, ezUInt32 uiKeyCode)
  : ezRazorEvent(sType, true, true)
  , m_uiKeyCode(uiKeyCode)
{
}

// ezRazorFocusEvent

ezRazorFocusEvent::ezRazorFocusEvent()
  : ezRazorEvent()
{
}

ezRazorFocusEvent::ezRazorFocusEvent(ezHashedString sType, ezRazorElementId relatedTarget)
  : ezRazorEvent(sType, sType == ezRazorEventType::FocusIn || sType == ezRazorEventType::FocusOut, false)
  , m_RelatedTarget(relatedTarget)
{
}

// ezRazorTouchEvent

ezRazorTouchEvent::ezRazorTouchEvent()
  : ezRazorEvent()
{
}

ezRazorTouchEvent::ezRazorTouchEvent(ezHashedString sType)
  : ezRazorEvent(sType, true, true)
{
}

// ezRazorGamepadButtonEvent

ezRazorGamepadButtonEvent::ezRazorGamepadButtonEvent()
  : ezRazorEvent()
{
}

ezRazorGamepadButtonEvent::ezRazorGamepadButtonEvent(ezHashedString sType, ezUInt8 uiGamepadIndex, ezRazorGamepadButton button)
  : ezRazorEvent(sType, true, true)
  , m_uiGamepadIndex(uiGamepadIndex)
  , m_Button(button)
{
}

// ezRazorGamepadAxisEvent

ezRazorGamepadAxisEvent::ezRazorGamepadAxisEvent()
  : ezRazorEvent()
{
  m_sType = ezRazorEventType::GamepadAxis;
}

ezRazorGamepadAxisEvent::ezRazorGamepadAxisEvent(ezUInt8 uiGamepadIndex, ezRazorGamepadAxis axis, float fValue)
  : ezRazorEvent(ezRazorEventType::GamepadAxis, false, false)
  , m_uiGamepadIndex(uiGamepadIndex)
  , m_Axis(axis)
  , m_fValue(fValue)
{
}

EZ_STATICLINK_FILE(RazorCore, RazorCore_Input_Implementation_RazorEvent);
