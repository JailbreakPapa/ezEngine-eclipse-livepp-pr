#pragma once

#include <RendererCore/../../../Data/Base/Shaders/Particles/GPUParticleConstants.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererFoundation/RendererFoundationDLL.h>

/// Compute pass that simulates GPU particles.
///
/// Runs before the transparent forward pass. Consumes depth for screen-space collision
/// and optionally generates an SDF volume for volumetric collision.
/// Each frame: emit new particles, simulate physics, handle collision, update size/color.
class EZ_RENDERERCORE_DLL ezGPUParticlePass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezGPUParticlePass, ezRenderPipelinePass);

public:
  ezGPUParticlePass();
  ~ezGPUParticlePass();

  virtual bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;
  virtual void InitRenderPipelinePass(const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

  virtual void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  virtual void ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

  virtual ezResult Serialize(ezStreamWriter& inout_stream) const override;
  virtual ezResult Deserialize(ezStreamReader& inout_stream) override;

  void SetSDFResolution(ezUInt32 uiRes);
  ezUInt32 GetSDFResolution() const;

  void SetSDFWorldExtent(float fExtent);
  float GetSDFWorldExtent() const;

protected:
  void EnsureSDFTexture();
  void DestroySDFTexture();

  ezRenderPipelineNodeInputPin m_PinDepthInput;
  ezRenderPipelineNodePassThroughPin m_PinColor;

  ezConstantBufferStorageHandle m_hConstantBuffer;

  ezShaderResourceHandle m_hEmitShader;
  ezShaderResourceHandle m_hSimulateShader;
  ezShaderResourceHandle m_hSDFShader;

  ezGALTextureHandle m_hSDFTexture;
  ezUInt32 m_uiSDFResolution = 64;
  float m_fSDFWorldExtent = 50.0f;
  bool m_bSDFDirty = true;
};
