#pragma once

#include <RendererCore/../../../Data/Base/Shaders/Pipeline/VolumetricFogConstants.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererFoundation/RendererFoundationDLL.h>

/// Froxel-based volumetric fog rendering pass.
///
/// Computes volumetric light scattering in a 3D froxel grid using three compute dispatches:
/// injection (per-light scattering via Henyey-Greenstein phase function),
/// temporal reprojection (blending with the previous frame for stability),
/// and front-to-back integration (accumulating inscatter and transmittance).
///
/// A final fullscreen pass composites the result with the scene color.
/// Requires a VolumetricFogComponent in the scene to provide fog parameters.
class EZ_RENDERERCORE_DLL ezVolumetricFogPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezVolumetricFogPass, ezRenderPipelinePass);

public:
  ezVolumetricFogPass();
  ~ezVolumetricFogPass();

  virtual bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;
  virtual void InitRenderPipelinePass(const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

  virtual void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  virtual void ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;
  virtual ezResult Serialize(ezStreamWriter& inout_stream) const override;
  virtual ezResult Deserialize(ezStreamReader& inout_stream) override;

  void SetFroxelGridSizeX(ezUInt32 uiSize);   // [ property ]
  ezUInt32 GetFroxelGridSizeX() const;         // [ property ]
  void SetFroxelGridSizeY(ezUInt32 uiSize);   // [ property ]
  ezUInt32 GetFroxelGridSizeY() const;         // [ property ]
  void SetFroxelGridSizeZ(ezUInt32 uiSize);   // [ property ]
  ezUInt32 GetFroxelGridSizeZ() const;         // [ property ]

protected:
  void EnsureFroxelTextures();
  void DestroyFroxelTextures();

  ezRenderPipelineNodeInputPin m_PinDepthInput;
  ezRenderPipelineNodePassThroughPin m_PinColor;

  ezConstantBufferStorageHandle m_hConstantBuffer;

  ezShaderResourceHandle m_hShaderInject;
  ezShaderResourceHandle m_hShaderTemporal;
  ezShaderResourceHandle m_hShaderIntegrate;
  ezShaderResourceHandle m_hShaderApply;

  // Froxel grid 3D textures (double-buffered for temporal reprojection)
  ezGALTextureHandle m_hFroxelGridA;
  ezGALTextureHandle m_hFroxelGridB;
  ezGALTextureHandle m_hIntegratedFroxelGrid;
  bool m_bUseGridA = true; // Ping-pong flag

  ezUInt32 m_uiFroxelGridSizeX = 160;
  ezUInt32 m_uiFroxelGridSizeY = 90;
  ezUInt32 m_uiFroxelGridSizeZ = 64;
  bool m_bFroxelTexturesDirty = true;

  ezMat4 m_PrevViewProjectionMatrix = ezMat4::MakeIdentity();
  bool m_bFirstFrame = true;
};
