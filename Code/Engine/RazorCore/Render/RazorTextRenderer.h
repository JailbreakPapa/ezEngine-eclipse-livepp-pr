#pragma once

#include <RazorCore/RazorCoreDLL.h>
#include <RazorCore/Render/RazorGlyphQuad.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Rect.h>
#include <Foundation/Math/Vec2.h>
#include <Foundation/Strings/String.h>

struct skb_font_collection_t;
struct skb_image_atlas_t;
struct skb_rasterizer_t;

/// Manages font loading, text shaping, glyph atlas, and quad generation via Skribidi.
///
/// One instance is shared by all Razor documents in a process. Owns the font collection,
/// the glyph image atlas, and the Skribidi rasterizer. Call `ShapeText()` to lay out a
/// text run and get glyph quads for rendering.
///
/// The atlas is updated lazily — new glyphs are rasterized on the next call to
/// `RasterizeMissingGlyphs()`. Dirty atlas texture regions can be queried and uploaded to GPU.
class EZ_RAZORCORE_DLL ezRazorTextRenderer
{
  EZ_DISALLOW_COPY_AND_ASSIGN(ezRazorTextRenderer);

public:
  ezRazorTextRenderer();
  ~ezRazorTextRenderer();

  /// Initializes the font collection and atlas. Call once at startup.
  void Initialize();

  /// Shuts down and releases all Skribidi resources.
  void Shutdown();

  bool IsInitialized() const { return m_pFontCollection != nullptr; }

  /// Adds a font from file. Returns a font handle (0 = failure).
  ezUInt32 AddFontFromFile(ezStringView sPath, ezStringView sFamilyName);

  /// Adds a font from memory. The data must remain valid until Shutdown.
  ezUInt32 AddFontFromMemory(ezStringView sName, ezArrayPtr<const ezUInt8> data, ezStringView sFamilyName);

  /// Shapes a text string and produces glyph quads positioned at (fX, fY).
  ///
  /// Quads are appended to `out_glyphs`. Returns the number of quads added.
  /// Uses the font collection for fallback matching.
  ezUInt32 ShapeText(ezStringView sText, float fFontSize, const ezColor& textColor,
                     float fX, float fY, float fMaxWidth,
                     ezRazorGlyphBuffer& out_glyphs);

  /// Call once per frame after all ShapeText calls to rasterize newly needed glyphs.
  /// Returns true if any atlas textures were updated (dirty regions need GPU upload).
  bool RasterizeMissingGlyphs();

  /// Compacts the atlas by evicting unused glyphs. Call once per frame.
  void CompactAtlas();

  /// Returns the number of atlas textures.
  ezUInt32 GetAtlasTextureCount() const;

  /// Returns raw atlas texture data (for GPU upload). Pixels are alpha8 or RGBA.
  struct AtlasTextureInfo
  {
    const void* m_pPixels = nullptr;
    ezUInt32 m_uiWidth = 0;
    ezUInt32 m_uiHeight = 0;
    ezUInt8 m_uiChannels = 0; ///< 1 = alpha/SDF, 4 = color.
  };

  AtlasTextureInfo GetAtlasTexture(ezUInt32 uiIndex) const;

  struct DirtyRect
  {
    ezInt32 m_iX = 0, m_iY = 0, m_iWidth = 0, m_iHeight = 0;
    bool IsEmpty() const { return m_iWidth <= 0 || m_iHeight <= 0; }
  };

  /// Gets and resets the dirty region for an atlas texture.
  DirtyRect GetAndResetDirtyBounds(ezUInt32 uiTextureIndex);

  /// Access the raw Skribidi font collection (for advanced use).
  skb_font_collection_t* GetFontCollection() { return m_pFontCollection; }
  skb_image_atlas_t* GetImageAtlas() { return m_pAtlas; }

private:
  skb_font_collection_t* m_pFontCollection = nullptr;
  skb_image_atlas_t* m_pAtlas = nullptr;
  skb_rasterizer_t* m_pRasterizer = nullptr;
};
