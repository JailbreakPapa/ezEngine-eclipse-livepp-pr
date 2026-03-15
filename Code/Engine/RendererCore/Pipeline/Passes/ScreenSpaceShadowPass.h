#pragma once

#include <RendererCore/../../../Data/Base/Shaders/Pipeline/ScreenSpaceShadowConstants.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererFoundation/RendererFoundationDLL.h>

/// Screen-space contact shadows using Bend Studios' wavefront ray-marching technique.
///
/// Traces rays along the depth buffer from the brightest directional light to add fine contact
/// shadows that shadow maps cannot resolve. The result is published through
/// ezScreenSpaceShadowDataProvider and combined with the shadow term during lighting.
///
/// Place after the depth/GBuffer pass and before DeferredLighting in the pipeline.
class EZ_RENDERERCORE_DLL ezScreenSpaceShadowPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezScreenSpaceShadowPass, ezRenderPipelinePass);

public:
  ezScreenSpaceShadowPass();
  ~ezScreenSpaceShadowPass();

  virtual bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;
  virtual void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  virtual void ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  virtual ezResult Serialize(ezStreamWriter& inout_stream) const override;
  virtual ezResult Deserialize(ezStreamReader& inout_stream) override;

protected:
  void EnsureResources(ezUInt32 uiWidth, ezUInt32 uiHeight);
  void DestroyResources();

  ezRenderPipelineNodeInputPin m_PinDepthStencil;

  ezConstantBufferStorageHandle m_hConstantBuffer;
  ezShaderResourceHandle m_hShader;

  ezGALTextureHandle m_hShadowTexture;
  ezGALSamplerStateHandle m_hPointBorderSampler;

  ezUInt32 m_uiWidth = 0;
  ezUInt32 m_uiHeight = 0;

  // Properties
  float m_fSurfaceThickness = 0.005f;
  float m_fBilinearThreshold = 0.02f;
  float m_fShadowContrast = 4.0f;
};
