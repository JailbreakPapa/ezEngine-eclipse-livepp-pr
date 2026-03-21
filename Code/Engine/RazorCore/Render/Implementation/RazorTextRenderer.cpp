#include <RazorCore/RazorCorePCH.h>

#include <RazorCore/Render/RazorTextRenderer.h>

#include <Foundation/Logging/Log.h>

#include <skb_attributes.h>
#include <skb_font_collection.h>
#include <skb_image_atlas.h>
#include <skb_layout.h>
#include <skb_rasterizer.h>

ezRazorTextRenderer::ezRazorTextRenderer() = default;

ezRazorTextRenderer::~ezRazorTextRenderer()
{
  Shutdown();
}

void ezRazorTextRenderer::Initialize()
{
  if (m_pFontCollection != nullptr)
    return;

  m_pFontCollection = skb_font_collection_create();

  skb_image_atlas_config_t config = skb_image_atlas_get_default_config();
  m_pAtlas = skb_image_atlas_create(&config);

  skb_rasterizer_config_t rasterConfig = {};
  rasterConfig.on_edge_value = 128;
  rasterConfig.pixel_dist_scale = 32.0f;
  m_pRasterizer = skb_rasterizer_create(&rasterConfig);

  ezLog::Info("RazorTextRenderer: Skribidi text system initialized.");
}

void ezRazorTextRenderer::Shutdown()
{
  if (m_pRasterizer != nullptr)
  {
    skb_rasterizer_destroy(m_pRasterizer);
    m_pRasterizer = nullptr;
  }

  if (m_pAtlas != nullptr)
  {
    skb_image_atlas_destroy(m_pAtlas);
    m_pAtlas = nullptr;
  }

  if (m_pFontCollection != nullptr)
  {
    skb_font_collection_destroy(m_pFontCollection);
    m_pFontCollection = nullptr;
  }
}

ezUInt32 ezRazorTextRenderer::AddFontFromFile(ezStringView sPath, ezStringView sFamilyName)
{
  if (m_pFontCollection == nullptr)
    return 0;

  ezStringBuilder sPathStr(sPath);
  skb_font_create_params_t params = {};

  skb_font_handle_t handle = skb_font_collection_add_font(
    m_pFontCollection, sPathStr.GetData(), SKB_FONT_FAMILY_SANS_SERIF, &params);

  return static_cast<ezUInt32>(handle);
}

ezUInt32 ezRazorTextRenderer::AddFontFromMemory(ezStringView sName, ezArrayPtr<const ezUInt8> data, ezStringView sFamilyName)
{
  if (m_pFontCollection == nullptr)
    return 0;

  ezStringBuilder sNameStr(sName);
  skb_font_create_params_t params = {};

  skb_font_handle_t handle = skb_font_collection_add_font_from_data(
    m_pFontCollection, sNameStr.GetData(),
    data.GetPtr(), static_cast<size_t>(data.GetCount()),
    nullptr, nullptr,
    SKB_FONT_FAMILY_SANS_SERIF, &params);

  return static_cast<ezUInt32>(handle);
}

ezUInt32 ezRazorTextRenderer::ShapeText(ezStringView sText, float fFontSize, const ezColor& textColor,
                                         float fX, float fY, float fMaxWidth,
                                         ezRazorGlyphBuffer& out_glyphs)
{
  if (m_pFontCollection == nullptr || m_pAtlas == nullptr)
    return 0;

  // Set up layout parameters.
  skb_layout_params_t params = {};
  params.font_collection = m_pFontCollection;
  params.layout_width = fMaxWidth;
  params.layout_height = 0; // Unlimited height.

  // Set up default text attributes (font size).
  skb_attribute_t attribs[1];
  attribs[0] = skb_attribute_make_font_size(fFontSize);

  skb_attribute_set_t attribSet = {};
  attribSet.attributes = attribs;
  attribSet.attributes_count = 1;
  params.layout_attributes = attribSet;

  // Create layout from UTF-8 text.
  ezStringBuilder sTextStr(sText);
  skb_layout_t* pLayout = skb_layout_create_utf8(nullptr, &params, sTextStr.GetData(), -1, attribSet);
  if (pLayout == nullptr)
    return 0;

  // Get layout results.
  const int32_t iRunCount = skb_layout_get_layout_runs_count(pLayout);
  const skb_layout_run_t* pRuns = skb_layout_get_layout_runs(pLayout);
  const int32_t iGlyphCount = skb_layout_get_glyphs_count(pLayout);
  const skb_glyph_t* pGlyphs = skb_layout_get_glyphs(pLayout);

  const float fPixelScale = 1.0f;
  const skb_color_t color = {
    static_cast<uint8_t>(textColor.r * 255.0f),
    static_cast<uint8_t>(textColor.g * 255.0f),
    static_cast<uint8_t>(textColor.b * 255.0f),
    static_cast<uint8_t>(textColor.a * 255.0f)};

  ezUInt32 uiQuadsAdded = 0;

  // Iterate runs and glyphs, produce atlas quads.
  for (int32_t r = 0; r < iRunCount; r++)
  {
    const skb_layout_run_t& run = pRuns[r];

    for (int32_t g = run.glyph_range.start; g < run.glyph_range.end; g++)
    {
      const skb_glyph_t& glyph = pGlyphs[g];

      float gx = fX + glyph.offset_x;
      float gy = fY + glyph.offset_y;

      skb_quad_t quad = skb_image_atlas_get_glyph_quad(
        m_pAtlas, gx, gy, fPixelScale,
        m_pFontCollection, run.font_handle, glyph.gid, run.font_size,
        color, SKB_RASTERIZE_ALPHA_SDF);

      // Convert skb_quad_t to ezRazorGlyphQuad.
      ezRazorGlyphQuad razorQuad;
      razorQuad.m_Rect = ezRectFloat(quad.geom.x, quad.geom.y,
                                      quad.geom.width,
                                      quad.geom.height);
      razorQuad.m_vUVTopLeft = ezVec2(quad.texture.x, quad.texture.y);
      razorQuad.m_vUVBottomRight = ezVec2(quad.texture.x + quad.texture.width, quad.texture.y + quad.texture.height);
      razorQuad.m_Color = textColor;
      razorQuad.m_uiAtlasIndex = quad.texture_idx;
      razorQuad.m_bIsSDF = (quad.flags & SKB_QUAD_IS_SDF) != 0;
      razorQuad.m_bIsColor = (quad.flags & SKB_QUAD_IS_COLOR) != 0;

      out_glyphs.AddGlyph(razorQuad);
      uiQuadsAdded++;
    }
  }

  skb_layout_destroy(pLayout);

  return uiQuadsAdded;
}

bool ezRazorTextRenderer::RasterizeMissingGlyphs()
{
  if (m_pAtlas == nullptr || m_pRasterizer == nullptr)
    return false;

  return skb_image_atlas_rasterize_missing_items(m_pAtlas, nullptr, m_pRasterizer);
}

void ezRazorTextRenderer::CompactAtlas()
{
  if (m_pAtlas != nullptr)
  {
    skb_image_atlas_compact(m_pAtlas);
  }
}

ezUInt32 ezRazorTextRenderer::GetAtlasTextureCount() const
{
  if (m_pAtlas == nullptr)
    return 0;

  return static_cast<ezUInt32>(skb_image_atlas_get_texture_count(m_pAtlas));
}

ezRazorTextRenderer::AtlasTextureInfo ezRazorTextRenderer::GetAtlasTexture(ezUInt32 uiIndex) const
{
  AtlasTextureInfo info;
  if (m_pAtlas == nullptr)
    return info;

  const skb_image_t* pImg = skb_image_atlas_get_texture(m_pAtlas, static_cast<int32_t>(uiIndex));
  if (pImg == nullptr)
    return info;

  info.m_pPixels = pImg->buffer;
  info.m_uiWidth = static_cast<ezUInt32>(pImg->width);
  info.m_uiHeight = static_cast<ezUInt32>(pImg->height);
  info.m_uiChannels = pImg->bpp;

  return info;
}

ezRazorTextRenderer::DirtyRect ezRazorTextRenderer::GetAndResetDirtyBounds(ezUInt32 uiTextureIndex)
{
  DirtyRect result;
  if (m_pAtlas == nullptr)
    return result;

  skb_rect2i_t dirty = skb_image_atlas_get_and_reset_texture_dirty_bounds(
    m_pAtlas, static_cast<int32_t>(uiTextureIndex));

  result.m_iX = dirty.x;
  result.m_iY = dirty.y;
  result.m_iWidth = dirty.width;
  result.m_iHeight = dirty.height;

  return result;
}

EZ_STATICLINK_FILE(RazorCore, RazorCore_Render_RazorTextRenderer);
