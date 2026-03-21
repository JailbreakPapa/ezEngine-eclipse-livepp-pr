#pragma once

#include <RazorCore/RazorCoreDLL.h>
#include <RazorCore/DOM/RazorElement.h>
#include <RazorCore/Input/RazorEvent.h>

#include <Foundation/Containers/HashTable.h>
#include <Foundation/Types/Delegate.h>

class ezRazorDocument;

/// Callback type for event handlers.
/// Returns true if the event was handled (prevents further handlers on this target).
using ezRazorEventHandler = ezDelegate<bool(ezRazorEvent&)>;

/// Options for registering an event listener.
struct ezRazorEventListenerOptions
{
  bool m_bCapture = false;    ///< Listen during capture phase instead of bubble
  bool m_bOnce = false;       ///< Automatically remove after first invocation
  bool m_bPassive = false;    ///< Handler won't call preventDefault()
};

/// Manages event handler registration and dispatches events through the DOM.
///
/// Implements the standard capture-target-bubble propagation model:
/// 1. Capture phase: root → target (handlers with capture=true)
/// 2. Target phase: handlers on the target element
/// 3. Bubble phase: target → root (handlers with capture=false)
class EZ_RAZORCORE_DLL ezRazorEventDispatcher
{
  EZ_DISALLOW_COPY_AND_ASSIGN(ezRazorEventDispatcher);

public:
  ezRazorEventDispatcher();
  ~ezRazorEventDispatcher();

  /// Sets the document this dispatcher operates on.
  void SetDocument(ezRazorDocument* pDocument) { m_pDocument = pDocument; }

  /// Registers an event handler on an element.
  /// Returns a handle that can be used to remove the listener.
  ezUInt32 AddEventListener(
    ezRazorElementId target,
    ezHashedString sEventType,
    ezRazorEventHandler handler,
    const ezRazorEventListenerOptions& options = {});

  /// Removes an event handler by its handle.
  void RemoveEventListener(ezUInt32 uiHandle);

  /// Removes all event handlers for a specific element (called when element is destroyed).
  void RemoveAllListeners(ezRazorElementId target);

  /// Dispatches an event to a target element, following capture-target-bubble propagation.
  /// Returns true if the event's default action should proceed (was not prevented).
  bool DispatchEvent(ezRazorElementId target, ezRazorEvent& event);

  /// Dispatches an event directly to a single element without propagation.
  /// Useful for events that don't bubble (focus, blur on the element itself).
  void DispatchDirect(ezRazorElementId target, ezRazorEvent& event);

private:
  struct Listener
  {
    ezRazorElementId m_Target;
    ezHashedString m_sEventType;
    ezRazorEventHandler m_Handler;
    bool m_bCapture = false;
    bool m_bOnce = false;
    bool m_bPassive = false;
    bool m_bRemoved = false;  // Marked for deferred removal
  };

  void InvokeListeners(ezRazorElementId element, ezRazorEvent& event, bool bCapturePhase);
  void BuildPropagationPath(ezRazorElementId target, ezDynamicArray<ezRazorElementId>& out_path);
  void CleanupRemovedListeners();

  ezRazorDocument* m_pDocument = nullptr;
  ezDynamicArray<Listener> m_Listeners;
  ezUInt32 m_uiNextHandle = 1;
  ezUInt32 m_uiDispatchDepth = 0;  // Track nested dispatch calls
};
