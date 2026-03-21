#pragma once

#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ShaderResource.h>

/// Compute pass that tessellates hair strand control points into camera-facing triangle strip vertices.
///
/// Must run before any pass that renders the LitHair category (DepthOnlyPass, TransparentForwardRenderPass).
/// Uses a pass-through color pin to establish pipeline ordering.
class EZ_RENDERERCORE_DLL ezHairStrandTessellationPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezHairStrandTessellationPass, ezRenderPipelinePass);

public:
  ezHairStrandTessellationPass();
  ~ezHairStrandTessellationPass();

  virtual bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;

  virtual void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

protected:
  ezRenderPipelineNodePassThroughPin m_PinColor;

  ezShaderResourceHandle m_hTessellateShader;
};
