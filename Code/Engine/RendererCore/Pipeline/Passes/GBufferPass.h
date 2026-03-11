#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>

/// GBuffer fill pass for deferred rendering.
///
/// Renders opaque and masked objects to a multi-render-target GBuffer instead of computing
/// lighting per-pixel. Uses the RENDER_PASS_GBUFFER permutation to make material shaders
/// output encoded surface properties (albedo, normal, roughness, metallic, emission, AO).
/// The resulting GBuffer is consumed by the DeferredLightingPass.
class EZ_RENDERERCORE_DLL ezGBufferPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezGBufferPass, ezRenderPipelinePass);

public:
  ezGBufferPass(const char* szName = "GBufferPass");
  ~ezGBufferPass();

  virtual bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;
  virtual void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

protected:
  ezRenderPipelineNodePassThroughPin m_PinDepthStencil;
  ezRenderPipelineNodeOutputPin m_PinGBuffer0; // Albedo + Metallic (RGBA8)
  ezRenderPipelineNodeOutputPin m_PinGBuffer1; // Normal + Roughness + Flags (RGB10A2)
  ezRenderPipelineNodeOutputPin m_PinGBuffer2; // Emission + AO (RGBA8)
};
