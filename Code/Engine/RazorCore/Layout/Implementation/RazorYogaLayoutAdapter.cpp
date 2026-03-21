#include <RazorCore/RazorCorePCH.h>

#include <RazorCore/Layout/RazorYogaLayoutAdapter.h>
#include <RazorCore/DOM/RazorDocument.h>
#include <RazorCore/DOM/RazorElement.h>
#include <RazorCore/Style/RazorComputedStyle.h>

#include <yoga/Yoga.h>

struct ezRazorYogaLayoutAdapter::Impl
{
  Impl()
  {
    m_pConfig = YGConfigNew();
    YGConfigSetUseWebDefaults(m_pConfig, true);
  }

  ~Impl()
  {
    Clear();
    if (m_pConfig)
    {
      YGConfigFree(m_pConfig);
      m_pConfig = nullptr;
    }
  }

  void Clear()
  {
    if (m_pRoot)
    {
      YGNodeFreeRecursive(m_pRoot);
      m_pRoot = nullptr;
    }
    m_Nodes.Clear();
  }

  YGConfigRef m_pConfig = nullptr;
  YGNodeRef m_pRoot = nullptr;
  ezDynamicArray<YGNodeRef> m_Nodes; // Indexed by element ID
};

namespace
{
  /// Helper to set a Yoga dimension value from ezRazorStyleValue
  void SetYogaValue(YGNodeRef node, const ezRazorStyleValue& val,
                    void (*setPoint)(YGNodeRef, float),
                    void (*setPercent)(YGNodeRef, float),
                    void (*setAuto)(YGNodeRef) = nullptr)
  {
    switch (val.m_Unit)
    {
      case ezRazorStyleValue::Unit::Pixels:
        setPoint(node, val.m_fValue);
        break;
      case ezRazorStyleValue::Unit::Percent:
        setPercent(node, val.m_fValue);
        break;
      case ezRazorStyleValue::Unit::Auto:
        if (setAuto)
          setAuto(node);
        break;
      case ezRazorStyleValue::Unit::Em:
        // Convert em to pixels (assume 16px base)
        setPoint(node, val.m_fValue * 16.0f);
        break;
      case ezRazorStyleValue::Unit::Rem:
        // Convert rem to pixels (assume 16px base)
        setPoint(node, val.m_fValue * 16.0f);
        break;
      default:
        // Leave undefined
        break;
    }
  }

  /// Helper to set edge values (margin, padding, etc.)
  void SetYogaEdgeValue(YGNodeRef node, YGEdge edge, const ezRazorStyleValue& val,
                        void (*setPoint)(YGNodeRef, YGEdge, float),
                        void (*setPercent)(YGNodeRef, YGEdge, float),
                        void (*setAuto)(YGNodeRef, YGEdge) = nullptr)
  {
    switch (val.m_Unit)
    {
      case ezRazorStyleValue::Unit::Pixels:
        setPoint(node, edge, val.m_fValue);
        break;
      case ezRazorStyleValue::Unit::Percent:
        setPercent(node, edge, val.m_fValue);
        break;
      case ezRazorStyleValue::Unit::Auto:
        if (setAuto)
          setAuto(node, edge);
        break;
      case ezRazorStyleValue::Unit::Em:
      case ezRazorStyleValue::Unit::Rem:
        setPoint(node, edge, val.m_fValue * 16.0f);
        break;
      default:
        break;
    }
  }

  /// Convert ezRazorFlexDirection to YGFlexDirection
  YGFlexDirection ToYogaFlexDirection(ezRazorFlexDirection::Enum dir)
  {
    switch (dir)
    {
      case ezRazorFlexDirection::Row: return YGFlexDirectionRow;
      case ezRazorFlexDirection::RowReverse: return YGFlexDirectionRowReverse;
      case ezRazorFlexDirection::Column: return YGFlexDirectionColumn;
      case ezRazorFlexDirection::ColumnReverse: return YGFlexDirectionColumnReverse;
      default: return YGFlexDirectionColumn;
    }
  }

  /// Convert ezRazorFlexWrap to YGWrap
  YGWrap ToYogaWrap(ezRazorFlexWrap::Enum wrap)
  {
    switch (wrap)
    {
      case ezRazorFlexWrap::NoWrap: return YGWrapNoWrap;
      case ezRazorFlexWrap::Wrap: return YGWrapWrap;
      case ezRazorFlexWrap::WrapReverse: return YGWrapWrapReverse;
      default: return YGWrapNoWrap;
    }
  }

  /// Convert ezRazorJustifyContent to YGJustify
  YGJustify ToYogaJustify(ezRazorJustifyContent::Enum justify)
  {
    switch (justify)
    {
      case ezRazorJustifyContent::FlexStart: return YGJustifyFlexStart;
      case ezRazorJustifyContent::Center: return YGJustifyCenter;
      case ezRazorJustifyContent::FlexEnd: return YGJustifyFlexEnd;
      case ezRazorJustifyContent::SpaceBetween: return YGJustifySpaceBetween;
      case ezRazorJustifyContent::SpaceAround: return YGJustifySpaceAround;
      case ezRazorJustifyContent::SpaceEvenly: return YGJustifySpaceEvenly;
      default: return YGJustifyFlexStart;
    }
  }

  /// Convert ezRazorAlignItems to YGAlign
  YGAlign ToYogaAlign(ezRazorAlignItems::Enum align)
  {
    switch (align)
    {
      case ezRazorAlignItems::FlexStart: return YGAlignFlexStart;
      case ezRazorAlignItems::Center: return YGAlignCenter;
      case ezRazorAlignItems::FlexEnd: return YGAlignFlexEnd;
      case ezRazorAlignItems::Stretch: return YGAlignStretch;
      case ezRazorAlignItems::Baseline: return YGAlignBaseline;
      default: return YGAlignStretch;
    }
  }

  /// Convert ezRazorPosition to YGPositionType
  YGPositionType ToYogaPosition(ezRazorPosition::Enum pos)
  {
    switch (pos)
    {
      case ezRazorPosition::Static: return YGPositionTypeStatic;
      case ezRazorPosition::Relative: return YGPositionTypeRelative;
      case ezRazorPosition::Absolute: return YGPositionTypeAbsolute;
      default: return YGPositionTypeRelative;
    }
  }

  /// Convert ezRazorOverflow to YGOverflow
  YGOverflow ToYogaOverflow(ezRazorOverflow::Enum overflow)
  {
    switch (overflow)
    {
      case ezRazorOverflow::Visible: return YGOverflowVisible;
      case ezRazorOverflow::Hidden: return YGOverflowHidden;
      case ezRazorOverflow::Scroll: return YGOverflowScroll;
      default: return YGOverflowVisible;
    }
  }

  /// Convert ezRazorDisplay to YGDisplay
  YGDisplay ToYogaDisplay(ezRazorDisplay::Enum display)
  {
    switch (display)
    {
      case ezRazorDisplay::Flex: return YGDisplayFlex;
      case ezRazorDisplay::None: return YGDisplayNone;
      default: return YGDisplayFlex;
    }
  }

  /// Recursively create Yoga nodes matching the DOM tree structure
  YGNodeRef CreateYogaNodesRecursive(const ezRazorDocument& doc, ezRazorElementId id,
                                      YGConfigRef config, ezDynamicArray<YGNodeRef>& nodes)
  {
    if (!id.IsValid())
      return nullptr;

    const ezRazorElement* pElement = doc.GetElement(id);
    if (!pElement)
      return nullptr;

    // Ensure node array is large enough
    while (nodes.GetCount() <= id.m_uiIndex)
    {
      nodes.PushBack(nullptr);
    }

    // Create node if not exists
    YGNodeRef node = YGNodeNewWithConfig(config);
    nodes[id.m_uiIndex] = node;

    // Process children
    ezRazorElementId childId = pElement->m_FirstChild;
    size_t childIndex = 0;
    while (childId.IsValid())
    {
      YGNodeRef childNode = CreateYogaNodesRecursive(doc, childId, config, nodes);
      if (childNode)
      {
        YGNodeInsertChild(node, childNode, childIndex++);
      }

      const ezRazorElement* pChild = doc.GetElement(childId);
      childId = pChild ? pChild->m_NextSibling : ezRazorElementId();
    }

    return node;
  }

  /// Apply computed style to a Yoga node
  void ApplyStyleToNode(YGNodeRef node, const ezRazorComputedStyle& style)
  {
    // Display
    YGNodeStyleSetDisplay(node, ToYogaDisplay(style.m_Display));

    // Position type
    YGNodeStyleSetPositionType(node, ToYogaPosition(style.m_Position));

    // Flex container properties
    YGNodeStyleSetFlexDirection(node, ToYogaFlexDirection(style.m_FlexDirection));
    YGNodeStyleSetFlexWrap(node, ToYogaWrap(style.m_FlexWrap));
    YGNodeStyleSetJustifyContent(node, ToYogaJustify(style.m_JustifyContent));
    YGNodeStyleSetAlignItems(node, ToYogaAlign(style.m_AlignItems));
    YGNodeStyleSetOverflow(node, ToYogaOverflow(style.m_Overflow));

    // Flex item properties
    YGNodeStyleSetFlexGrow(node, style.m_fFlexGrow);
    YGNodeStyleSetFlexShrink(node, style.m_fFlexShrink);

    // Flex basis
    SetYogaValue(node, style.m_FlexBasis,
                 YGNodeStyleSetFlexBasis, YGNodeStyleSetFlexBasisPercent, YGNodeStyleSetFlexBasisAuto);

    // Width/Height
    SetYogaValue(node, style.m_Width,
                 YGNodeStyleSetWidth, YGNodeStyleSetWidthPercent, YGNodeStyleSetWidthAuto);
    SetYogaValue(node, style.m_Height,
                 YGNodeStyleSetHeight, YGNodeStyleSetHeightPercent, YGNodeStyleSetHeightAuto);

    // Min/Max dimensions
    SetYogaValue(node, style.m_MinWidth, YGNodeStyleSetMinWidth, YGNodeStyleSetMinWidthPercent);
    SetYogaValue(node, style.m_MinHeight, YGNodeStyleSetMinHeight, YGNodeStyleSetMinHeightPercent);
    SetYogaValue(node, style.m_MaxWidth, YGNodeStyleSetMaxWidth, YGNodeStyleSetMaxWidthPercent);
    SetYogaValue(node, style.m_MaxHeight, YGNodeStyleSetMaxHeight, YGNodeStyleSetMaxHeightPercent);

    // Margin (top=0, right=1, bottom=2, left=3)
    SetYogaEdgeValue(node, YGEdgeTop, style.m_Margin[0],
                     YGNodeStyleSetMargin, YGNodeStyleSetMarginPercent, YGNodeStyleSetMarginAuto);
    SetYogaEdgeValue(node, YGEdgeRight, style.m_Margin[1],
                     YGNodeStyleSetMargin, YGNodeStyleSetMarginPercent, YGNodeStyleSetMarginAuto);
    SetYogaEdgeValue(node, YGEdgeBottom, style.m_Margin[2],
                     YGNodeStyleSetMargin, YGNodeStyleSetMarginPercent, YGNodeStyleSetMarginAuto);
    SetYogaEdgeValue(node, YGEdgeLeft, style.m_Margin[3],
                     YGNodeStyleSetMargin, YGNodeStyleSetMarginPercent, YGNodeStyleSetMarginAuto);

    // Padding
    SetYogaEdgeValue(node, YGEdgeTop, style.m_Padding[0],
                     YGNodeStyleSetPadding, YGNodeStyleSetPaddingPercent);
    SetYogaEdgeValue(node, YGEdgeRight, style.m_Padding[1],
                     YGNodeStyleSetPadding, YGNodeStyleSetPaddingPercent);
    SetYogaEdgeValue(node, YGEdgeBottom, style.m_Padding[2],
                     YGNodeStyleSetPadding, YGNodeStyleSetPaddingPercent);
    SetYogaEdgeValue(node, YGEdgeLeft, style.m_Padding[3],
                     YGNodeStyleSetPadding, YGNodeStyleSetPaddingPercent);

    // Border (Yoga only takes point values)
    if (!style.m_BorderWidth[0].IsUndefined())
      YGNodeStyleSetBorder(node, YGEdgeTop, style.m_BorderWidth[0].m_fValue);
    if (!style.m_BorderWidth[1].IsUndefined())
      YGNodeStyleSetBorder(node, YGEdgeRight, style.m_BorderWidth[1].m_fValue);
    if (!style.m_BorderWidth[2].IsUndefined())
      YGNodeStyleSetBorder(node, YGEdgeBottom, style.m_BorderWidth[2].m_fValue);
    if (!style.m_BorderWidth[3].IsUndefined())
      YGNodeStyleSetBorder(node, YGEdgeLeft, style.m_BorderWidth[3].m_fValue);

    // Gap
    if (!style.m_Gap.IsUndefined())
    {
      if (style.m_Gap.m_Unit == ezRazorStyleValue::Unit::Percent)
        YGNodeStyleSetGapPercent(node, YGGutterAll, style.m_Gap.m_fValue);
      else
        YGNodeStyleSetGap(node, YGGutterAll, style.m_Gap.m_fValue);
    }

    // Absolute positioning offsets
    if (!style.m_Top.IsUndefined())
      SetYogaEdgeValue(node, YGEdgeTop, style.m_Top,
                       YGNodeStyleSetPosition, YGNodeStyleSetPositionPercent, YGNodeStyleSetPositionAuto);
    if (!style.m_Right.IsUndefined())
      SetYogaEdgeValue(node, YGEdgeRight, style.m_Right,
                       YGNodeStyleSetPosition, YGNodeStyleSetPositionPercent, YGNodeStyleSetPositionAuto);
    if (!style.m_Bottom.IsUndefined())
      SetYogaEdgeValue(node, YGEdgeBottom, style.m_Bottom,
                       YGNodeStyleSetPosition, YGNodeStyleSetPositionPercent, YGNodeStyleSetPositionAuto);
    if (!style.m_Left.IsUndefined())
      SetYogaEdgeValue(node, YGEdgeLeft, style.m_Left,
                       YGNodeStyleSetPosition, YGNodeStyleSetPositionPercent, YGNodeStyleSetPositionAuto);
  }

  /// Recursively extract layout results from Yoga nodes
  void ExtractLayoutRecursive(const ezRazorDocument& doc, ezRazorElementId id,
                               const ezDynamicArray<YGNodeRef>& nodes,
                               ezDynamicArray<ezRectFloat>& out_rects)
  {
    if (!id.IsValid() || id.m_uiIndex >= nodes.GetCount())
      return;

    YGNodeRef node = nodes[id.m_uiIndex];
    if (!node)
      return;

    // Ensure output array is large enough
    while (out_rects.GetCount() <= id.m_uiIndex)
    {
      out_rects.PushBack(ezRectFloat(0, 0, 0, 0));
    }

    // Get layout results
    float left = YGNodeLayoutGetLeft(node);
    float top = YGNodeLayoutGetTop(node);
    float width = YGNodeLayoutGetWidth(node);
    float height = YGNodeLayoutGetHeight(node);

    out_rects[id.m_uiIndex] = ezRectFloat(left, top, width, height);

    // Process children
    const ezRazorElement* pElement = doc.GetElement(id);
    if (pElement)
    {
      ezRazorElementId childId = pElement->m_FirstChild;
      while (childId.IsValid())
      {
        ExtractLayoutRecursive(doc, childId, nodes, out_rects);

        const ezRazorElement* pChild = doc.GetElement(childId);
        childId = pChild ? pChild->m_NextSibling : ezRazorElementId();
      }
    }
  }
} // anonymous namespace

ezRazorYogaLayoutAdapter::ezRazorYogaLayoutAdapter()
{
  m_pImpl = EZ_DEFAULT_NEW(Impl);
}

ezRazorYogaLayoutAdapter::~ezRazorYogaLayoutAdapter() = default;

void ezRazorYogaLayoutAdapter::SyncFromDocument(const ezRazorDocument& document)
{
  // Clear existing nodes
  m_pImpl->Clear();

  ezRazorElementId rootId = document.GetRootElement();
  if (!rootId.IsValid())
    return;

  // Create Yoga nodes matching the DOM tree
  m_pImpl->m_pRoot = CreateYogaNodesRecursive(document, rootId, m_pImpl->m_pConfig, m_pImpl->m_Nodes);
}

void ezRazorYogaLayoutAdapter::ApplyStyles(const ezRazorDocument& document)
{
  // Apply computed styles to each Yoga node
  for (ezUInt32 i = 1; i < m_pImpl->m_Nodes.GetCount(); ++i)
  {
    YGNodeRef node = m_pImpl->m_Nodes[i];
    if (!node)
      continue;

    const ezRazorComputedStyle* pStyle = document.GetComputedStyle(ezRazorElementId{i});
    if (pStyle)
    {
      ApplyStyleToNode(node, *pStyle);
    }
  }
}

void ezRazorYogaLayoutAdapter::ComputeLayout(const ezVec2& vViewportSize, ezDynamicArray<ezRectFloat>& out_layoutRects)
{
  if (!m_pImpl->m_pRoot)
  {
    return;
  }

  // Calculate layout
  YGNodeCalculateLayout(m_pImpl->m_pRoot, vViewportSize.x, vViewportSize.y, YGDirectionLTR);

  // Extract results - we need the document for tree traversal, but we don't have it here.
  // Instead, we'll do a simpler extraction: just iterate nodes and compute absolute positions.

  // Ensure output array has at least count for slot 0 (sentinel)
  if (out_layoutRects.IsEmpty())
  {
    out_layoutRects.PushBack(ezRectFloat(0, 0, 0, 0));
  }

  // First pass: extract local positions from Yoga
  for (ezUInt32 i = 1; i < m_pImpl->m_Nodes.GetCount(); ++i)
  {
    YGNodeRef node = m_pImpl->m_Nodes[i];
    if (!node)
      continue;

    while (out_layoutRects.GetCount() <= i)
    {
      out_layoutRects.PushBack(ezRectFloat(0, 0, 0, 0));
    }

    float left = YGNodeLayoutGetLeft(node);
    float top = YGNodeLayoutGetTop(node);
    float width = YGNodeLayoutGetWidth(node);
    float height = YGNodeLayoutGetHeight(node);

    out_layoutRects[i] = ezRectFloat(left, top, width, height);
  }

  // Second pass: convert to absolute positions by walking up parent chain
  // We need to compute absolute positions because Yoga gives us positions relative to parent
  for (ezUInt32 i = 1; i < m_pImpl->m_Nodes.GetCount(); ++i)
  {
    YGNodeRef node = m_pImpl->m_Nodes[i];
    if (!node)
      continue;

    float absX = out_layoutRects[i].x;
    float absY = out_layoutRects[i].y;

    // Walk up parent chain accumulating offsets
    YGNodeRef parent = YGNodeGetOwner(node);
    while (parent)
    {
      // Find parent index
      for (ezUInt32 p = 1; p < m_pImpl->m_Nodes.GetCount(); ++p)
      {
        if (m_pImpl->m_Nodes[p] == parent)
        {
          absX += out_layoutRects[p].x;
          absY += out_layoutRects[p].y;
          break;
        }
      }
      parent = YGNodeGetOwner(parent);
    }

    out_layoutRects[i].x = absX;
    out_layoutRects[i].y = absY;
  }
}

void ezRazorYogaLayoutAdapter::Invalidate(ezRazorElementId subtreeRoot)
{
  if (subtreeRoot.IsValid() && subtreeRoot.m_uiIndex < m_pImpl->m_Nodes.GetCount())
  {
    YGNodeRef node = m_pImpl->m_Nodes[subtreeRoot.m_uiIndex];
    if (node)
    {
      YGNodeMarkDirty(node);
    }
  }
  else
  {
    // Invalidate root
    if (m_pImpl->m_pRoot)
    {
      YGNodeMarkDirty(m_pImpl->m_pRoot);
    }
  }
}

EZ_STATICLINK_FILE(RazorCore, RazorCore_Layout_RazorYogaLayoutAdapter);
