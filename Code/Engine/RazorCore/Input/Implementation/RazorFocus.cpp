#include <RazorCore/RazorCorePCH.h>

#include <RazorCore/Input/RazorFocus.h>
#include <RazorCore/Input/RazorEventDispatcher.h>
#include <RazorCore/DOM/RazorDocument.h>
#include <RazorCore/Style/RazorComputedStyle.h>

// ezRazorFocusManager

ezRazorFocusManager::ezRazorFocusManager() = default;
ezRazorFocusManager::~ezRazorFocusManager() = default;

void ezRazorFocusManager::Initialize(ezRazorDocument* pDocument, ezRazorEventDispatcher* pDispatcher)
{
  m_pDocument = pDocument;
  m_pDispatcher = pDispatcher;
  m_FocusedElement = {};
}

bool ezRazorFocusManager::SetFocus(ezRazorElementId element)
{
  if (element.IsValid() && !IsFocusable(element))
    return false;

  if (element == m_FocusedElement)
    return false;

  ezRazorElementId oldFocus = m_FocusedElement;
  m_FocusedElement = element;

  DispatchFocusEvents(oldFocus, element);

  return true;
}

void ezRazorFocusManager::ClearFocus()
{
  SetFocus({});
}

ezRazorElementId ezRazorFocusManager::MoveFocus(ezRazorFocusDirection direction)
{
  ezRazorElementId next;

  switch (direction)
  {
    case ezRazorFocusDirection::Forward:
    case ezRazorFocusDirection::Backward:
      next = FindNextFocusable(direction);
      break;

    case ezRazorFocusDirection::Up:
    case ezRazorFocusDirection::Down:
    case ezRazorFocusDirection::Left:
    case ezRazorFocusDirection::Right:
      next = FindSpatialFocusable(direction);
      break;
  }

  if (next.IsValid())
  {
    SetFocus(next);
  }

  return m_FocusedElement;
}

bool ezRazorFocusManager::IsFocusable(ezRazorElementId element) const
{
  if (!m_pDocument || !element.IsValid())
    return false;

  const ezRazorElement* pElement = m_pDocument->GetElement(element);
  if (!pElement)
    return false;

  // Check tab index
  ezInt32 tabIndex = GetElementTabIndex(element);
  if (tabIndex < 0)
    return false;

  // TODO: Check if element is visible and enabled
  // const ezRazorComputedStyle* pStyle = m_pDocument->GetComputedStyle(element);
  // if (pStyle && pStyle->m_Visibility != ezRazorVisibility::Visible)
  //   return false;

  return true;
}

void ezRazorFocusManager::GetFocusableElements(ezDynamicArray<ezRazorElementId>& out_elements) const
{
  out_elements.Clear();

  if (!m_pDocument)
    return;

  ezRazorElementId root = m_pDocument->GetRootElement();
  if (!root.IsValid())
    return;

  CollectFocusableElements(root, out_elements);

  // Sort by tab index: tabindex > 0 first (in order), then tabindex = 0 (in tree order)
  // Elements with same positive tabindex keep their tree order
  out_elements.Sort([this](ezRazorElementId a, ezRazorElementId b) {
    ezInt32 tabA = GetElementTabIndex(a);
    ezInt32 tabB = GetElementTabIndex(b);

    // Both zero = keep original order (stable sort would be ideal)
    if (tabA == 0 && tabB == 0)
      return a.m_uiIndex < b.m_uiIndex;

    // Zero comes after positive
    if (tabA == 0)
      return false;
    if (tabB == 0)
      return true;

    // Compare positive values
    return tabA < tabB;
  });
}

ezInt32 ezRazorFocusManager::GetTabIndex(ezRazorElementId element) const
{
  return GetElementTabIndex(element);
}

ezRazorElementId ezRazorFocusManager::FindNextFocusable(ezRazorFocusDirection direction) const
{
  ezHybridArray<ezRazorElementId, 32> focusable;
  GetFocusableElements(focusable);

  if (focusable.IsEmpty())
    return {};

  // Find current position
  ezUInt32 currentIdx = ezInvalidIndex;
  for (ezUInt32 i = 0; i < focusable.GetCount(); ++i)
  {
    if (focusable[i] == m_FocusedElement)
    {
      currentIdx = i;
      break;
    }
  }

  // Determine next index
  ezUInt32 nextIdx;
  if (currentIdx == ezInvalidIndex)
  {
    // No current focus - go to first/last
    nextIdx = (direction == ezRazorFocusDirection::Forward) ? 0 : (focusable.GetCount() - 1);
  }
  else if (direction == ezRazorFocusDirection::Forward)
  {
    if (currentIdx >= focusable.GetCount() - 1)
    {
      if (m_bWrapNavigation)
        nextIdx = 0;
      else
        return m_FocusedElement;  // Stay at end
    }
    else
    {
      nextIdx = currentIdx + 1;
    }
  }
  else
  {
    if (currentIdx == 0)
    {
      if (m_bWrapNavigation)
        nextIdx = focusable.GetCount() - 1;
      else
        return m_FocusedElement;  // Stay at start
    }
    else
    {
      nextIdx = currentIdx - 1;
    }
  }

  return focusable[nextIdx];
}

ezRazorElementId ezRazorFocusManager::FindSpatialFocusable(ezRazorFocusDirection direction) const
{
  if (!m_pDocument || !m_FocusedElement.IsValid())
    return FindNextFocusable(ezRazorFocusDirection::Forward);

  const ezRectFloat* pCurrentRect = m_pDocument->GetLayoutRect(m_FocusedElement);
  if (!pCurrentRect)
    return {};

  ezVec2 currentCenter(pCurrentRect->x + pCurrentRect->width * 0.5f, pCurrentRect->y + pCurrentRect->height * 0.5f);

  // Get direction vector
  ezVec2 dirVec;
  switch (direction)
  {
    case ezRazorFocusDirection::Up:    dirVec = ezVec2(0, -1); break;
    case ezRazorFocusDirection::Down:  dirVec = ezVec2(0, 1); break;
    case ezRazorFocusDirection::Left:  dirVec = ezVec2(-1, 0); break;
    case ezRazorFocusDirection::Right: dirVec = ezVec2(1, 0); break;
    default: return {};
  }

  ezHybridArray<ezRazorElementId, 32> focusable;
  GetFocusableElements(focusable);

  ezRazorElementId best;
  float bestScore = ezMath::HighValue<float>();

  for (ezRazorElementId candidate : focusable)
  {
    if (candidate == m_FocusedElement)
      continue;

    const ezRectFloat* pCandidateRect = m_pDocument->GetLayoutRect(candidate);
    if (!pCandidateRect)
      continue;

    ezVec2 candidateCenter(pCandidateRect->x + pCandidateRect->width * 0.5f, pCandidateRect->y + pCandidateRect->height * 0.5f);
    ezVec2 toCandidate = candidateCenter - currentCenter;

    // Check if candidate is in the right direction
    float dot = toCandidate.Dot(dirVec);
    if (dot <= 0)
      continue;

    // Score: prefer elements more directly in front, closer distance
    float dist = toCandidate.GetLength();
    float alignment = dot / dist;  // 0 to 1, higher = more aligned

    // Score prefers alignment, then distance
    float score = dist * (2.0f - alignment);

    if (score < bestScore)
    {
      bestScore = score;
      best = candidate;
    }
  }

  return best;
}

void ezRazorFocusManager::CollectFocusableElements(ezRazorElementId parent, ezDynamicArray<ezRazorElementId>& out_elements) const
{
  const ezRazorElement* pElement = m_pDocument->GetElement(parent);
  if (!pElement)
    return;

  if (IsFocusable(parent))
  {
    out_elements.PushBack(parent);
  }

  ezRazorElementId child = pElement->m_FirstChild;
  while (child.IsValid())
  {
    CollectFocusableElements(child, out_elements);

    const ezRazorElement* pChild = m_pDocument->GetElement(child);
    if (!pChild)
      break;
    child = pChild->m_NextSibling;
  }
}

ezInt32 ezRazorFocusManager::GetElementTabIndex(ezRazorElementId element) const
{
  if (!m_pDocument || !element.IsValid())
    return -1;

  const ezRazorElement* pElement = m_pDocument->GetElement(element);
  if (!pElement)
    return -1;

  // TODO: Get tab index from element attributes or computed style
  // For now, treat certain tag types as focusable by default

  ezStringView sTag = pElement->m_sTag.GetView();

  // Interactive elements are focusable by default
  if (sTag == "button" || sTag == "input" || sTag == "select" || sTag == "textarea" || sTag == "a")
  {
    return 0;  // Natural tab order
  }

  // Other elements need explicit tabindex
  // TODO: Check for tabindex attribute
  return -1;
}

void ezRazorFocusManager::DispatchFocusEvents(ezRazorElementId oldFocus, ezRazorElementId newFocus)
{
  if (!m_pDispatcher)
    return;

  // Dispatch blur on old element
  if (oldFocus.IsValid())
  {
    ezRazorFocusEvent blurEvent(ezRazorEventType::Blur, newFocus);
    m_pDispatcher->DispatchDirect(oldFocus, blurEvent);

    ezRazorFocusEvent focusOutEvent(ezRazorEventType::FocusOut, newFocus);
    m_pDispatcher->DispatchEvent(oldFocus, focusOutEvent);
  }

  // Dispatch focus on new element
  if (newFocus.IsValid())
  {
    ezRazorFocusEvent focusEvent(ezRazorEventType::Focus, oldFocus);
    m_pDispatcher->DispatchDirect(newFocus, focusEvent);

    ezRazorFocusEvent focusInEvent(ezRazorEventType::FocusIn, oldFocus);
    m_pDispatcher->DispatchEvent(newFocus, focusInEvent);
  }
}

// ezRazorPointerCapture

ezRazorPointerCapture::ezRazorPointerCapture() = default;
ezRazorPointerCapture::~ezRazorPointerCapture() = default;

void ezRazorPointerCapture::SetCapture(ezRazorElementId element)
{
  m_CaptureElement = element;
}

void ezRazorPointerCapture::ReleaseCapture()
{
  m_CaptureElement = {};
}

EZ_STATICLINK_FILE(RazorCore, RazorCore_Input_Implementation_RazorFocus);
