#include <RazorCore/RazorCorePCH.h>

#include <RazorCore/Input/RazorInputHandler.h>
#include <RazorCore/DOM/RazorDocument.h>

ezRazorInputHandler::ezRazorInputHandler()
{
  ezMemoryUtils::ZeroFill(m_GamepadStates, EZ_ARRAY_SIZE(m_GamepadStates));
}

ezRazorInputHandler::~ezRazorInputHandler() = default;

void ezRazorInputHandler::SetDocument(ezRazorDocument* pDocument)
{
  m_pDocument = pDocument;

  m_EventDispatcher.SetDocument(pDocument);
  m_HitTester.SetDocument(pDocument);
  m_HoverTracker.Initialize(pDocument, &m_HitTester);
  m_FocusManager.Initialize(pDocument, &m_EventDispatcher);
}

// --- Mouse Input ---

ezRazorElementId ezRazorInputHandler::OnMouseMove(const ezVec2& vPosition, const ezRazorModifierState& modifiers)
{
  m_vMousePosition = vPosition;

  // Update hover tracking
  ezRazorElementId hovered = m_HoverTracker.UpdatePointerPosition(vPosition);

  // Determine target: captured element or hovered element
  ezRazorElementId target = m_PointerCapture.HasCapture() ? m_PointerCapture.GetCaptureElement() : hovered;

  // Generate leave events for elements no longer hovered
  for (ezRazorElementId left : m_HoverTracker.GetLeftElements())
  {
    ezRazorMouseEvent leaveEvent(ezRazorEventType::MouseLeave, vPosition);
    leaveEvent.SetModifiers(modifiers);
    leaveEvent.SetButtons(m_uiMouseButtons);
    leaveEvent.SetRelatedTarget(hovered);
    m_EventDispatcher.DispatchDirect(left, leaveEvent);

    ezRazorMouseEvent outEvent(ezRazorEventType::MouseOut, vPosition);
    outEvent.SetModifiers(modifiers);
    outEvent.SetButtons(m_uiMouseButtons);
    outEvent.SetRelatedTarget(hovered);
    m_EventDispatcher.DispatchEvent(left, outEvent);
  }

  // Generate enter events for newly hovered elements
  for (ezRazorElementId entered : m_HoverTracker.GetEnteredElements())
  {
    ezRazorMouseEvent enterEvent(ezRazorEventType::MouseEnter, vPosition);
    enterEvent.SetModifiers(modifiers);
    enterEvent.SetButtons(m_uiMouseButtons);
    m_EventDispatcher.DispatchDirect(entered, enterEvent);

    ezRazorMouseEvent overEvent(ezRazorEventType::MouseOver, vPosition);
    overEvent.SetModifiers(modifiers);
    overEvent.SetButtons(m_uiMouseButtons);
    m_EventDispatcher.DispatchEvent(entered, overEvent);
  }

  // Generate mousemove event
  if (target.IsValid())
  {
    ezRazorMouseEvent moveEvent(ezRazorEventType::MouseMove, vPosition);
    moveEvent.SetModifiers(modifiers);
    moveEvent.SetButtons(m_uiMouseButtons);
    m_EventDispatcher.DispatchEvent(target, moveEvent);
  }

  return hovered;
}

void ezRazorInputHandler::OnMouseDown(ezRazorMouseButton button, const ezVec2& vPosition, const ezRazorModifierState& modifiers)
{
  m_vMousePosition = vPosition;
  m_uiMouseButtons |= (1 << (ezUInt8)button);

  ezRazorElementId target = m_HoverTracker.GetHoveredElement();
  if (!target.IsValid())
    return;

  // Store for click detection
  m_MouseDownTarget = target;
  m_vMouseDownPosition = vPosition;
  m_MouseDownTime = ezTime::Now();

  // Generate mousedown event
  ezRazorMouseEvent downEvent(ezRazorEventType::MouseDown, vPosition, button);
  downEvent.SetModifiers(modifiers);
  downEvent.SetButtons(m_uiMouseButtons);
  m_EventDispatcher.DispatchEvent(target, downEvent);

  // Focus the element on left click
  if (button == ezRazorMouseButton::Left && m_FocusManager.IsFocusable(target))
  {
    m_FocusManager.SetFocus(target);
  }
}

void ezRazorInputHandler::OnMouseUp(ezRazorMouseButton button, const ezVec2& vPosition, const ezRazorModifierState& modifiers)
{
  m_vMousePosition = vPosition;
  m_uiMouseButtons &= ~(1 << (ezUInt8)button);

  ezRazorElementId target = m_PointerCapture.HasCapture() ? m_PointerCapture.GetCaptureElement() : m_HoverTracker.GetHoveredElement();

  // Release capture if all buttons released
  if (m_uiMouseButtons == 0)
  {
    m_PointerCapture.ReleaseCapture();
  }

  if (!target.IsValid())
    return;

  // Generate mouseup event
  ezRazorMouseEvent upEvent(ezRazorEventType::MouseUp, vPosition, button);
  upEvent.SetModifiers(modifiers);
  upEvent.SetButtons(m_uiMouseButtons);
  m_EventDispatcher.DispatchEvent(target, upEvent);

  // Check for click (mousedown and mouseup on same element)
  if (button == ezRazorMouseButton::Left && target == m_MouseDownTarget)
  {
    float dist = (vPosition - m_vMouseDownPosition).GetLength();
    if (dist <= m_Config.m_fDragThreshold)
    {
      HandleClick(target, vPosition, modifiers);
    }
  }

  m_MouseDownTarget = {};
}

void ezRazorInputHandler::OnMouseWheel(const ezVec2& vDelta, const ezVec2& vPosition, const ezRazorModifierState& modifiers)
{
  ezRazorElementId target = m_HoverTracker.GetHoveredElement();
  if (!target.IsValid())
    return;

  ezRazorWheelEvent wheelEvent(vPosition, vDelta);
  wheelEvent.SetModifiers(modifiers);
  wheelEvent.SetButtons(m_uiMouseButtons);
  m_EventDispatcher.DispatchEvent(target, wheelEvent);
}

void ezRazorInputHandler::OnMouseLeave()
{
  m_HoverTracker.ClearHover();
  m_vMousePosition = ezVec2::MakeZero();
}

// --- Keyboard Input ---

void ezRazorInputHandler::OnKeyDown(ezUInt32 uiKeyCode, ezUInt32 uiCharacter, bool bRepeat, const ezRazorModifierState& modifiers)
{
  // Handle navigation keys
  HandleNavigation(uiKeyCode, modifiers);

  // Dispatch to focused element
  ezRazorElementId target = m_FocusManager.GetFocusedElement();
  if (!target.IsValid())
    return;

  ezRazorKeyboardEvent keyEvent(ezRazorEventType::KeyDown, uiKeyCode);
  keyEvent.SetCharacter(uiCharacter);
  keyEvent.SetRepeat(bRepeat);
  keyEvent.SetModifiers(modifiers);
  m_EventDispatcher.DispatchEvent(target, keyEvent);
}

void ezRazorInputHandler::OnKeyUp(ezUInt32 uiKeyCode, const ezRazorModifierState& modifiers)
{
  ezRazorElementId target = m_FocusManager.GetFocusedElement();
  if (!target.IsValid())
    return;

  ezRazorKeyboardEvent keyEvent(ezRazorEventType::KeyUp, uiKeyCode);
  keyEvent.SetModifiers(modifiers);
  m_EventDispatcher.DispatchEvent(target, keyEvent);
}

// --- Touch Input ---

void ezRazorInputHandler::OnTouchStart(ezArrayPtr<const ezRazorTouchPoint> touches)
{
  ezRazorTouchEvent event(ezRazorEventType::TouchStart);

  for (const auto& touch : touches)
  {
    m_ActiveTouches.PushBack(touch);
    event.AddChangedTouch(touch);
  }

  for (const auto& touch : m_ActiveTouches)
  {
    event.AddTouch(touch);
  }

  // Dispatch to first touch target
  if (!touches.IsEmpty())
  {
    m_EventDispatcher.DispatchEvent(touches[0].m_Target, event);
  }
}

void ezRazorInputHandler::OnTouchMove(ezArrayPtr<const ezRazorTouchPoint> touches)
{
  ezRazorTouchEvent event(ezRazorEventType::TouchMove);

  for (const auto& touch : touches)
  {
    event.AddChangedTouch(touch);

    // Update stored touch
    for (auto& stored : m_ActiveTouches)
    {
      if (stored.m_iIdentifier == touch.m_iIdentifier)
      {
        stored = touch;
        break;
      }
    }
  }

  for (const auto& touch : m_ActiveTouches)
  {
    event.AddTouch(touch);
  }

  if (!m_ActiveTouches.IsEmpty())
  {
    m_EventDispatcher.DispatchEvent(m_ActiveTouches[0].m_Target, event);
  }
}

void ezRazorInputHandler::OnTouchEnd(ezArrayPtr<const ezRazorTouchPoint> touches)
{
  ezRazorTouchEvent event(ezRazorEventType::TouchEnd);

  for (const auto& touch : touches)
  {
    event.AddChangedTouch(touch);

    // Remove from active touches
    for (ezUInt32 i = 0; i < m_ActiveTouches.GetCount(); ++i)
    {
      if (m_ActiveTouches[i].m_iIdentifier == touch.m_iIdentifier)
      {
        m_ActiveTouches.RemoveAtAndSwap(i);
        break;
      }
    }
  }

  for (const auto& touch : m_ActiveTouches)
  {
    event.AddTouch(touch);
  }

  if (!touches.IsEmpty())
  {
    m_EventDispatcher.DispatchEvent(touches[0].m_Target, event);
  }
}

void ezRazorInputHandler::OnTouchCancel(ezArrayPtr<const ezRazorTouchPoint> touches)
{
  ezRazorTouchEvent event(ezRazorEventType::TouchCancel);

  for (const auto& touch : touches)
  {
    event.AddChangedTouch(touch);
  }

  m_ActiveTouches.Clear();

  if (!touches.IsEmpty())
  {
    m_EventDispatcher.DispatchEvent(touches[0].m_Target, event);
  }
}

// --- Gamepad Input ---

void ezRazorInputHandler::OnGamepadButtonDown(ezUInt8 uiGamepadIndex, ezRazorGamepadButton button, float fValue)
{
  if (uiGamepadIndex >= 4)
    return;

  GamepadState& state = m_GamepadStates[uiGamepadIndex];
  state.m_bButtons[(int)button] = true;

  // Handle navigation buttons
  ezRazorFocusDirection dir = ezRazorFocusDirection::Forward;
  bool navigate = true;

  switch (button)
  {
    case ezRazorGamepadButton::DPadUp:    dir = ezRazorFocusDirection::Up; break;
    case ezRazorGamepadButton::DPadDown:  dir = ezRazorFocusDirection::Down; break;
    case ezRazorGamepadButton::DPadLeft:  dir = ezRazorFocusDirection::Left; break;
    case ezRazorGamepadButton::DPadRight: dir = ezRazorFocusDirection::Right; break;
    case ezRazorGamepadButton::LeftBumper: dir = ezRazorFocusDirection::Backward; break;
    case ezRazorGamepadButton::RightBumper: dir = ezRazorFocusDirection::Forward; break;
    default: navigate = false; break;
  }

  if (navigate)
  {
    m_FocusManager.MoveFocus(dir);
    state.m_NextRepeatTime = ezTime::Now() + ezTime::MakeFromSeconds(m_Config.m_fGamepadRepeatDelay);
    state.m_RepeatDirection = dir;
    state.m_bRepeating = true;
  }

  // Dispatch button event
  ezRazorElementId target = m_FocusManager.GetFocusedElement();
  if (target.IsValid())
  {
    ezRazorGamepadButtonEvent event(ezRazorEventType::GamepadButtonDown, uiGamepadIndex, button);
    event.SetValue(fValue);
    m_EventDispatcher.DispatchEvent(target, event);
  }

  // A button acts like Enter/click
  if (button == ezRazorGamepadButton::A && target.IsValid())
  {
    ezRazorMouseEvent clickEvent(ezRazorEventType::Click, ezVec2::MakeZero());
    m_EventDispatcher.DispatchEvent(target, clickEvent);
  }
}

void ezRazorInputHandler::OnGamepadButtonUp(ezUInt8 uiGamepadIndex, ezRazorGamepadButton button)
{
  if (uiGamepadIndex >= 4)
    return;

  GamepadState& state = m_GamepadStates[uiGamepadIndex];
  state.m_bButtons[(int)button] = false;
  state.m_bRepeating = false;

  ezRazorElementId target = m_FocusManager.GetFocusedElement();
  if (target.IsValid())
  {
    ezRazorGamepadButtonEvent event(ezRazorEventType::GamepadButtonUp, uiGamepadIndex, button);
    m_EventDispatcher.DispatchEvent(target, event);
  }
}

void ezRazorInputHandler::OnGamepadAxis(ezUInt8 uiGamepadIndex, ezRazorGamepadAxis axis, float fValue)
{
  if (uiGamepadIndex >= 4)
    return;

  GamepadState& state = m_GamepadStates[uiGamepadIndex];
  state.m_fAxes[(int)axis] = fValue;

  ezRazorElementId target = m_FocusManager.GetFocusedElement();
  if (target.IsValid())
  {
    ezRazorGamepadAxisEvent event(uiGamepadIndex, axis, fValue);
    m_EventDispatcher.DispatchEvent(target, event);
  }
}

// --- Frame Update ---

void ezRazorInputHandler::Update(ezTime deltaTime)
{
  // Handle gamepad stick navigation and repeat
  HandleGamepadNavigation();
}

bool ezRazorInputHandler::IsMouseButtonDown(ezRazorMouseButton button) const
{
  return (m_uiMouseButtons & (1 << (ezUInt8)button)) != 0;
}

// --- Private ---

void ezRazorInputHandler::HandleClick(ezRazorElementId target, const ezVec2& vPosition, const ezRazorModifierState& modifiers)
{
  ezTime now = ezTime::Now();

  // Check for double-click
  bool isDoubleClick = false;
  if (target == m_LastClickTarget)
  {
    ezTime timeSinceLastClick = now - m_LastClickTime;
    float dist = (vPosition - m_vLastClickPosition).GetLength();

    if (timeSinceLastClick.GetSeconds() <= m_Config.m_fDoubleClickTime && dist <= m_Config.m_fDoubleClickDistance)
    {
      isDoubleClick = true;
    }
  }

  // Generate click event
  ezRazorMouseEvent clickEvent(ezRazorEventType::Click, vPosition, ezRazorMouseButton::Left);
  clickEvent.SetModifiers(modifiers);
  m_EventDispatcher.DispatchEvent(target, clickEvent);

  // Generate double-click event
  if (isDoubleClick)
  {
    ezRazorMouseEvent dblClickEvent(ezRazorEventType::DblClick, vPosition, ezRazorMouseButton::Left);
    dblClickEvent.SetModifiers(modifiers);
    m_EventDispatcher.DispatchEvent(target, dblClickEvent);

    // Reset to prevent triple-click
    m_LastClickTarget = {};
  }
  else
  {
    m_LastClickTime = now;
    m_vLastClickPosition = vPosition;
    m_LastClickTarget = target;
  }
}

void ezRazorInputHandler::HandleNavigation(ezUInt32 uiKeyCode, const ezRazorModifierState& modifiers)
{
  // Tab navigation
  // TODO: Use proper key codes from ezEngine input system
  const ezUInt32 KEY_TAB = 9;
  const ezUInt32 KEY_UP = 38;
  const ezUInt32 KEY_DOWN = 40;
  const ezUInt32 KEY_LEFT = 37;
  const ezUInt32 KEY_RIGHT = 39;

  if (uiKeyCode == KEY_TAB)
  {
    if (modifiers.m_bShift)
      m_FocusManager.MoveFocus(ezRazorFocusDirection::Backward);
    else
      m_FocusManager.MoveFocus(ezRazorFocusDirection::Forward);
  }
  else if (uiKeyCode == KEY_UP)
  {
    m_FocusManager.MoveFocus(ezRazorFocusDirection::Up);
  }
  else if (uiKeyCode == KEY_DOWN)
  {
    m_FocusManager.MoveFocus(ezRazorFocusDirection::Down);
  }
  else if (uiKeyCode == KEY_LEFT)
  {
    m_FocusManager.MoveFocus(ezRazorFocusDirection::Left);
  }
  else if (uiKeyCode == KEY_RIGHT)
  {
    m_FocusManager.MoveFocus(ezRazorFocusDirection::Right);
  }
}

void ezRazorInputHandler::HandleGamepadNavigation()
{
  ezTime now = ezTime::Now();

  for (ezUInt8 i = 0; i < 4; ++i)
  {
    GamepadState& state = m_GamepadStates[i];

    // Handle repeat
    if (state.m_bRepeating && now >= state.m_NextRepeatTime)
    {
      m_FocusManager.MoveFocus(state.m_RepeatDirection);
      state.m_NextRepeatTime = now + ezTime::MakeFromSeconds(m_Config.m_fGamepadRepeatRate);
    }

    // Handle left stick navigation
    float leftX = state.m_fAxes[(int)ezRazorGamepadAxis::LeftStickX];
    float leftY = state.m_fAxes[(int)ezRazorGamepadAxis::LeftStickY];

    if (ezMath::Abs(leftX) > m_Config.m_fGamepadDeadzone || ezMath::Abs(leftY) > m_Config.m_fGamepadDeadzone)
    {
      // Determine dominant direction
      if (!state.m_bRepeating)
      {
        ezRazorFocusDirection dir = ezRazorFocusDirection::Forward;
        if (ezMath::Abs(leftX) > ezMath::Abs(leftY))
        {
          dir = leftX > 0 ? ezRazorFocusDirection::Right : ezRazorFocusDirection::Left;
        }
        else
        {
          dir = leftY > 0 ? ezRazorFocusDirection::Down : ezRazorFocusDirection::Up;
        }

        m_FocusManager.MoveFocus(dir);
        state.m_NextRepeatTime = now + ezTime::MakeFromSeconds(m_Config.m_fGamepadRepeatDelay);
        state.m_RepeatDirection = dir;
        state.m_bRepeating = true;
      }
    }
    else
    {
      // Stick returned to center - only reset if no d-pad buttons held
      bool anyDPadHeld = state.m_bButtons[(int)ezRazorGamepadButton::DPadUp] ||
                         state.m_bButtons[(int)ezRazorGamepadButton::DPadDown] ||
                         state.m_bButtons[(int)ezRazorGamepadButton::DPadLeft] ||
                         state.m_bButtons[(int)ezRazorGamepadButton::DPadRight];
      if (!anyDPadHeld)
      {
        state.m_bRepeating = false;
      }
    }
  }
}

EZ_STATICLINK_FILE(RazorCore, RazorCore_Input_Implementation_RazorInputHandler);
