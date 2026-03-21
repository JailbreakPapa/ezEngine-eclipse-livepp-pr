#include <RazorCore/RazorCorePCH.h>

#include <RazorCore/Input/RazorHitTest.h>
#include <RazorCore/DOM/RazorDocument.h>
#include <RazorCore/Style/RazorComputedStyle.h>

// ezRazorHitTester

ezRazorHitTester::ezRazorHitTester() = default;
ezRazorHitTester::~ezRazorHitTester() = default;

ezRazorHitTestResult ezRazorHitTester::HitTest(const ezVec2& vViewportPosition, const ezRazorHitTestOptions& options) const
{
  ezRazorHitTestResult result;

  if (!m_pDocument)
    return result;

  ezRazorElementId root = m_pDocument->GetRootElement();
  if (!root.IsValid())
    return result;

  // Collect all hits and return the topmost one
  ezHybridArray<ezRazorHitTestResult, 16> allHits;
  HitTestElementRecursive(root, vViewportPosition, options, allHits);

  if (!allHits.IsEmpty())
  {
    // The last element in the array is the topmost (painted last)
    result = allHits.PeekBack();
  }

  return result;
}

void ezRazorHitTester::HitTestAll(
  const ezVec2& vViewportPosition,
  ezDynamicArray<ezRazorHitTestResult>& out_results,
  const ezRazorHitTestOptions& options) const
{
  out_results.Clear();

  if (!m_pDocument)
    return;

  ezRazorElementId root = m_pDocument->GetRootElement();
  if (!root.IsValid())
    return;

  HitTestElementRecursive(root, vViewportPosition, options, out_results);

  // Reverse to get front-to-back order
  for (ezUInt32 i = 0; i < out_results.GetCount() / 2; ++i)
  {
    ezMath::Swap(out_results[i], out_results[out_results.GetCount() - 1 - i]);
  }
}

bool ezRazorHitTester::IsPointInElement(const ezVec2& vViewportPosition, ezRazorElementId element) const
{
  if (!m_pDocument || !element.IsValid())
    return false;

  const ezRectFloat* pRect = m_pDocument->GetLayoutRect(element);
  if (!pRect)
    return false;

  return pRect->Contains(vViewportPosition);
}

void ezRazorHitTester::GetElementPath(ezRazorElementId element, ezDynamicArray<ezRazorElementId>& out_path) const
{
  out_path.Clear();

  if (!m_pDocument)
    return;

  ezRazorElementId current = element;
  while (current.IsValid())
  {
    out_path.PushBack(current);
    const ezRazorElement* pElement = m_pDocument->GetElement(current);
    if (!pElement)
      break;
    current = pElement->m_Parent;
  }

  // Reverse to get root-to-element order
  for (ezUInt32 i = 0; i < out_path.GetCount() / 2; ++i)
  {
    ezMath::Swap(out_path[i], out_path[out_path.GetCount() - 1 - i]);
  }
}

bool ezRazorHitTester::HitTestElement(
  ezRazorElementId element,
  const ezVec2& vViewportPosition,
  const ezRazorHitTestOptions& options,
  ezRazorHitTestResult& out_result) const
{
  if (!ShouldTestElement(element, options))
    return false;

  const ezRectFloat* pRect = m_pDocument->GetLayoutRect(element);
  if (!pRect)
    return false;

  if (!pRect->Contains(vViewportPosition))
    return false;

  out_result.m_Element = element;
  out_result.m_vLocalPosition = vViewportPosition - ezVec2(pRect->x, pRect->y);
  out_result.m_fDistance = 0.0f;

  return true;
}

void ezRazorHitTester::HitTestElementRecursive(
  ezRazorElementId element,
  const ezVec2& vViewportPosition,
  const ezRazorHitTestOptions& options,
  ezDynamicArray<ezRazorHitTestResult>& out_results) const
{
  const ezRazorElement* pElement = m_pDocument->GetElement(element);
  if (!pElement)
    return;

  // Check if this element is hit (before children, so parent is added first)
  ezRazorHitTestResult result;
  if (HitTestElement(element, vViewportPosition, options, result))
  {
    out_results.PushBack(result);
  }

  // TODO: Check clipping/overflow for options.m_bIncludeClipped
  // If element has overflow:hidden and point is outside, skip children

  // Recursively test children (in tree order = paint order for now)
  ezRazorElementId child = pElement->m_FirstChild;
  while (child.IsValid())
  {
    HitTestElementRecursive(child, vViewportPosition, options, out_results);

    const ezRazorElement* pChild = m_pDocument->GetElement(child);
    if (!pChild)
      break;
    child = pChild->m_NextSibling;
  }
}

bool ezRazorHitTester::ShouldTestElement(ezRazorElementId element, const ezRazorHitTestOptions& options) const
{
  const ezRazorComputedStyle* pStyle = m_pDocument->GetComputedStyle(element);
  if (!pStyle)
    return true;  // No style = test

  // Check visibility
  if (!options.m_bIncludeInvisible)
  {
    // TODO: Check computed visibility property
    // if (pStyle->m_Visibility == ezRazorVisibility::Hidden)
    //   return false;
  }

  // Check pointer-events
  if (!options.m_bIgnorePointerEvents)
  {
    // TODO: Check computed pointer-events property
    // if (pStyle->m_PointerEvents == ezRazorPointerEvents::None)
    //   return false;
  }

  return true;
}

// ezRazorHoverTracker

ezRazorHoverTracker::ezRazorHoverTracker() = default;
ezRazorHoverTracker::~ezRazorHoverTracker() = default;

void ezRazorHoverTracker::Initialize(ezRazorDocument* pDocument, ezRazorHitTester* pHitTester)
{
  m_pDocument = pDocument;
  m_pHitTester = pHitTester;
  ClearHover();
}

ezRazorElementId ezRazorHoverTracker::UpdatePointerPosition(const ezVec2& vViewportPosition)
{
  m_EnteredElements.Clear();
  m_LeftElements.Clear();

  if (!m_pHitTester)
    return {};

  // Hit test to find new hovered element
  ezRazorHitTestResult hit = m_pHitTester->HitTest(vViewportPosition);
  ezRazorElementId newHovered = hit.m_Element;

  if (newHovered == m_HoveredElement)
  {
    // No change
    return m_HoveredElement;
  }

  // Compute old and new paths
  ezHybridArray<ezRazorElementId, 8> newPath;
  ComputeAncestorPath(newHovered, newPath);

  // Find the common ancestor
  ezUInt32 commonDepth = 0;
  ezUInt32 minDepth = ezMath::Min(m_HoverPath.GetCount(), newPath.GetCount());
  for (ezUInt32 i = 0; i < minDepth; ++i)
  {
    // Paths are leaf-to-root, so we compare from the root end
    ezUInt32 oldIdx = m_HoverPath.GetCount() - 1 - i;
    ezUInt32 newIdx = newPath.GetCount() - 1 - i;
    if (m_HoverPath[oldIdx] == newPath[newIdx])
    {
      commonDepth = i + 1;
    }
    else
    {
      break;
    }
  }

  // Elements that were left (old path minus common)
  if (m_HoverPath.GetCount() > commonDepth)
  {
    for (ezUInt32 i = 0; i < m_HoverPath.GetCount() - commonDepth; ++i)
    {
      m_LeftElements.PushBack(m_HoverPath[i]);
    }
  }

  // Elements that were entered (new path minus common, in reverse order for enter)
  if (newPath.GetCount() > commonDepth)
  {
    for (ezUInt32 i = newPath.GetCount() - commonDepth; i > 0; --i)
    {
      m_EnteredElements.PushBack(newPath[i - 1]);
    }
  }

  m_HoveredElement = newHovered;
  m_HoverPath = newPath;

  return m_HoveredElement;
}

void ezRazorHoverTracker::ClearHover()
{
  if (m_HoveredElement.IsValid())
  {
    m_LeftElements = m_HoverPath;
    m_EnteredElements.Clear();
    m_HoveredElement = {};
    m_HoverPath.Clear();
  }
}

void ezRazorHoverTracker::ComputeAncestorPath(ezRazorElementId element, ezDynamicArray<ezRazorElementId>& out_path)
{
  out_path.Clear();

  if (!m_pDocument || !element.IsValid())
    return;

  ezRazorElementId current = element;
  while (current.IsValid())
  {
    out_path.PushBack(current);
    const ezRazorElement* pElement = m_pDocument->GetElement(current);
    if (!pElement)
      break;
    current = pElement->m_Parent;
  }
}

EZ_STATICLINK_FILE(RazorCore, RazorCore_Input_Implementation_RazorHitTest);
