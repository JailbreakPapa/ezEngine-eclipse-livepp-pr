#pragma once

#include <RendererCore/../../../Data/Base/Shaders/Pipeline/SSSConstants.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererFoundation/RendererFoundationDLL.h>

/// Separable screen-space subsurface scattering blur pass.
///
/// Applies a two-pass (horizontal + vertical) depth-aware blur to pixels identified
/// as subsurface scattering materials. Uses diffusion profile kernels that approximate
/// real light transport through translucent materials like skin, wax, or jade.
/// The blur is depth-weighted to prevent bleeding across geometric edges.
///
/// In the deferred path, SSS pixels are identified by the material flag bits in GBuffer RT1.
/// In the forward path, SSS objects write a stencil bit during the opaque pass.
class EZ_RENDERERCORE_DLL ezSSSBlurPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezSSSBlurPass, ezRenderPipelinePass);

public:
  ezSSSBlurPass(const char* szName = "SSSBlurPass");
  ~ezSSSBlurPass();

  virtual bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;
  virtual void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

  virtual ezResult Serialize(ezStreamWriter& inout_stream) const override;
  virtual ezResult Deserialize(ezStreamReader& inout_stream) override;

  void SetSSSStrength(float fStrength);
  float GetSSSStrength() const;

  void SetKernelSize(ezUInt32 uiSize);
  ezUInt32 GetKernelSize() const;

  void SetDepthThreshold(float fThreshold);
  float GetDepthThreshold() const;

  void SetProfileIndex(ezUInt32 uiIndex);
  ezUInt32 GetProfileIndex() const;

protected:
  ezRenderPipelineNodeInputPin m_PinColorInput;
  ezRenderPipelineNodeInputPin m_PinDepthInput;
  ezRenderPipelineNodeInputPin m_PinGBuffer1Input; // For material flag identification
  ezRenderPipelineNodeOutputPin m_PinColorOutput;

  float m_fSSSStrength = 1.0f;
  ezUInt32 m_uiKernelSize = 25;
  float m_fDepthThreshold = 0.05f;
  ezUInt32 m_uiProfileIndex = 0; // 0=skin, 1=wax, 2=jade, 3=milk

  ezConstantBufferStorageHandle m_hConstantBuffer;
  ezShaderResourceHandle m_hShader;
};
