#include <RazorCore/RazorCorePCH.h>

#include <RazorCore/DOM/RazorDocument.h>
#include <RazorCore/Layout/RazorYogaLayoutAdapter.h>
#include <RazorCore/Style/RazorStyleSheet.h>

#include <Foundation/Algorithm/Sorting.h>
#include <Foundation/IO/Stream.h>

namespace
{
  /// Checks if an element matches a simple selector (tag, id, classes).
  bool MatchesSimpleSelector(const ezRazorElement& element, const ezRazorSelector& selector)
  {
    // Check tag match (empty selector tag matches any element)
    if (!selector.m_sTag.IsEmpty() && selector.m_sTag != element.m_sTag)
      return false;

    // Check ID match
    if (!selector.m_sId.IsEmpty() && selector.m_sId != element.m_sId)
      return false;

    // Check all classes present on element
    for (const auto& cls : selector.m_Classes)
    {
      bool bFound = false;
      for (const auto& elemCls : element.m_Classes)
      {
        if (elemCls == cls)
        {
          bFound = true;
          break;
        }
      }
      if (!bFound)
        return false;
    }

    return true;
  }

  /// Applies a single declaration to a computed style.
  void ApplyDeclaration(ezRazorComputedStyle& style, const ezRazorDeclaration& decl)
  {
    ezStringView prop = decl.m_sProperty.GetView();

    // Layout properties
    if (prop == "width")
    {
      if (decl.m_Value.m_Unit == ezRazorStyleValue::Unit::Pixels)
        style.m_Width = ezRazorStyleValue::MakePixels(decl.m_Value.m_fValue);
      else if (decl.m_Value.m_Unit == ezRazorStyleValue::Unit::Percent)
        style.m_Width = ezRazorStyleValue::MakePercent(decl.m_Value.m_fValue);
      else if (decl.m_Value.m_Unit == ezRazorStyleValue::Unit::Auto)
        style.m_Width = ezRazorStyleValue::MakeAuto();
    }
    else if (prop == "height")
    {
      if (decl.m_Value.m_Unit == ezRazorStyleValue::Unit::Pixels)
        style.m_Height = ezRazorStyleValue::MakePixels(decl.m_Value.m_fValue);
      else if (decl.m_Value.m_Unit == ezRazorStyleValue::Unit::Percent)
        style.m_Height = ezRazorStyleValue::MakePercent(decl.m_Value.m_fValue);
      else if (decl.m_Value.m_Unit == ezRazorStyleValue::Unit::Auto)
        style.m_Height = ezRazorStyleValue::MakeAuto();
    }
    // Margin (all sides)
    else if (prop == "margin")
    {
      ezRazorStyleValue val = ezRazorStyleValue::MakePixels(decl.m_Value.m_fValue);
      if (decl.m_Value.m_Unit == ezRazorStyleValue::Unit::Percent)
        val = ezRazorStyleValue::MakePercent(decl.m_Value.m_fValue);
      style.m_Margin[0] = val; // top
      style.m_Margin[1] = val; // right
      style.m_Margin[2] = val; // bottom
      style.m_Margin[3] = val; // left
    }
    else if (prop == "margin-top")
      style.m_Margin[0] = ezRazorStyleValue::MakePixels(decl.m_Value.m_fValue);
    else if (prop == "margin-right")
      style.m_Margin[1] = ezRazorStyleValue::MakePixels(decl.m_Value.m_fValue);
    else if (prop == "margin-bottom")
      style.m_Margin[2] = ezRazorStyleValue::MakePixels(decl.m_Value.m_fValue);
    else if (prop == "margin-left")
      style.m_Margin[3] = ezRazorStyleValue::MakePixels(decl.m_Value.m_fValue);
    // Padding (all sides)
    else if (prop == "padding")
    {
      ezRazorStyleValue val = ezRazorStyleValue::MakePixels(decl.m_Value.m_fValue);
      if (decl.m_Value.m_Unit == ezRazorStyleValue::Unit::Percent)
        val = ezRazorStyleValue::MakePercent(decl.m_Value.m_fValue);
      style.m_Padding[0] = val; // top
      style.m_Padding[1] = val; // right
      style.m_Padding[2] = val; // bottom
      style.m_Padding[3] = val; // left
    }
    else if (prop == "padding-top")
      style.m_Padding[0] = ezRazorStyleValue::MakePixels(decl.m_Value.m_fValue);
    else if (prop == "padding-right")
      style.m_Padding[1] = ezRazorStyleValue::MakePixels(decl.m_Value.m_fValue);
    else if (prop == "padding-bottom")
      style.m_Padding[2] = ezRazorStyleValue::MakePixels(decl.m_Value.m_fValue);
    else if (prop == "padding-left")
      style.m_Padding[3] = ezRazorStyleValue::MakePixels(decl.m_Value.m_fValue);
    // Border width (all sides)
    else if (prop == "border-width")
    {
      ezRazorStyleValue val = ezRazorStyleValue::MakePixels(decl.m_Value.m_fValue);
      style.m_BorderWidth[0] = val;
      style.m_BorderWidth[1] = val;
      style.m_BorderWidth[2] = val;
      style.m_BorderWidth[3] = val;
    }
    // Visual properties
    else if (prop == "background-color" && decl.m_bIsColor)
    {
      style.m_BackgroundColor = decl.m_ColorValue;
    }
    else if (prop == "color" && decl.m_bIsColor)
    {
      style.m_TextColor = decl.m_ColorValue;
    }
    else if (prop == "border-color" && decl.m_bIsColor)
    {
      style.m_BorderColor = decl.m_ColorValue;
    }
    else if (prop == "opacity")
    {
      style.m_fOpacity = decl.m_Value.m_fValue;
    }
    else if (prop == "font-size")
    {
      style.m_fFontSize = decl.m_Value.m_fValue;
    }
    // Flex properties
    else if (prop == "display")
    {
      // Handled via enum conversion if needed
    }
    else if (prop == "flex-direction")
    {
      // Would need string comparison for row/column/etc.
    }
  }

  /// Structure for sorting rules by specificity during cascade.
  struct RuleMatch
  {
    EZ_DECLARE_POD_TYPE();

    const ezRazorStyleRule* m_pRule;
    ezUInt32 m_uiSpecificity;
    ezUInt32 m_uiSourceOrder;
  };

  bool CompareRuleMatch(const RuleMatch& a, const RuleMatch& b)
  {
    // Lower specificity comes first (so higher specificity wins when applied later)
    if (a.m_uiSpecificity != b.m_uiSpecificity)
      return a.m_uiSpecificity < b.m_uiSpecificity;
    return a.m_uiSourceOrder < b.m_uiSourceOrder;
  }
} // anonymous namespace

ezRazorDocument::ezRazorDocument()
{
  // Slot 0 is reserved as null sentinel.
  m_Elements.ExpandAndGetRef();
  m_ComputedStyles.ExpandAndGetRef();
  m_LayoutRects.ExpandAndGetRef();

  // Create Yoga layout adapter
  m_pYogaAdapter = EZ_DEFAULT_NEW(ezRazorYogaLayoutAdapter);
}

ezRazorDocument::~ezRazorDocument()
{
  m_pYogaAdapter.Clear();
}

ezRazorElementId ezRazorDocument::AllocateElement()
{
  ezRazorElementId id;

  if (!m_FreeList.IsEmpty())
  {
    id.m_uiIndex = m_FreeList.PeekBack();
    m_FreeList.PopBack();

    m_Elements[id.m_uiIndex] = ezRazorElement();
    m_ComputedStyles[id.m_uiIndex] = ezRazorComputedStyle();
    m_LayoutRects[id.m_uiIndex] = ezRectFloat(0, 0, 0, 0);
  }
  else
  {
    id.m_uiIndex = m_Elements.GetCount();
    m_Elements.ExpandAndGetRef();
    m_ComputedStyles.ExpandAndGetRef();
    m_LayoutRects.ExpandAndGetRef();
  }

  m_uiElementCount++;
  return id;
}

void ezRazorDocument::FreeElement(ezRazorElementId id)
{
  if (!id.IsValid())
    return;

  m_FreeList.PushBack(id.m_uiIndex);
  m_Elements[id.m_uiIndex] = ezRazorElement();
  m_uiElementCount--;
}

ezRazorElementId ezRazorDocument::CreateElement(ezStringView sTag)
{
  ezRazorElementId id = AllocateElement();
  m_Elements[id.m_uiIndex].m_sTag.Assign(sTag);
  m_bStyleDirty = true;
  return id;
}

void ezRazorDocument::DestroyElement(ezRazorElementId id)
{
  if (!id.IsValid())
    return;

  // Recursively destroy children first.
  ezRazorElement* pElement = GetElement(id);
  if (pElement == nullptr)
    return;

  ezRazorElementId child = pElement->m_FirstChild;
  while (child.IsValid())
  {
    ezRazorElementId next = m_Elements[child.m_uiIndex].m_NextSibling;
    DestroyElement(child);
    child = next;
  }

  // Unlink from parent's child list.
  if (pElement->m_Parent.IsValid())
  {
    ezRazorElement* pParent = GetElement(pElement->m_Parent);
    if (pParent != nullptr)
    {
      if (pParent->m_FirstChild == id)
      {
        pParent->m_FirstChild = pElement->m_NextSibling;
      }
      else
      {
        ezRazorElementId sibling = pParent->m_FirstChild;
        while (sibling.IsValid())
        {
          ezRazorElement* pSibling = GetElement(sibling);
          if (pSibling->m_NextSibling == id)
          {
            pSibling->m_NextSibling = pElement->m_NextSibling;
            break;
          }
          sibling = pSibling->m_NextSibling;
        }
      }
    }
  }

  FreeElement(id);
  m_bStyleDirty = true;
  m_bLayoutDirty = true;
}

void ezRazorDocument::AppendChild(ezRazorElementId parent, ezRazorElementId child)
{
  EZ_ASSERT_DEV(parent.IsValid() && child.IsValid(), "Invalid element IDs");

  ezRazorElement* pChild = GetElement(child);
  EZ_ASSERT_DEV(pChild != nullptr, "Child element does not exist");
  EZ_ASSERT_DEV(!pChild->m_Parent.IsValid(), "Child already has a parent");

  pChild->m_Parent = parent;

  ezRazorElement* pParent = GetElement(parent);
  EZ_ASSERT_DEV(pParent != nullptr, "Parent element does not exist");

  if (!pParent->m_FirstChild.IsValid())
  {
    pParent->m_FirstChild = child;
  }
  else
  {
    // Walk to last sibling.
    ezRazorElementId sibling = pParent->m_FirstChild;
    while (m_Elements[sibling.m_uiIndex].m_NextSibling.IsValid())
    {
      sibling = m_Elements[sibling.m_uiIndex].m_NextSibling;
    }
    m_Elements[sibling.m_uiIndex].m_NextSibling = child;
  }

  m_bStyleDirty = true;
  m_bLayoutDirty = true;
}

const ezRazorElement* ezRazorDocument::GetElement(ezRazorElementId id) const
{
  if (!id.IsValid() || id.m_uiIndex >= m_Elements.GetCount())
    return nullptr;

  return &m_Elements[id.m_uiIndex];
}

ezRazorElement* ezRazorDocument::GetElement(ezRazorElementId id)
{
  if (!id.IsValid() || id.m_uiIndex >= m_Elements.GetCount())
    return nullptr;

  return &m_Elements[id.m_uiIndex];
}

void ezRazorDocument::AddStyleSheet(const ezRazorStyleSheet* pStyleSheet)
{
  m_StyleSheets.PushBack(pStyleSheet);
  m_bStyleDirty = true;
}

bool ezRazorDocument::UpdateLayout(const ezVec2& vViewportSize)
{
  if (m_bStyleDirty)
  {
    // Style resolution pass — match selectors, cascade, compute final styles.
    for (ezUInt32 elemIdx = 1; elemIdx < m_Elements.GetCount(); ++elemIdx)
    {
      const ezRazorElement& element = m_Elements[elemIdx];
      if (element.m_sTag.IsEmpty())
        continue; // Skip freed elements

      // Reset to default style
      ezRazorComputedStyle& style = m_ComputedStyles[elemIdx];
      style = ezRazorComputedStyle();

      // Default styling based on tag
      if (element.m_sTag.GetView() == "Text" || element.m_sTag.GetView() == "text")
      {
        // Text elements get default content size
        style.m_Width = ezRazorStyleValue::MakeAuto();
        style.m_Height = ezRazorStyleValue::MakeAuto();
        style.m_TextColor = ezColor::White;
        style.m_fFontSize = 16.0f;
      }
      else
      {
        // Container elements default to full width, auto height
        style.m_Width = ezRazorStyleValue::MakePercent(100.0f);
        style.m_Height = ezRazorStyleValue::MakeAuto();
      }

      // Collect all matching rules from all stylesheets
      ezHybridArray<RuleMatch, 16> matchingRules;
      ezUInt32 sourceOrder = 0;

      for (const ezRazorStyleSheet* pStyleSheet : m_StyleSheets)
      {
        if (pStyleSheet == nullptr)
          continue;

        for (const ezRazorStyleRule& rule : pStyleSheet->GetRules())
        {
          if (MatchesSimpleSelector(element, rule.m_Selector))
          {
            RuleMatch match;
            match.m_pRule = &rule;
            match.m_uiSpecificity = rule.m_Selector.GetSpecificity();
            match.m_uiSourceOrder = sourceOrder++;
            matchingRules.PushBack(match);
          }
        }
      }

      // Sort by specificity (ascending so higher specificity applied last, wins)
      ezSorting::QuickSort(matchingRules, CompareRuleMatch);

      // Apply declarations from all matching rules in cascade order
      for (const RuleMatch& match : matchingRules)
      {
        for (const ezRazorDeclaration& decl : match.m_pRule->m_Declarations)
        {
          ApplyDeclaration(style, decl);
        }
      }
    }

    m_bStyleDirty = false;
    m_bLayoutDirty = true;
  }

  if (m_bLayoutDirty)
  {
    // Sync Yoga tree structure from DOM
    m_pYogaAdapter->SyncFromDocument(*this);

    // Apply computed styles to Yoga nodes
    m_pYogaAdapter->ApplyStyles(*this);

    // Compute layout with Yoga
    m_pYogaAdapter->ComputeLayout(vViewportSize, m_LayoutRects);

    m_bLayoutDirty = false;
    ++m_uiLayoutVersion;
    return true;
  }

  return false;
}

const ezRazorComputedStyle* ezRazorDocument::GetComputedStyle(ezRazorElementId id) const
{
  if (!id.IsValid() || id.m_uiIndex >= m_ComputedStyles.GetCount())
    return nullptr;

  return &m_ComputedStyles[id.m_uiIndex];
}

const ezRectFloat* ezRazorDocument::GetLayoutRect(ezRazorElementId id) const
{
  if (!id.IsValid() || id.m_uiIndex >= m_LayoutRects.GetCount())
    return nullptr;

  return &m_LayoutRects[id.m_uiIndex];
}

void ezRazorDocument::InvalidateAll()
{
  m_bStyleDirty = true;
  m_bLayoutDirty = true;
}

ezResult ezRazorDocument::Serialize(ezStreamWriter& inout_stream) const
{
  const ezUInt32 uiVersion = 1;
  inout_stream << uiVersion;

  // Write element count (not including slot 0 sentinel)
  inout_stream << m_uiElementCount;

  // Write root element index
  inout_stream << m_RootElement.m_uiIndex;

  // Write all elements (skip slot 0)
  inout_stream << static_cast<ezUInt32>(m_Elements.GetCount() - 1);
  for (ezUInt32 i = 1; i < m_Elements.GetCount(); ++i)
  {
    const ezRazorElement& elem = m_Elements[i];

    inout_stream << elem.m_sTag;
    inout_stream << elem.m_sId;

    inout_stream << static_cast<ezUInt32>(elem.m_Classes.GetCount());
    for (const auto& cls : elem.m_Classes)
    {
      inout_stream << cls;
    }

    inout_stream << elem.m_Parent.m_uiIndex;
    inout_stream << elem.m_FirstChild.m_uiIndex;
    inout_stream << elem.m_NextSibling.m_uiIndex;
    inout_stream << elem.m_sTextContent;
  }

  // Write free list
  inout_stream << static_cast<ezUInt32>(m_FreeList.GetCount());
  for (ezUInt32 idx : m_FreeList)
  {
    inout_stream << idx;
  }

  return EZ_SUCCESS;
}

ezResult ezRazorDocument::Deserialize(ezStreamReader& inout_stream)
{
  ezUInt32 uiVersion = 0;
  inout_stream >> uiVersion;

  if (uiVersion != 1)
  {
    ezLog::Error("Unknown Razor document version: {}", uiVersion);
    return EZ_FAILURE;
  }

  inout_stream >> m_uiElementCount;
  inout_stream >> m_RootElement.m_uiIndex;

  ezUInt32 uiArrayCount = 0;
  inout_stream >> uiArrayCount;

  // Reset arrays (keep slot 0 as sentinel)
  m_Elements.SetCount(1);
  m_ComputedStyles.SetCount(1);
  m_LayoutRects.SetCount(1);

  m_Elements.Reserve(uiArrayCount + 1);
  m_ComputedStyles.Reserve(uiArrayCount + 1);
  m_LayoutRects.Reserve(uiArrayCount + 1);

  for (ezUInt32 i = 0; i < uiArrayCount; ++i)
  {
    ezRazorElement& elem = m_Elements.ExpandAndGetRef();
    m_ComputedStyles.ExpandAndGetRef();
    m_LayoutRects.ExpandAndGetRef() = ezRectFloat(0, 0, 0, 0);

    inout_stream >> elem.m_sTag;
    inout_stream >> elem.m_sId;

    ezUInt32 uiClassCount = 0;
    inout_stream >> uiClassCount;
    elem.m_Classes.SetCount(uiClassCount);
    for (ezUInt32 c = 0; c < uiClassCount; ++c)
    {
      inout_stream >> elem.m_Classes[c];
    }

    inout_stream >> elem.m_Parent.m_uiIndex;
    inout_stream >> elem.m_FirstChild.m_uiIndex;
    inout_stream >> elem.m_NextSibling.m_uiIndex;
    inout_stream >> elem.m_sTextContent;
  }

  // Read free list
  ezUInt32 uiFreeCount = 0;
  inout_stream >> uiFreeCount;
  m_FreeList.Clear();
  m_FreeList.Reserve(uiFreeCount);
  for (ezUInt32 i = 0; i < uiFreeCount; ++i)
  {
    ezUInt32 idx;
    inout_stream >> idx;
    m_FreeList.PushBack(idx);
  }

  m_bStyleDirty = true;
  m_bLayoutDirty = true;

  return EZ_SUCCESS;
}

EZ_STATICLINK_FILE(RazorCore, RazorCore_DOM_RazorDocument);
