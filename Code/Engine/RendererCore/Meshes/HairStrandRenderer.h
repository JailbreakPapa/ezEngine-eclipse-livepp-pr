#pragma once

#include <RendererCore/Pipeline/Renderer.h>
#include <RendererCore/Pipeline/RenderDataBatch.h>
#include <RendererCore/Meshes/HairStrandResource.h>

/// Render data emitted by ezHairComponent in strand mode.
///
/// Contains handles to the hair strand resource and material, plus per-instance
/// animation parameters. Tessellation is handled by ezHairStrandTessellationPass;
/// drawing is handled by ezHairStrandRenderer.
class EZ_RENDERERCORE_DLL ezHairStrandRenderData : public ezRenderData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezHairStrandRenderData, ezRenderData);

public:
  ezHairStrandResourceHandle m_hStrandResource;
  ezMaterialResourceHandle m_hMaterial;
  ezTransform m_GlobalTransform;
  ezColor m_Color = ezColor::White;
  float m_fWindInfluence = 0.5f;
  float m_fGravityStrength = 1.0f;
  float m_fStiffness = 0.5f;
  float m_fWidthScale = 1.0f;
  float m_fStrandDensity = 1.0f;
};

/// Renders pre-tessellated hair strands using null mesh buffer + SV_VertexID.
///
/// Tessellation must already have been performed by ezHairStrandTessellationPass.
/// This renderer only binds the material and draws the triangle strips.
class EZ_RENDERERCORE_DLL ezHairStrandRenderer : public ezRenderer
{
  EZ_ADD_DYNAMIC_REFLECTION(ezHairStrandRenderer, ezRenderer);
  EZ_DISALLOW_COPY_AND_ASSIGN(ezHairStrandRenderer);

public:
  ezHairStrandRenderer();
  ~ezHairStrandRenderer();

  virtual void GetSupportedRenderDataTypes(ezDynamicArray<const ezRTTI*>& out_types) const override;
  virtual void RenderBatch(
    const ezRenderViewContext& renderViewContext, const ezRenderPipelinePass* pPass, const ezRenderDataBatch& batch) const override;
};
