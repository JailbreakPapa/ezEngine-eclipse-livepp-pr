#pragma once

#include <RendererCore/../../../Data/Base/Shaders/Pipeline/DeferredLightingConstants.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererFoundation/RendererFoundationDLL.h>

/// Fullscreen deferred lighting pass.
///
/// Reads the GBuffer produced by ezGBufferPass (albedo, normal, roughness, metallic, emission, AO),
/// reconstructs world-space positions from depth, and evaluates clustered lighting using the same
/// BRDF as the forward path. Outputs a fully lit HDR color buffer.
class EZ_RENDERERCORE_DLL ezDeferredLightingPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezDeferredLightingPass, ezRenderPipelinePass);

public:
  ezDeferredLightingPass(const char* szName = "DeferredLightingPass");
  ~ezDeferredLightingPass();

  virtual bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;
  virtual void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

protected:
  ezRenderPipelineNodeInputPin m_PinGBuffer0;    // Albedo + Metallic
  ezRenderPipelineNodeInputPin m_PinGBuffer1;    // Normal + Roughness + Flags
  ezRenderPipelineNodeInputPin m_PinGBuffer2;    // Emission + AO
  ezRenderPipelineNodeInputPin m_PinDepthStencil;
  ezRenderPipelineNodeInputPin m_PinSSAO;        // Optional SSAO input
  ezRenderPipelineNodeOutputPin m_PinColorOutput;

  ezConstantBufferStorageHandle m_hConstantBuffer;
  ezShaderResourceHandle m_hShader;
};
