#include <RazorCore/RazorCorePCH.h>

#include <RazorCore/Binding/RazorBlackboard.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorValueConverter, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

// --- Blackboard ---

ezRazorBlackboard::ezRazorBlackboard() = default;
ezRazorBlackboard::~ezRazorBlackboard() = default;

ezVariant ezRazorBlackboard::GetValue(ezStringView sPath) const
{
  ezHashedString key;
  key.Assign(sPath);

  ezVariant* pValue = nullptr;
  if (m_Values.TryGetValue(key, pValue))
  {
    return *pValue;
  }
  return {};
}

void ezRazorBlackboard::SetValue(ezStringView sPath, const ezVariant& value)
{
  ezHashedString key;
  key.Assign(sPath);

  ezVariant oldValue;
  m_Values.TryGetValue(key, oldValue);

  // Skip if unchanged
  if (oldValue == value)
    return;

  m_Values[key] = value;

  // Fire change event
  ezRazorBlackboardChangeEvent e;
  e.m_sPath = sPath;
  e.m_OldValue = oldValue;
  e.m_NewValue = value;
  m_OnChange.Broadcast(e);
}

void ezRazorBlackboard::RemoveValue(ezStringView sPath)
{
  ezHashedString key;
  key.Assign(sPath);

  ezVariant oldValue;
  if (m_Values.TryGetValue(key, oldValue))
  {
    m_Values.Remove(key);

    ezRazorBlackboardChangeEvent e;
    e.m_sPath = sPath;
    e.m_OldValue = oldValue;
    e.m_NewValue = {};
    m_OnChange.Broadcast(e);
  }
}

bool ezRazorBlackboard::HasValue(ezStringView sPath) const
{
  ezHashedString key;
  key.Assign(sPath);
  return m_Values.Contains(key);
}

void ezRazorBlackboard::Clear()
{
  m_Values.Clear();
}

// --- BindingManager ---

ezRazorBindingManager::ezRazorBindingManager() = default;

ezRazorBindingManager::~ezRazorBindingManager()
{
  if (m_pBlackboard && m_BlackboardSubscription != 0)
  {
    m_pBlackboard->m_OnChange.RemoveEventHandler(m_BlackboardSubscription);
  }
}

void ezRazorBindingManager::SetBlackboard(ezRazorBlackboard* pBlackboard)
{
  // Unsubscribe from old
  if (m_pBlackboard && m_BlackboardSubscription != 0)
  {
    m_pBlackboard->m_OnChange.RemoveEventHandler(m_BlackboardSubscription);
    m_BlackboardSubscription = {};
  }

  m_pBlackboard = pBlackboard;

  // Subscribe to new
  if (m_pBlackboard)
  {
    m_BlackboardSubscription = m_pBlackboard->m_OnChange.AddEventHandler(
      ezMakeDelegate(&ezRazorBindingManager::OnBlackboardChanged, this));
  }
}

void ezRazorBindingManager::AddBinding(ezRazorElementId element, const ezRazorBinding& binding)
{
  auto& elementBindings = m_ElementBindings[element];
  elementBindings.m_Bindings.PushBack(binding);

  // Track path -> element mapping for efficient updates
  auto& elements = m_PathToElements[binding.m_sPath];
  if (!elements.Contains(element))
  {
    elements.PushBack(element);
  }

  // Mark for initial sync
  m_DirtyPaths.Insert(binding.m_sPath);
}

void ezRazorBindingManager::RemoveBindings(ezRazorElementId element)
{
  ElementBindings* pBindings = nullptr;
  if (m_ElementBindings.TryGetValue(element, pBindings))
  {
    // Remove from path mappings
    for (const auto& binding : pBindings->m_Bindings)
    {
      auto it = m_PathToElements.Find(binding.m_sPath);
      if (it.IsValid())
      {
        it.Value().RemoveAndCopy(element);
        if (it.Value().IsEmpty())
        {
          m_PathToElements.Remove(it);
        }
      }
    }

    m_ElementBindings.Remove(element);
  }
}

void ezRazorBindingManager::RemoveBinding(ezRazorElementId element, ezStringView sProperty)
{
  ElementBindings* pBindings = nullptr;
  if (!m_ElementBindings.TryGetValue(element, pBindings))
    return;

  ezHashedString propKey;
  propKey.Assign(sProperty);

  for (ezUInt32 i = 0; i < pBindings->m_Bindings.GetCount(); ++i)
  {
    if (pBindings->m_Bindings[i].m_sProperty == propKey)
    {
      // Remove from path mapping
      auto it = m_PathToElements.Find(pBindings->m_Bindings[i].m_sPath);
      if (it.IsValid())
      {
        it.Value().RemoveAndCopy(element);
        if (it.Value().IsEmpty())
        {
          m_PathToElements.Remove(it);
        }
      }

      pBindings->m_Bindings.RemoveAtAndCopy(i);
      break;
    }
  }
}

ezArrayPtr<const ezRazorBinding> ezRazorBindingManager::GetBindings(ezRazorElementId element) const
{
  const ElementBindings* pBindings = nullptr;
  if (m_ElementBindings.TryGetValue(element, pBindings))
  {
    return pBindings->m_Bindings;
  }
  return {};
}

void ezRazorBindingManager::RefreshAll()
{
  if (!m_pBlackboard)
    return;

  for (auto& pair : m_PathToElements)
  {
    m_DirtyPaths.Insert(pair.Key());
  }

  ProcessDirtyBindings();
}

void ezRazorBindingManager::RefreshPath(ezStringView sPath)
{
  ezHashedString key;
  key.Assign(sPath);
  m_DirtyPaths.Insert(key);
}

void ezRazorBindingManager::OnPropertyChanged(ezRazorElementId element, ezStringView sProperty, const ezVariant& value)
{
  if (!m_pBlackboard)
    return;

  ElementBindings* pBindings = nullptr;
  if (!m_ElementBindings.TryGetValue(element, pBindings))
    return;

  ezHashedString propKey;
  propKey.Assign(sProperty);

  for (const auto& binding : pBindings->m_Bindings)
  {
    if (binding.m_sProperty != propKey)
      continue;

    // Only process two-way or one-way-to-source bindings
    if (binding.m_Mode != ezRazorBindingMode::TwoWay &&
        binding.m_Mode != ezRazorBindingMode::OneWayToSource)
      continue;

    // Apply converter if specified
    ezVariant finalValue = value;
    if (!binding.m_sConverter.IsEmpty())
    {
      ezRazorValueConverter* pConverter = ezRazorConverterFactory::GetInstance()->CreateConverter(binding.m_sConverter.GetView());
      if (pConverter)
      {
        finalValue = pConverter->ConvertBack(value, nullptr);
        EZ_DEFAULT_DELETE(pConverter);
      }
    }

    m_pBlackboard->SetValue(binding.m_sPath.GetView(), finalValue);
  }
}

void ezRazorBindingManager::MarkDirty(ezStringView sPath)
{
  ezHashedString key;
  key.Assign(sPath);
  m_DirtyPaths.Insert(key);
}

void ezRazorBindingManager::ProcessDirtyBindings()
{
  if (!m_pBlackboard || m_DirtyPaths.IsEmpty())
    return;

  for (const auto& path : m_DirtyPaths)
  {
    ezVariant value = m_pBlackboard->GetValue(path.GetView());

    auto* pElements = m_PathToElements.GetValue(path);
    if (!pElements)
      continue;

    for (ezRazorElementId element : *pElements)
    {
      ElementBindings* pBindings = nullptr;
      if (!m_ElementBindings.TryGetValue(element, pBindings))
        continue;

      for (const auto& binding : pBindings->m_Bindings)
      {
        if (binding.m_sPath != path)
          continue;

        // Skip one-way-to-source bindings
        if (binding.m_Mode == ezRazorBindingMode::OneWayToSource)
          continue;

        // Use fallback if value invalid
        ezVariant finalValue = value.IsValid() ? value : binding.m_FallbackValue;

        // Apply converter if specified
        if (!binding.m_sConverter.IsEmpty())
        {
          ezRazorValueConverter* pConverter = ezRazorConverterFactory::GetInstance()->CreateConverter(binding.m_sConverter.GetView());
          if (pConverter)
          {
            finalValue = pConverter->Convert(finalValue, nullptr);
            EZ_DEFAULT_DELETE(pConverter);
          }
        }

        // TODO: Apply value to element property
        // This requires access to element storage, will be wired up later
      }
    }
  }

  m_DirtyPaths.Clear();
}

void ezRazorBindingManager::OnBlackboardChanged(const ezRazorBlackboardChangeEvent& e)
{
  ezHashedString key;
  key.Assign(e.m_sPath);
  m_DirtyPaths.Insert(key);
}

// --- ConverterFactory ---

static ezRazorConverterFactory* s_pConverterFactory = nullptr;

ezRazorConverterFactory* ezRazorConverterFactory::GetInstance()
{
  if (s_pConverterFactory == nullptr)
  {
    s_pConverterFactory = EZ_DEFAULT_NEW(ezRazorConverterFactory);
  }
  return s_pConverterFactory;
}

void ezRazorConverterFactory::RegisterConverter(ezStringView sName, CreateFunc createFunc)
{
  ezHashedString key;
  key.Assign(sName);
  m_Converters.Insert(key, createFunc);
}

ezRazorValueConverter* ezRazorConverterFactory::CreateConverter(ezStringView sName) const
{
  ezHashedString key;
  key.Assign(sName);

  CreateFunc* pFunc = nullptr;
  if (m_Converters.TryGetValue(key, pFunc))
  {
    return (*pFunc)();
  }
  return nullptr;
}

bool ezRazorConverterFactory::IsRegistered(ezStringView sName) const
{
  ezHashedString key;
  key.Assign(sName);
  return m_Converters.Contains(key);
}

EZ_STATICLINK_FILE(RazorCore, RazorCore_Binding_Implementation_RazorBlackboard);
