#include <RazorPlugin/RazorPluginPCH.h>

#include <RazorPlugin/Rendering/RazorGeometryBuilder.h>

#include <RazorCore/Render/RazorDrawCommand.h>
#include <RazorCore/Render/RazorGlyphQuad.h>

ezRazorGeometryBuilder::ezRazorGeometryBuilder() = default;
ezRazorGeometryBuilder::~ezRazorGeometryBuilder() = default;

void ezRazorGeometryBuilder::Build(const ezRazorRenderBatch& batch, ezRazorRenderData& out_renderData)
{
  out_renderData.m_Vertices.Clear();
  out_renderData.m_Indices.Clear();
  out_renderData.m_DrawCalls.Clear();
  m_ClipStack.Clear();
  m_bHasClip = false;
  m_CurrentMode = ezRazorDrawCall::Mode::Solid;
  m_uiDrawCallStartIndex = 0;

  const auto& commands = batch.GetCommands();
  const auto& glyphs = batch.GetGlyphBuffer();

  // Pre-allocate conservatively.
  out_renderData.m_Vertices.Reserve(commands.GetCount() * 4);
  out_renderData.m_Indices.Reserve(commands.GetCount() * 6);

  for (const ezRazorDrawCommand& cmd : commands)
  {
    switch (cmd.m_Type.GetValue())
    {
      case ezRazorDrawCommandType::FillRect:
      {
        FlushDrawCallIfModeChanged(ezRazorDrawCall::Mode::Solid, out_renderData);
        EmitFilledRect(cmd.m_Rect, cmd.m_Color, cmd.m_fOpacity, cmd.m_fBorderRadius, out_renderData);
        break;
      }

      case ezRazorDrawCommandType::BorderRect:
      {
        FlushDrawCallIfModeChanged(ezRazorDrawCall::Mode::Solid, out_renderData);
        EmitBorderRect(cmd.m_Rect, cmd.m_BorderColor, cmd.m_fOpacity, cmd.m_fBorderWidth, cmd.m_fBorderRadius, out_renderData);
        break;
      }

      case ezRazorDrawCommandType::Text:
      {
        FlushDrawCallIfModeChanged(ezRazorDrawCall::Mode::SDFText, out_renderData);
        EmitGlyphQuads(cmd, glyphs, out_renderData);
        break;
      }

      case ezRazorDrawCommandType::Image:
      {
        // TODO: Image rendering with texture binding.
        break;
      }

      case ezRazorDrawCommandType::PushClip:
      {
        FlushCurrentDrawCall(out_renderData);
        PushClip(cmd.m_ClipRect);
        break;
      }

      case ezRazorDrawCommandType::PopClip:
      {
        FlushCurrentDrawCall(out_renderData);
        PopClip();
        break;
      }

      default:
        break;
    }
  }

  // Flush final draw call.
  FlushCurrentDrawCall(out_renderData);
}

void ezRazorGeometryBuilder::EmitFilledRect(const ezRectFloat& rect, const ezColor& color, float fOpacity,
                                              const float* fRadius, ezRazorRenderData& out)
{
  ezColor finalColor = color;
  finalColor.a *= fOpacity;

  // TODO: For rounded corners with non-zero fRadius, tessellate arcs.
  // For now, emit a simple quad.
  ezUInt32 uiColor = ezRazorVertex::PackColor(finalColor);
  AddQuad(rect.x, rect.y, rect.width, rect.height, 0, 0, 0, 0, uiColor, out);
}

void ezRazorGeometryBuilder::EmitBorderRect(const ezRectFloat& rect, const ezColor& color, float fOpacity,
                                              const float* fBorderWidth, const float* fRadius,
                                              ezRazorRenderData& out)
{
  ezColor finalColor = color;
  finalColor.a *= fOpacity;
  ezUInt32 uiColor = ezRazorVertex::PackColor(finalColor);

  float t = fBorderWidth[0]; // top
  float r = fBorderWidth[1]; // right
  float b = fBorderWidth[2]; // bottom
  float l = fBorderWidth[3]; // left

  // Top edge.
  if (t > 0.0f)
    AddQuad(rect.x, rect.y, rect.width, t, 0, 0, 0, 0, uiColor, out);

  // Bottom edge.
  if (b > 0.0f)
    AddQuad(rect.x, rect.y + rect.height - b, rect.width, b, 0, 0, 0, 0, uiColor, out);

  // Left edge (between top and bottom).
  if (l > 0.0f)
    AddQuad(rect.x, rect.y + t, l, rect.height - t - b, 0, 0, 0, 0, uiColor, out);

  // Right edge (between top and bottom).
  if (r > 0.0f)
    AddQuad(rect.x + rect.width - r, rect.y + t, r, rect.height - t - b, 0, 0, 0, 0, uiColor, out);
}

void ezRazorGeometryBuilder::EmitGlyphQuads(const ezRazorDrawCommand& cmd, const ezRazorGlyphBuffer& glyphs,
                                              ezRazorRenderData& out)
{
  for (ezUInt32 i = cmd.m_uiGlyphStart; i < cmd.m_uiGlyphStart + cmd.m_uiGlyphCount; i++)
  {
    const ezRazorGlyphQuad& gq = glyphs.GetGlyph(i);

    ezColor finalColor = gq.m_Color;
    finalColor.a *= cmd.m_fOpacity;
    ezUInt32 uiColor = ezRazorVertex::PackColor(finalColor);

    AddQuad(gq.m_Rect.x, gq.m_Rect.y, gq.m_Rect.width, gq.m_Rect.height,
            gq.m_vUVTopLeft.x, gq.m_vUVTopLeft.y,
            gq.m_vUVBottomRight.x, gq.m_vUVBottomRight.y,
            uiColor, out);
  }
}

void ezRazorGeometryBuilder::AddQuad(float x, float y, float w, float h,
                                      float u0, float v0, float u1, float v1,
                                      ezUInt32 uiColor, ezRazorRenderData& out)
{
  ezUInt16 baseVertex = static_cast<ezUInt16>(out.m_Vertices.GetCount());

  // 4 vertices: TL, TR, BL, BR.
  auto& vtl = out.m_Vertices.ExpandAndGetRef();
  vtl.m_fPosX = x;
  vtl.m_fPosY = y;
  vtl.m_fTexU = u0;
  vtl.m_fTexV = v0;
  vtl.m_uiColor = uiColor;

  auto& vtr = out.m_Vertices.ExpandAndGetRef();
  vtr.m_fPosX = x + w;
  vtr.m_fPosY = y;
  vtr.m_fTexU = u1;
  vtr.m_fTexV = v0;
  vtr.m_uiColor = uiColor;

  auto& vbl = out.m_Vertices.ExpandAndGetRef();
  vbl.m_fPosX = x;
  vbl.m_fPosY = y + h;
  vbl.m_fTexU = u0;
  vbl.m_fTexV = v1;
  vbl.m_uiColor = uiColor;

  auto& vbr = out.m_Vertices.ExpandAndGetRef();
  vbr.m_fPosX = x + w;
  vbr.m_fPosY = y + h;
  vbr.m_fTexU = u1;
  vbr.m_fTexV = v1;
  vbr.m_uiColor = uiColor;

  // Two triangles: TL-TR-BL, TR-BR-BL.
  out.m_Indices.PushBack(baseVertex);
  out.m_Indices.PushBack(static_cast<ezUInt16>(baseVertex + 1));
  out.m_Indices.PushBack(static_cast<ezUInt16>(baseVertex + 2));

  out.m_Indices.PushBack(static_cast<ezUInt16>(baseVertex + 1));
  out.m_Indices.PushBack(static_cast<ezUInt16>(baseVertex + 3));
  out.m_Indices.PushBack(static_cast<ezUInt16>(baseVertex + 2));
}

void ezRazorGeometryBuilder::PushClip(const ezRectFloat& rect)
{
  m_ClipStack.PushBack(rect);
  m_CurrentClip = rect;
  m_bHasClip = true;
}

void ezRazorGeometryBuilder::PopClip()
{
  if (!m_ClipStack.IsEmpty())
  {
    m_ClipStack.PopBack();
    if (!m_ClipStack.IsEmpty())
    {
      m_CurrentClip = m_ClipStack.PeekBack();
      m_bHasClip = true;
    }
    else
    {
      m_bHasClip = false;
    }
  }
}

void ezRazorGeometryBuilder::FlushDrawCallIfModeChanged(ezRazorDrawCall::Mode mode, ezRazorRenderData& out)
{
  if (mode != m_CurrentMode && out.m_Indices.GetCount() > m_uiDrawCallStartIndex)
  {
    FlushCurrentDrawCall(out);
  }
  m_CurrentMode = mode;
}

void ezRazorGeometryBuilder::FlushCurrentDrawCall(ezRazorRenderData& out)
{
  ezUInt32 uiIndexCount = out.m_Indices.GetCount() - m_uiDrawCallStartIndex;
  if (uiIndexCount == 0)
    return;

  auto& dc = out.m_DrawCalls.ExpandAndGetRef();
  dc.m_uiFirstIndex = m_uiDrawCallStartIndex;
  dc.m_uiIndexCount = uiIndexCount;
  dc.m_Mode = m_CurrentMode;
  dc.m_bHasScissor = m_bHasClip;
  if (m_bHasClip)
    dc.m_ScissorRect = m_CurrentClip;

  m_uiDrawCallStartIndex = out.m_Indices.GetCount();
}

EZ_STATICLINK_FILE(RazorPlugin, RazorPlugin_Rendering_RazorGeometryBuilder);
