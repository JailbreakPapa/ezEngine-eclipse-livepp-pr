#pragma once

#include <RazorPlugin/RazorPluginDLL.h>

#include <RendererCore/Pipeline/RenderData.h>
#include <RendererFoundation/RendererFoundationDLL.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Rect.h>
#include <RendererFoundation/Resources/RenderTargetSetup.h>

class ezRazorRenderBatch;

/// Razor UI vertex for GPU submission.
struct EZ_RAZORPLUGIN_DLL ezRazorVertex
{
  EZ_DECLARE_POD_TYPE();

  float m_fPosX;
  float m_fPosY;
  float m_fTexU;
  float m_fTexV;
  ezUInt32 m_uiColor; ///< RGBA8 packed.

  static ezUInt32 PackColor(const ezColor& color);
};

/// Describes one draw call within a Razor render batch.
struct EZ_RAZORPLUGIN_DLL ezRazorDrawCall
{
  EZ_DECLARE_POD_TYPE();

  ezUInt32 m_uiFirstIndex = 0;
  ezUInt32 m_uiIndexCount = 0;
  ezGALTextureHandle m_hTexture;

  enum class Mode : ezUInt8
  {
    Solid,
    Textured,
    SDFText,
    AlphaText,
  };
  Mode m_Mode = Mode::Solid;

  ezRectFloat m_ScissorRect;
  bool m_bHasScissor = false;
};

/// Render data submitted to the ezEngine render pipeline for Razor UI.
///
/// Contains pre-built vertex/index data and a list of draw calls.
/// The RazorRenderer processes these in the GUI render pass.
class EZ_RAZORPLUGIN_DLL ezRazorRenderData : public ezRenderData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorRenderData, ezRenderData);

public:
  ezDynamicArray<ezRazorVertex> m_Vertices;
  ezDynamicArray<ezUInt16> m_Indices;
  ezDynamicArray<ezRazorDrawCall> m_DrawCalls;

  /// When valid, render to this texture instead of the backbuffer (used by Canvas3D).
  ezGALTextureHandle m_hTargetTexture;
};
