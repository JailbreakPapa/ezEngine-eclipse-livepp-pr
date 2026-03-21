#include <RazorCore/RazorCorePCH.h>

#include <RazorCore/Scripting/RazorController.h>
#include <RazorCore/DOM/RazorDocument.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorController, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezRazorController::ezRazorController() = default;
ezRazorController::~ezRazorController() = default;

void ezRazorController::Initialize(ezRazorDocument* pDocument, ezRazorElementId element)
{
  m_pDocument = pDocument;
  m_Element = element;
}

ezRazorElementId ezRazorController::Query(ezStringView sSelector) const
{
  // TODO: Implement CSS selector query
  EZ_ASSERT_NOT_IMPLEMENTED;
  return {};
}

ezDynamicArray<ezRazorElementId> ezRazorController::QueryAll(ezStringView sSelector) const
{
  // TODO: Implement CSS selector query
  EZ_ASSERT_NOT_IMPLEMENTED;
  return {};
}

ezStringView ezRazorController::GetAttribute(ezStringView sName) const
{
  if (!m_pDocument || !m_Element.IsValid())
    return {};

  // TODO: Access element attribute storage
  return {};
}

void ezRazorController::SetAttribute(ezStringView sName, ezStringView sValue)
{
  if (!m_pDocument || !m_Element.IsValid())
    return;

  // TODO: Modify element attribute storage
}

void ezRazorController::AddClass(ezStringView sClass)
{
  if (!m_pDocument || !m_Element.IsValid())
    return;

  // TODO: Modify element class list
}

void ezRazorController::RemoveClass(ezStringView sClass)
{
  if (!m_pDocument || !m_Element.IsValid())
    return;

  // TODO: Modify element class list
}

bool ezRazorController::HasClass(ezStringView sClass) const
{
  if (!m_pDocument || !m_Element.IsValid())
    return false;

  // TODO: Query element class list
  return false;
}

void ezRazorController::ToggleClass(ezStringView sClass)
{
  if (HasClass(sClass))
    RemoveClass(sClass);
  else
    AddClass(sClass);
}

ezStringView ezRazorController::GetTextContent() const
{
  if (!m_pDocument || !m_Element.IsValid())
    return {};

  // TODO: Get element text content
  return {};
}

void ezRazorController::SetTextContent(ezStringView sText)
{
  if (!m_pDocument || !m_Element.IsValid())
    return;

  // TODO: Set element text content
}

ezVariant ezRazorController::GetData(ezStringView sPath) const
{
  if (!m_pDocument)
    return {};

  // TODO: Query blackboard
  return {};
}

void ezRazorController::SetData(ezStringView sPath, const ezVariant& value)
{
  if (!m_pDocument)
    return;

  // TODO: Update blackboard
}

void ezRazorController::Navigate(ezStringView sRoute)
{
  if (!m_pDocument)
    return;

  // TODO: Trigger navigation
}

void ezRazorController::ShowModal(ezStringView sDialogId)
{
  if (!m_pDocument)
    return;

  // TODO: Show modal dialog
}

void ezRazorController::CloseModal()
{
  if (!m_pDocument)
    return;

  // TODO: Close modal dialog
}

// --- Factory ---

static ezRazorControllerFactory* s_pControllerFactory = nullptr;

ezRazorControllerFactory* ezRazorControllerFactory::GetInstance()
{
  if (s_pControllerFactory == nullptr)
  {
    s_pControllerFactory = EZ_DEFAULT_NEW(ezRazorControllerFactory);
  }
  return s_pControllerFactory;
}

void ezRazorControllerFactory::RegisterController(ezStringView sName, CreateFunc createFunc)
{
  ezHashedString key;
  key.Assign(sName);
  m_Controllers.Insert(key, createFunc);
}

ezRazorController* ezRazorControllerFactory::CreateController(ezStringView sName) const
{
  ezHashedString key;
  key.Assign(sName);

  CreateFunc* pFunc = nullptr;
  if (m_Controllers.TryGetValue(key, pFunc))
  {
    return (*pFunc)();
  }

  return nullptr;
}

bool ezRazorControllerFactory::IsRegistered(ezStringView sName) const
{
  ezHashedString key;
  key.Assign(sName);
  return m_Controllers.Contains(key);
}

EZ_STATICLINK_FILE(RazorCore, RazorCore_Scripting_Implementation_RazorController);
