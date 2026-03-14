#pragma once

#include <RendererCore/../../../Data/Base/Shaders/Pipeline/GodRaysConstants.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererFoundation/RendererFoundationDLL.h>

/// Screen-space god rays (crepuscular rays) post-processing pass.
///
/// Generates volumetric light shaft effects from the brightest directional light by
/// building an occlusion mask from the depth buffer and applying a radial blur toward
/// the projected light position. The result is additively composited into the scene color.
///
/// Place after deferred lighting / volumetric fog and before tonemapping in the pipeline.
/// Requires a depth input and a color pass-through pin.
class EZ_RENDERERCORE_DLL ezGodRaysPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezGodRaysPass, ezRenderPipelinePass);

public:
  ezGodRaysPass();
  ~ezGodRaysPass();

  virtual bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;
  virtual void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  virtual void ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  virtual ezResult Serialize(ezStreamWriter& inout_stream) const override;
  virtual ezResult Deserialize(ezStreamReader& inout_stream) override;

  void SetNumSamples(ezUInt32 uiSamples);   // [ property ]
  ezUInt32 GetNumSamples() const;            // [ property ]
  void SetIntensity(float fIntensity);       // [ property ]
  float GetIntensity() const;                // [ property ]

protected:
  void EnsureResources(ezUInt32 uiHalfWidth, ezUInt32 uiHalfHeight);
  void DestroyResources();

  ezRenderPipelineNodeInputPin m_PinDepthInput;
  ezRenderPipelineNodePassThroughPin m_PinColor;

  ezConstantBufferStorageHandle m_hConstantBuffer;
  ezShaderResourceHandle m_hOcclusionShader;
  ezShaderResourceHandle m_hRadialBlurShader;
  ezShaderResourceHandle m_hCompositeShader;

  // Half-res textures for occlusion and god rays result
  ezGALTextureHandle m_hOcclusionTexture;
  ezGALTextureHandle m_hGodRaysTexture;
  ezUInt32 m_uiHalfWidth = 0;
  ezUInt32 m_uiHalfHeight = 0;

  // Properties
  ezUInt32 m_uiNumSamples = 64;
  float m_fDensity = 1.0f;
  float m_fWeight = 0.01f;
  float m_fDecay = 0.97f;
  float m_fExposure = 1.0f;
  float m_fIntensity = 1.0f;
  float m_fMaxRayLength = 1.0f;
  float m_fDepthThreshold = 0.9999f;
};
