#pragma once

#include <RazorCore/RazorCoreDLL.h>
#include <RazorCore/DOM/RazorElement.h>
#include <RazorCore/Input/RazorEvent.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>

class ezRazorDocument;

/// Lifecycle callbacks for controller initialization and destruction.
enum class ezRazorControllerLifecycle
{
  OnCreate,    ///< Called when controller is attached to element.
  OnDestroy,   ///< Called when controller is detached or element destroyed.
  OnActivate,  ///< Called when element becomes visible.
  OnDeactivate ///< Called when element becomes hidden.
};

/// Base class for script-driven UI behavior attached to elements.
///
/// Controllers are attached to elements via the `controller` attribute in XML:
/// ```xml
/// <div controller="MyController">...</div>
/// ```
///
/// In AngelScript, inherit from this to handle events and manage state:
/// ```angelscript
/// class MyController : RazorController
/// {
///   void OnCreate() { ... }
///   void OnClick(RazorMouseEvent@ e) { ... }
/// }
/// ```
class EZ_RAZORCORE_DLL ezRazorController : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorController, ezReflectedClass);

public:
  ezRazorController();
  virtual ~ezRazorController();

  /// Initializes the controller with its attached element.
  void Initialize(ezRazorDocument* pDocument, ezRazorElementId element);

  /// Returns the element this controller is attached to.
  ezRazorElementId GetElement() const { return m_Element; }

  /// Returns the document.
  ezRazorDocument* GetDocument() const { return m_pDocument; }

  /// Called each frame to update the controller. Override in derived classes.
  virtual void Update(ezTime deltaTime) {}

  // --- Lifecycle callbacks (override in script) ---

  virtual void OnCreate() {}
  virtual void OnDestroy() {}
  virtual void OnActivate() {}
  virtual void OnDeactivate() {}

  // --- Event callbacks (override in script) ---

  virtual void OnMouseEnter(ezRazorMouseEvent& ref_event) {}
  virtual void OnMouseLeave(ezRazorMouseEvent& ref_event) {}
  virtual void OnMouseMove(ezRazorMouseEvent& ref_event) {}
  virtual void OnMouseDown(ezRazorMouseEvent& ref_event) {}
  virtual void OnMouseUp(ezRazorMouseEvent& ref_event) {}
  virtual void OnClick(ezRazorMouseEvent& ref_event) {}
  virtual void OnDblClick(ezRazorMouseEvent& ref_event) {}
  virtual void OnWheel(ezRazorWheelEvent& ref_event) {}

  virtual void OnKeyDown(ezRazorKeyboardEvent& ref_event) {}
  virtual void OnKeyUp(ezRazorKeyboardEvent& ref_event) {}

  virtual void OnFocus(ezRazorFocusEvent& ref_event) {}
  virtual void OnBlur(ezRazorFocusEvent& ref_event) {}

  virtual void OnTouchStart(ezRazorTouchEvent& ref_event) {}
  virtual void OnTouchMove(ezRazorTouchEvent& ref_event) {}
  virtual void OnTouchEnd(ezRazorTouchEvent& ref_event) {}
  virtual void OnTouchCancel(ezRazorTouchEvent& ref_event) {}

  virtual void OnGamepadButtonDown(ezRazorGamepadButtonEvent& ref_event) {}
  virtual void OnGamepadButtonUp(ezRazorGamepadButtonEvent& ref_event) {}
  virtual void OnGamepadAxis(ezRazorGamepadAxisEvent& ref_event) {}

  // --- Element manipulation helpers ---

  /// Queries for a descendant element by selector (CSS-like).
  ezRazorElementId Query(ezStringView sSelector) const;

  /// Queries for all matching descendant elements.
  ezDynamicArray<ezRazorElementId> QueryAll(ezStringView sSelector) const;

  /// Gets the value of an attribute on this element.
  ezStringView GetAttribute(ezStringView sName) const;

  /// Sets an attribute on this element.
  void SetAttribute(ezStringView sName, ezStringView sValue);

  /// Adds a CSS class to this element.
  void AddClass(ezStringView sClass);

  /// Removes a CSS class from this element.
  void RemoveClass(ezStringView sClass);

  /// Checks if this element has a CSS class.
  bool HasClass(ezStringView sClass) const;

  /// Toggles a CSS class on this element.
  void ToggleClass(ezStringView sClass);

  /// Gets the text content of this element.
  ezStringView GetTextContent() const;

  /// Sets the text content of this element.
  void SetTextContent(ezStringView sText);

  /// Gets a bound data value from the blackboard.
  ezVariant GetData(ezStringView sPath) const;

  /// Sets a bound data value on the blackboard.
  void SetData(ezStringView sPath, const ezVariant& value);

  /// Triggers a navigation to another view/page.
  void Navigate(ezStringView sRoute);

  /// Shows a modal dialog.
  void ShowModal(ezStringView sDialogId);

  /// Closes the current modal dialog.
  void CloseModal();

protected:
  ezRazorDocument* m_pDocument = nullptr;
  ezRazorElementId m_Element;
};

/// Factory for creating controller instances by name.
///
/// Controllers are registered with a name that matches the `controller` attribute value:
/// ```cpp
/// EZ_RAZOR_REGISTER_CONTROLLER(MyController);
/// ```
class EZ_RAZORCORE_DLL ezRazorControllerFactory
{
public:
  using CreateFunc = ezRazorController* (*)();

  static ezRazorControllerFactory* GetInstance();

  /// Registers a controller type with a name.
  void RegisterController(ezStringView sName, CreateFunc createFunc);

  /// Creates a controller instance by name. Caller takes ownership.
  ezRazorController* CreateController(ezStringView sName) const;

  /// Checks if a controller type is registered.
  bool IsRegistered(ezStringView sName) const;

private:
  ezHashTable<ezHashedString, CreateFunc> m_Controllers;
};

/// Macro to register a controller type with the factory.
#define EZ_RAZOR_REGISTER_CONTROLLER(Type)                             \
  namespace                                                            \
  {                                                                    \
    struct Type##_ControllerRegistrar                                  \
    {                                                                  \
      Type##_ControllerRegistrar()                                     \
      {                                                                \
        ezRazorControllerFactory::GetInstance()->RegisterController(   \
          #Type,                                                       \
          []() -> ezRazorController* { return EZ_DEFAULT_NEW(Type); }  \
        );                                                             \
      }                                                                \
    };                                                                 \
    static Type##_ControllerRegistrar s_##Type##_Registrar;            \
  }
