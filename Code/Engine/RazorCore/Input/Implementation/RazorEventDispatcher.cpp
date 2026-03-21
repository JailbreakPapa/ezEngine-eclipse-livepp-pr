#include <RazorCore/RazorCorePCH.h>

#include <RazorCore/Input/RazorEventDispatcher.h>
#include <RazorCore/DOM/RazorDocument.h>

ezRazorEventDispatcher::ezRazorEventDispatcher() = default;
ezRazorEventDispatcher::~ezRazorEventDispatcher() = default;

ezUInt32 ezRazorEventDispatcher::AddEventListener(
  ezRazorElementId target,
  ezHashedString sEventType,
  ezRazorEventHandler handler,
  const ezRazorEventListenerOptions& options)
{
  Listener& listener = m_Listeners.ExpandAndGetRef();
  listener.m_Target = target;
  listener.m_sEventType = sEventType;
  listener.m_Handler = handler;
  listener.m_bCapture = options.m_bCapture;
  listener.m_bOnce = options.m_bOnce;
  listener.m_bPassive = options.m_bPassive;
  listener.m_bRemoved = false;

  return m_uiNextHandle++;
}

void ezRazorEventDispatcher::RemoveEventListener(ezUInt32 uiHandle)
{
  // Handle is index + 1
  if (uiHandle > 0 && uiHandle <= m_Listeners.GetCount())
  {
    m_Listeners[uiHandle - 1].m_bRemoved = true;
  }

  // Clean up if not currently dispatching
  if (m_uiDispatchDepth == 0)
  {
    CleanupRemovedListeners();
  }
}

void ezRazorEventDispatcher::RemoveAllListeners(ezRazorElementId target)
{
  for (Listener& listener : m_Listeners)
  {
    if (listener.m_Target == target)
    {
      listener.m_bRemoved = true;
    }
  }

  if (m_uiDispatchDepth == 0)
  {
    CleanupRemovedListeners();
  }
}

bool ezRazorEventDispatcher::DispatchEvent(ezRazorElementId target, ezRazorEvent& event)
{
  if (!m_pDocument || !target.IsValid())
    return true;

  // Build the propagation path from root to target
  ezHybridArray<ezRazorElementId, 16> path;
  BuildPropagationPath(target, path);

  event.SetTarget(target);
  m_uiDispatchDepth++;

  // Capture phase: root to target (exclusive)
  event.SetPhase(ezRazorEventPhase::Capture);
  for (ezUInt32 i = path.GetCount(); i > 1; --i)
  {
    ezRazorElementId current = path[i - 1];
    event.SetCurrentTarget(current);
    InvokeListeners(current, event, true);

    if (event.IsPropagationStopped())
      break;
  }

  // Target phase
  if (!event.IsPropagationStopped())
  {
    event.SetPhase(ezRazorEventPhase::Target);
    event.SetCurrentTarget(target);
    InvokeListeners(target, event, true);  // Capture listeners on target
    if (!event.IsPropagationStopped())
    {
      InvokeListeners(target, event, false); // Bubble listeners on target
    }
  }

  // Bubble phase: target (exclusive) to root
  if (event.GetBubbles() && !event.IsPropagationStopped())
  {
    event.SetPhase(ezRazorEventPhase::Bubble);
    for (ezUInt32 i = 1; i < path.GetCount(); ++i)
    {
      ezRazorElementId current = path[i];
      event.SetCurrentTarget(current);
      InvokeListeners(current, event, false);

      if (event.IsPropagationStopped())
        break;
    }
  }

  m_uiDispatchDepth--;

  // Clean up any listeners marked for removal during dispatch
  if (m_uiDispatchDepth == 0)
  {
    CleanupRemovedListeners();
  }

  return !event.IsDefaultPrevented();
}

void ezRazorEventDispatcher::DispatchDirect(ezRazorElementId target, ezRazorEvent& event)
{
  event.SetTarget(target);
  event.SetCurrentTarget(target);
  event.SetPhase(ezRazorEventPhase::Target);

  m_uiDispatchDepth++;
  InvokeListeners(target, event, true);
  InvokeListeners(target, event, false);
  m_uiDispatchDepth--;

  if (m_uiDispatchDepth == 0)
  {
    CleanupRemovedListeners();
  }
}

void ezRazorEventDispatcher::InvokeListeners(ezRazorElementId element, ezRazorEvent& event, bool bCapturePhase)
{
  // Iterate by index since handlers might modify the list
  for (ezUInt32 i = 0; i < m_Listeners.GetCount(); ++i)
  {
    Listener& listener = m_Listeners[i];

    if (listener.m_bRemoved)
      continue;

    if (listener.m_Target != element)
      continue;

    if (listener.m_sEventType != event.GetType())
      continue;

    // In target phase, invoke all listeners regardless of capture flag
    if (event.GetPhase() != ezRazorEventPhase::Target)
    {
      if (listener.m_bCapture != bCapturePhase)
        continue;
    }
    else
    {
      // Target phase: capture listeners first, then bubble listeners
      if (bCapturePhase != listener.m_bCapture)
        continue;
    }

    // Invoke the handler
    bool bHandled = listener.m_Handler(event);

    // Mark for removal if one-shot
    if (listener.m_bOnce)
    {
      listener.m_bRemoved = true;
    }

    if (event.IsImmediatePropagationStopped())
      break;
  }
}

void ezRazorEventDispatcher::BuildPropagationPath(ezRazorElementId target, ezDynamicArray<ezRazorElementId>& out_path)
{
  out_path.Clear();

  ezRazorElementId current = target;
  while (current.IsValid())
  {
    out_path.PushBack(current);

    const ezRazorElement* pElement = m_pDocument->GetElement(current);
    if (!pElement)
      break;

    current = pElement->m_Parent;
  }

  // Path is now target to root, which is what we need for iteration
}

void ezRazorEventDispatcher::CleanupRemovedListeners()
{
  for (ezInt32 i = (ezInt32)m_Listeners.GetCount() - 1; i >= 0; --i)
  {
    if (m_Listeners[i].m_bRemoved)
    {
      m_Listeners.RemoveAtAndSwap(i);
    }
  }
}

EZ_STATICLINK_FILE(RazorCore, RazorCore_Input_Implementation_RazorEventDispatcher);
