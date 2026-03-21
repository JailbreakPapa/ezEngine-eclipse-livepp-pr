#pragma once

#include <RazorCore/RazorCoreDLL.h>
#include <RazorCore/DOM/RazorElement.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/Variant.h>
#include <Foundation/Communication/Event.h>

/// Event fired when a blackboard value changes.
struct EZ_RAZORCORE_DLL ezRazorBlackboardChangeEvent
{
  ezStringView m_sPath;     ///< Path of the changed value.
  ezVariant m_OldValue;     ///< Previous value.
  ezVariant m_NewValue;     ///< New value.
};

/// Hierarchical data store for UI data binding.
///
/// The blackboard stores named values that can be bound to UI elements.
/// Values are accessed via dot-notation paths: `user.profile.name`.
///
/// ```cpp
/// ezRazorBlackboard bb;
/// bb.SetValue("user.name", "Alice");
/// bb.SetValue("user.score", 100);
/// ezVariant name = bb.GetValue("user.name"); // "Alice"
/// ```
///
/// Supports nested objects via hierarchical paths:
/// - `foo.bar.baz` creates nested structure
/// - Parent changes propagate to children
///
/// Subscribe to changes for reactive updates:
/// ```cpp
/// bb.m_OnChange.AddEventHandler([](const ezRazorBlackboardChangeEvent& e) {
///   // React to change
/// });
/// ```
class EZ_RAZORCORE_DLL ezRazorBlackboard
{
public:
  ezRazorBlackboard();
  ~ezRazorBlackboard();

  /// Gets a value by path. Returns invalid variant if not found.
  ezVariant GetValue(ezStringView sPath) const;

  /// Gets a value by path with a default if not found.
  template <typename T>
  T GetValueOr(ezStringView sPath, const T& defaultValue) const
  {
    ezVariant val = GetValue(sPath);
    if (val.IsValid() && val.CanConvertTo<T>())
      return val.ConvertTo<T>();
    return defaultValue;
  }

  /// Sets a value by path. Fires change event if value changed.
  void SetValue(ezStringView sPath, const ezVariant& value);

  /// Removes a value by path.
  void RemoveValue(ezStringView sPath);

  /// Checks if a path exists.
  bool HasValue(ezStringView sPath) const;

  /// Clears all values.
  void Clear();

  /// Event fired when any value changes.
  ezEvent<const ezRazorBlackboardChangeEvent&> m_OnChange;

private:
  ezHashTable<ezHashedString, ezVariant> m_Values;
};

/// Direction of data binding flow.
enum class ezRazorBindingMode
{
  OneWay,       ///< Source -> Target only
  TwoWay,       ///< Source <-> Target
  OneWayToSource, ///< Target -> Source only
  OneTime       ///< Initial value only, no updates
};

/// Describes a single data binding between a blackboard path and an element property.
struct EZ_RAZORCORE_DLL ezRazorBinding
{
  ezHashedString m_sPath;          ///< Blackboard path.
  ezHashedString m_sProperty;      ///< Element property name.
  ezRazorBindingMode m_Mode = ezRazorBindingMode::OneWay;
  ezHashedString m_sConverter;     ///< Optional value converter name.
  ezVariant m_FallbackValue;       ///< Value to use if source is null/invalid.
};

/// Manages data bindings between blackboard values and UI elements.
///
/// Bindings are declared in XML:
/// ```xml
/// <text bind:content="user.name" />
/// <input bind:value="search.query" bind-mode="two-way" />
/// ```
///
/// Or created programmatically:
/// ```cpp
/// ezRazorBinding binding;
/// binding.m_sPath = "user.name";
/// binding.m_sProperty = "content";
/// manager.AddBinding(elementId, binding);
/// ```
class EZ_RAZORCORE_DLL ezRazorBindingManager
{
public:
  ezRazorBindingManager();
  ~ezRazorBindingManager();

  /// Sets the blackboard to use for data.
  void SetBlackboard(ezRazorBlackboard* pBlackboard);

  /// Gets the current blackboard.
  ezRazorBlackboard* GetBlackboard() const { return m_pBlackboard; }

  /// Adds a binding for an element.
  void AddBinding(ezRazorElementId element, const ezRazorBinding& binding);

  /// Removes all bindings for an element.
  void RemoveBindings(ezRazorElementId element);

  /// Removes a specific binding by property.
  void RemoveBinding(ezRazorElementId element, ezStringView sProperty);

  /// Gets all bindings for an element.
  ezArrayPtr<const ezRazorBinding> GetBindings(ezRazorElementId element) const;

  /// Refreshes all bindings (pushes current blackboard values to elements).
  void RefreshAll();

  /// Refreshes bindings for a specific path (pushes value to bound elements).
  void RefreshPath(ezStringView sPath);

  /// Called when an element property changes; updates blackboard for two-way bindings.
  void OnPropertyChanged(ezRazorElementId element, ezStringView sProperty, const ezVariant& value);

  /// Marks a binding as dirty (needs update next frame).
  void MarkDirty(ezStringView sPath);

  /// Processes dirty bindings. Call once per frame.
  void ProcessDirtyBindings();

private:
  void OnBlackboardChanged(const ezRazorBlackboardChangeEvent& e);

  struct ElementBindings
  {
    ezDynamicArray<ezRazorBinding> m_Bindings;
  };

  ezRazorBlackboard* m_pBlackboard = nullptr;
  ezHashTable<ezRazorElementId, ElementBindings> m_ElementBindings;
  ezHashTable<ezHashedString, ezDynamicArray<ezRazorElementId>> m_PathToElements;
  ezHashSet<ezHashedString> m_DirtyPaths;
  ezEventSubscriptionID m_BlackboardSubscription = {};
};

/// Base class for value converters in data binding.
///
/// Converters transform values between the blackboard and element properties:
/// ```xml
/// <text bind:content="price" converter="currency" />
/// ```
///
/// Register converters:
/// ```cpp
/// class CurrencyConverter : public ezRazorValueConverter { ... };
/// EZ_RAZOR_REGISTER_CONVERTER(CurrencyConverter, "currency");
/// ```
class EZ_RAZORCORE_DLL ezRazorValueConverter : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorValueConverter, ezReflectedClass);

public:
  virtual ~ezRazorValueConverter() = default;

  /// Converts a value from source (blackboard) to target (element).
  virtual ezVariant Convert(const ezVariant& sourceValue, const ezRTTI* pTargetType) = 0;

  /// Converts a value from target (element) back to source (blackboard).
  virtual ezVariant ConvertBack(const ezVariant& targetValue, const ezRTTI* pSourceType) = 0;
};

/// Factory for creating value converter instances by name.
class EZ_RAZORCORE_DLL ezRazorConverterFactory
{
public:
  using CreateFunc = ezRazorValueConverter* (*)();

  static ezRazorConverterFactory* GetInstance();

  void RegisterConverter(ezStringView sName, CreateFunc createFunc);
  ezRazorValueConverter* CreateConverter(ezStringView sName) const;
  bool IsRegistered(ezStringView sName) const;

private:
  ezHashTable<ezHashedString, CreateFunc> m_Converters;
};

#define EZ_RAZOR_REGISTER_CONVERTER(Type, Name)                        \
  namespace                                                            \
  {                                                                    \
    struct Type##_ConverterRegistrar                                   \
    {                                                                  \
      Type##_ConverterRegistrar()                                      \
      {                                                                \
        ezRazorConverterFactory::GetInstance()->RegisterConverter(     \
          Name,                                                        \
          []() -> ezRazorValueConverter* { return EZ_DEFAULT_NEW(Type); } \
        );                                                             \
      }                                                                \
    };                                                                 \
    static Type##_ConverterRegistrar s_##Type##_Registrar;             \
  }
