#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ShaderResource.h>

/// Compute pass that marks which virtual shadow map pages are needed.
///
/// Reads the scene depth buffer and projects each visible pixel back to shadow space.
/// For each pixel, determines which clipmap level and virtual page it falls into, then
/// atomically marks that page as requested in the PageRequestBuffer. The CPU then reads
/// back these requests to allocate physical pages.
class EZ_RENDERERCORE_DLL ezVSMPageMarkingPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezVSMPageMarkingPass, ezRenderPipelinePass);

public:
  ezVSMPageMarkingPass();
  ~ezVSMPageMarkingPass();

  virtual bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;
  virtual void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  virtual void ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

protected:
  ezRenderPipelineNodeInputPin m_PinDepthInput;

  ezShaderResourceHandle m_hShaderPageMarking;
};
