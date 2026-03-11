#pragma once

#include <RendererCore/../../../Data/Base/Shaders/Pipeline/VolumetricCloudsConstants.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererFoundation/RendererFoundationDLL.h>

/// Ray-marched volumetric cloud rendering pass.
///
/// Renders procedural clouds at quarter resolution using a compute shader ray-march,
/// then temporally upscales and composites with the scene. Uses Worley+Perlin noise
/// for cloud shape/detail, a weather map for coverage, and Henyey-Greenstein phase
/// function for light scattering.
class EZ_RENDERERCORE_DLL ezVolumetricCloudsPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezVolumetricCloudsPass, ezRenderPipelinePass);

public:
  ezVolumetricCloudsPass();
  ~ezVolumetricCloudsPass();

  virtual bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;

  virtual void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  virtual void ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  virtual ezResult Serialize(ezStreamWriter& inout_stream) const override;
  virtual ezResult Deserialize(ezStreamReader& inout_stream) override;

  void SetCloudLayerBottom(float fAltitude);  // [ property ]
  float GetCloudLayerBottom() const;          // [ property ]
  void SetCloudLayerTop(float fAltitude);     // [ property ]
  float GetCloudLayerTop() const;             // [ property ]
  void SetCloudCoverage(float fCoverage);     // [ property ]
  float GetCloudCoverage() const;             // [ property ]
  void SetCloudDensity(float fDensity);       // [ property ]
  float GetCloudDensity() const;              // [ property ]

protected:
  ezRenderPipelineNodeInputPin m_PinDepthInput;
  ezRenderPipelineNodePassThroughPin m_PinColor;

  ezConstantBufferStorageHandle m_hConstantBuffer;
  ezShaderResourceHandle m_hShaderCloudRaymarch;
  ezShaderResourceHandle m_hShaderCloudComposite;

  float m_fCloudLayerBottom = 100.0f;
  float m_fCloudLayerTop = 250.0f;
  float m_fCloudCoverage = 0.5f;
  float m_fCloudDensity = 0.05f;
  float m_fCloudAbsorption = 0.04f;
  float m_fWindSpeed = 10.0f;
  float m_fPhaseG = 0.6f;
  float m_fSilverLiningIntensity = 0.5f;
  float m_fSilverLiningSpread = 8.0f;

  ezMat4 m_PrevViewProjectionMatrix = ezMat4::MakeIdentity();
  ezUInt32 m_uiFrameIndex = 0;

  // Temporal history: double-buffered cloud-only data (inscatter RGB + transmittance A)
  ezGALTextureHandle m_hCloudHistoryA;
  ezGALTextureHandle m_hCloudHistoryB;
  bool m_bUseHistoryA = true;
  ezUInt32 m_uiHistoryWidth = 0;
  ezUInt32 m_uiHistoryHeight = 0;

  void EnsureHistoryTextures(ezUInt32 uiWidth, ezUInt32 uiHeight);
  void DestroyHistoryTextures();
};
