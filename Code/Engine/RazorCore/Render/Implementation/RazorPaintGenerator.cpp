#include <RazorCore/RazorCorePCH.h>

#include <RazorCore/Render/RazorPaintGenerator.h>
#include <RazorCore/DOM/RazorDocument.h>
#include <RazorCore/DOM/RazorElement.h>
#include <RazorCore/Style/RazorComputedStyle.h>

ezRazorPaintGenerator::ezRazorPaintGenerator() = default;
ezRazorPaintGenerator::~ezRazorPaintGenerator() = default;

void ezRazorPaintGenerator::Generate(const ezRazorDocument& doc, ezRazorRenderBatch& out_batch)
{
  out_batch.Clear();
  m_iGlobalOrder = 0;

  ezRazorElementId root = doc.GetRootElement();
  if (!root.IsValid())
    return;

  ezRectFloat viewportClip(0, 0, 100000.0f, 100000.0f);
  PaintElement(doc, root, viewportClip, 0, out_batch);

  out_batch.SortByOrder();
}

void ezRazorPaintGenerator::PaintElement(const ezRazorDocument& doc, ezRazorElementId id,
                                         const ezRectFloat& parentClip, ezInt32 iBaseOrder,
                                         ezRazorRenderBatch& out_batch)
{
  const ezRazorElement* pElement = doc.GetElement(id);
  if (pElement == nullptr)
    return;

  const ezRazorComputedStyle* pStyle = doc.GetComputedStyle(id);
  const ezRectFloat* pRect = doc.GetLayoutRect(id);
  if (pStyle == nullptr || pRect == nullptr)
    return;

  // Skip display:none elements.
  if (pStyle->m_Display == ezRazorDisplay::None)
    return;

  // Skip fully transparent elements.
  if (pStyle->m_fOpacity <= 0.0f)
    return;

  const ezInt32 iOrder = iBaseOrder + pStyle->m_iZIndex;
  const ezRectFloat& rect = *pRect;

  // Compute clip rect for overflow:hidden.
  ezRectFloat clipRect = parentClip;
  bool bPushedClip = false;

  if (pStyle->m_Overflow == ezRazorOverflow::Hidden || pStyle->m_Overflow == ezRazorOverflow::Scroll)
  {
    // Clip to this element's content box.
    clipRect = rect;

    ezRazorDrawCommand clipCmd;
    clipCmd.m_Type = ezRazorDrawCommandType::PushClip;
    clipCmd.m_ClipRect = clipRect;
    clipCmd.m_iSortOrder = m_iGlobalOrder++;
    out_batch.AddCommand(std::move(clipCmd));
    bPushedClip = true;
  }

  // 1. Background.
  if (pStyle->m_BackgroundColor.a > 0.0f)
  {
    EmitBackground(rect, *pStyle, m_iGlobalOrder++, out_batch);
  }

  // 2. Border.
  bool hasBorder = false;
  for (int i = 0; i < 4; i++)
  {
    if (pStyle->m_BorderWidth[i].m_fValue > 0.0f && !pStyle->m_BorderWidth[i].IsUndefined())
    {
      hasBorder = true;
      break;
    }
  }
  if (hasBorder && pStyle->m_BorderColor.a > 0.0f)
  {
    EmitBorder(rect, *pStyle, m_iGlobalOrder++, out_batch);
  }

  // 3. Text content.
  if (!pElement->m_sTextContent.IsEmpty())
  {
    EmitText(doc, id, rect, *pStyle, m_iGlobalOrder++, out_batch);
  }

  // 4. Paint children in tree order (z-index is handled by per-element sort order offset).
  ezRazorElementId child = pElement->m_FirstChild;
  while (child.IsValid())
  {
    PaintElement(doc, child, clipRect, iOrder, out_batch);

    const ezRazorElement* pChild = doc.GetElement(child);
    child = pChild ? pChild->m_NextSibling : ezRazorElementId();
  }

  // Pop clip.
  if (bPushedClip)
  {
    ezRazorDrawCommand popCmd;
    popCmd.m_Type = ezRazorDrawCommandType::PopClip;
    popCmd.m_iSortOrder = m_iGlobalOrder++;
    out_batch.AddCommand(std::move(popCmd));
  }
}

void ezRazorPaintGenerator::EmitBackground(const ezRectFloat& rect, const ezRazorComputedStyle& style,
                                            ezInt32 iOrder, ezRazorRenderBatch& out_batch)
{
  ezRazorDrawCommand cmd;
  cmd.m_Type = ezRazorDrawCommandType::FillRect;
  cmd.m_Rect = rect;
  cmd.m_Color = style.m_BackgroundColor;
  cmd.m_fOpacity = style.m_fOpacity;
  cmd.m_iSortOrder = iOrder;

  for (int i = 0; i < 4; i++)
    cmd.m_fBorderRadius[i] = style.m_fBorderRadius[i];

  out_batch.AddCommand(std::move(cmd));
}

void ezRazorPaintGenerator::EmitBorder(const ezRectFloat& rect, const ezRazorComputedStyle& style,
                                        ezInt32 iOrder, ezRazorRenderBatch& out_batch)
{
  ezRazorDrawCommand cmd;
  cmd.m_Type = ezRazorDrawCommandType::BorderRect;
  cmd.m_Rect = rect;
  cmd.m_BorderColor = style.m_BorderColor;
  cmd.m_fOpacity = style.m_fOpacity;
  cmd.m_iSortOrder = iOrder;

  for (int i = 0; i < 4; i++)
  {
    cmd.m_fBorderRadius[i] = style.m_fBorderRadius[i];
    cmd.m_fBorderWidth[i] = style.m_BorderWidth[i].IsUndefined() ? 0.0f : style.m_BorderWidth[i].m_fValue;
  }

  out_batch.AddCommand(std::move(cmd));
}

void ezRazorPaintGenerator::EmitText(const ezRazorDocument& doc, ezRazorElementId id,
                                      const ezRectFloat& rect, const ezRazorComputedStyle& style,
                                      ezInt32 iOrder, ezRazorRenderBatch& out_batch)
{
  const ezRazorElement* pElement = doc.GetElement(id);
  if (pElement == nullptr || pElement->m_sTextContent.IsEmpty())
    return;

  // TODO: Use Skribidi to shape text and produce glyph quads.
  // For now, emit a placeholder text draw command so the command stream is correct.
  // The renderer will handle text once Skribidi integration is complete.

  ezRazorGlyphBuffer& glyphBuf = out_batch.GetGlyphBuffer();
  ezUInt32 uiStart = glyphBuf.GetCount();

  // Placeholder: emit one quad per character as a stand-in for real glyph shaping.
  const char* szText = pElement->m_sTextContent.GetData();
  float fX = rect.x;
  const float fCharWidth = style.m_fFontSize * 0.6f;
  const float fCharHeight = style.m_fFontSize;

  ezStringView textView(szText);
  for (auto it = textView.GetIteratorFront(); it.IsValid(); ++it)
  {
    ezRazorGlyphQuad quad;
    quad.m_Rect = ezRectFloat(fX, rect.y, fCharWidth, fCharHeight);
    quad.m_vUVTopLeft = ezVec2::MakeZero();
    quad.m_vUVBottomRight = ezVec2(1.0f, 1.0f);
    quad.m_Color = style.m_TextColor;
    glyphBuf.AddGlyph(quad);

    fX += fCharWidth;
  }

  ezRazorDrawCommand cmd;
  cmd.m_Type = ezRazorDrawCommandType::Text;
  cmd.m_Rect = rect;
  cmd.m_Color = style.m_TextColor;
  cmd.m_fOpacity = style.m_fOpacity;
  cmd.m_uiGlyphStart = uiStart;
  cmd.m_uiGlyphCount = glyphBuf.GetCount() - uiStart;
  cmd.m_iSortOrder = iOrder;

  out_batch.AddCommand(std::move(cmd));
}

EZ_STATICLINK_FILE(RazorCore, RazorCore_Render_RazorPaintGenerator);
