#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/Lights/ClusteredDataProvider.h>
#include <RendererCore/Lights/ScreenSpaceShadowDataProvider.h>
#include <RendererCore/Pipeline/Passes/ScreenSpaceShadowPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/Texture.h>

#include <RendererCore/../../../Data/Base/Shaders/Common/LightData.h>
#include <RendererCore/../../../Data/Base/Shaders/Pipeline/bend_sss_cpu.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezScreenSpaceShadowPass, 1, ezRTTIDefaultAllocator<ezScreenSpaceShadowPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("DepthStencil", m_PinDepthStencil),
    EZ_MEMBER_PROPERTY("SurfaceThickness", m_fSurfaceThickness)
      ->AddAttributes(new ezDefaultValueAttribute(0.005f), new ezClampValueAttribute(0.001f, 0.1f)),
    EZ_MEMBER_PROPERTY("BilinearThreshold", m_fBilinearThreshold)
      ->AddAttributes(new ezDefaultValueAttribute(0.02f), new ezClampValueAttribute(0.001f, 0.1f)),
    EZ_MEMBER_PROPERTY("ShadowContrast", m_fShadowContrast)
      ->AddAttributes(new ezDefaultValueAttribute(4.0f), new ezClampValueAttribute(1.0f, 16.0f)),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Effects")
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezScreenSpaceShadowPass::ezScreenSpaceShadowPass()
  : ezRenderPipelinePass("ScreenSpaceShadowPass", true)
{
  m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/ScreenSpaceShadow.ezShader");
  m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezScreenSpaceShadowConstants>();

  // Point sampler with border-clamp returning white (1.0 = far depth in standard Z)
  {
    ezGALSamplerStateCreationDescription desc;
    desc.m_MinFilter = ezGALTextureFilterMode::Point;
    desc.m_MagFilter = ezGALTextureFilterMode::Point;
    desc.m_MipFilter = ezGALTextureFilterMode::Point;
    desc.m_AddressU = ezImageAddressMode::ClampBorder;
    desc.m_AddressV = ezImageAddressMode::ClampBorder;
    desc.m_AddressW = ezImageAddressMode::ClampBorder;
    desc.m_BorderColor = ezColor::White;
    m_hPointBorderSampler = ezGALDevice::GetDefaultDevice()->CreateSamplerState(desc);
  }
}

ezScreenSpaceShadowPass::~ezScreenSpaceShadowPass()
{
  DestroyResources();
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
}

bool ezScreenSpaceShadowPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  if (!inputs[m_PinDepthStencil.m_uiInputIndex])
  {
    ezLog::Error("No depth input connected to '{0}'!", GetName());
    return false;
  }

  return true;
}

void ezScreenSpaceShadowPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  auto pDepthInput = inputs[m_PinDepthStencil.m_uiInputIndex];
  if (pDepthInput == nullptr)
    return;

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  const ezGALTexture* pDepthTex = pDevice->GetTexture(pDepthInput->m_TextureHandle);
  const ezUInt32 uiWidth = pDepthTex->GetDescription().m_uiWidth;
  const ezUInt32 uiHeight = pDepthTex->GetDescription().m_uiHeight;

  EnsureResources(uiWidth, uiHeight);

  // Get the brightest directional light direction from clustered data.
  // m_vBrightestDirectionalLightDirection is the direction light rays travel (toward scene).
  // Bend SSS expects the direction TO the light, so negate it.
  auto pClusteredData = GetPipeline()->GetFrameDataProvider<ezClusteredDataProvider>()->GetData(renderViewContext);
  ezVec3 lightDir = -pClusteredData->m_vBrightestDirectionalLightDirection;

  auto pProvider = GetPipeline()->GetFrameDataProvider<ezScreenSpaceShadowDataProvider>();

  // If no valid directional light, publish the cleared texture and return
  if (lightDir.IsZero(0.001f))
  {
    if (pProvider != nullptr)
    {
      pProvider->SetTexture(ezGALTextureHandle());
    }
    return;
  }

  // Compute light clip-space projection: VP * float4(lightDir, 0)
  // ezMat4 uses column-major storage with column-vector multiplication (M * v)
  const ezMat4& vp = renderViewContext.m_pViewData->m_ViewProjectionMatrix[0];
  float lightProjection[4];
  lightProjection[0] = vp.Element(0, 0) * lightDir.x + vp.Element(1, 0) * lightDir.y + vp.Element(2, 0) * lightDir.z;
  lightProjection[1] = vp.Element(0, 1) * lightDir.x + vp.Element(1, 1) * lightDir.y + vp.Element(2, 1) * lightDir.z;
  lightProjection[2] = vp.Element(0, 2) * lightDir.x + vp.Element(1, 2) * lightDir.y + vp.Element(2, 2) * lightDir.z;
  lightProjection[3] = vp.Element(0, 3) * lightDir.x + vp.Element(1, 3) * lightDir.y + vp.Element(2, 3) * lightDir.z;

  int viewportSize[2] = {(int)uiWidth, (int)uiHeight};
  int minBounds[2] = {0, 0};
  int maxBounds[2] = {(int)uiWidth, (int)uiHeight};

  Bend::DispatchList dispatches = Bend::BuildDispatchList(lightProjection, viewportSize, minBounds, maxBounds, false, 64);

  if (dispatches.DispatchCount == 0)
  {
    if (pProvider != nullptr)
    {
      pProvider->SetTexture(ezGALTextureHandle());
    }
    return;
  }

  // Clear the shadow texture to 1.0 (no shadow) via a load-op clear
  {
    ezGALRenderingSetup renderingSetup;
    renderingSetup.SetColorTarget(0, pDevice->GetDefaultRenderTargetView(m_hShadowTexture));
    renderingSetup.SetClearColor(0, ezColor::White);

    auto pCommandEncoder = renderViewContext.m_pRenderContext->BeginRenderingScope(renderViewContext, renderingSetup, "SS Shadow Clear", false);
  }

  // Issue dispatches
  for (int d = 0; d < dispatches.DispatchCount; ++d)
  {
    auto& disp = dispatches.Dispatch[d];

    auto* cb = ezRenderContext::GetConstantBufferData<ezScreenSpaceShadowConstants>(m_hConstantBuffer);
    cb->SSSLightCoordinate = ezVec4(dispatches.LightCoordinate_Shader[0], dispatches.LightCoordinate_Shader[1],
      dispatches.LightCoordinate_Shader[2], dispatches.LightCoordinate_Shader[3]);
    cb->SSSWaveOffset.x = disp.WaveOffset_Shader[0];
    cb->SSSWaveOffset.y = disp.WaveOffset_Shader[1];
    cb->SSSSurfaceThickness = m_fSurfaceThickness;
    cb->SSSBilinearThreshold = m_fBilinearThreshold;
    cb->SSSShadowContrast = m_fShadowContrast;
    cb->SSSFarDepthValue = 1.0f;
    cb->SSSNearDepthValue = 0.0f;
    cb->SSSResolutionX = uiWidth;
    cb->SSSResolutionY = uiHeight;
    cb->SSSInvDepthTextureSize = ezVec2(1.0f / uiWidth, 1.0f / uiHeight);

    auto computeScope = renderViewContext.m_pRenderContext->BeginComputeScope(renderViewContext, "SS Shadow Dispatch");

    ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
    bindGroup.BindBuffer("ezScreenSpaceShadowConstants", m_hConstantBuffer);
    bindGroup.BindTexture("DepthTexture", pDepthInput->m_TextureHandle);
    bindGroup.BindTexture("OutputTexture", m_hShadowTexture);
    bindGroup.BindSampler("PointBorderSampler", m_hPointBorderSampler);

    renderViewContext.m_pRenderContext->BindShader(m_hShader);
    renderViewContext.m_pRenderContext->Dispatch(disp.WaveCount[0], disp.WaveCount[1], disp.WaveCount[2]).IgnoreResult();
  }

  // Publish the shadow texture and enable the flag in clustered constants
  if (pProvider != nullptr)
  {
    pProvider->SetTexture(m_hShadowTexture);
  }

  // Set the ScreenSpaceShadowEnabled flag so Lighting.h knows to sample the texture
  {
    auto* pConstants = ezRenderContext::GetConstantBufferData<ezClusteredDataConstants>(pClusteredData->m_hConstantBuffer);
    pConstants->ScreenSpaceShadowEnabled = 1;
  }
}

void ezScreenSpaceShadowPass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  auto pProvider = GetPipeline()->GetFrameDataProvider<ezScreenSpaceShadowDataProvider>();
  if (pProvider != nullptr)
  {
    pProvider->SetTexture(ezGALTextureHandle());
  }
}

void ezScreenSpaceShadowPass::EnsureResources(ezUInt32 uiWidth, ezUInt32 uiHeight)
{
  if (m_uiWidth == uiWidth && m_uiHeight == uiHeight && !m_hShadowTexture.IsInvalidated())
    return;

  DestroyResources();

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  ezGALTextureCreationDescription desc;
  desc.m_uiWidth = uiWidth;
  desc.m_uiHeight = uiHeight;
  desc.m_Format = ezGALResourceFormat::RFloat;
  desc.m_TextureFlags = ezGALTextureUsageFlags::ShaderResource | ezGALTextureUsageFlags::UnorderedAccess | ezGALTextureUsageFlags::RenderTarget;
  desc.m_ResourceAccess.m_bImmutable = false;

  m_hShadowTexture = pDevice->CreateTexture(desc);

  m_uiWidth = uiWidth;
  m_uiHeight = uiHeight;
}

void ezScreenSpaceShadowPass::DestroyResources()
{
  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  if (!m_hShadowTexture.IsInvalidated())
    pDevice->DestroyTexture(m_hShadowTexture);

  m_hShadowTexture.Invalidate();
  m_uiWidth = 0;
  m_uiHeight = 0;
}

ezResult ezScreenSpaceShadowPass::Serialize(ezStreamWriter& inout_stream) const
{
  EZ_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << m_fSurfaceThickness;
  inout_stream << m_fBilinearThreshold;
  inout_stream << m_fShadowContrast;
  return EZ_SUCCESS;
}

ezResult ezScreenSpaceShadowPass::Deserialize(ezStreamReader& inout_stream)
{
  EZ_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  inout_stream >> m_fSurfaceThickness;
  inout_stream >> m_fBilinearThreshold;
  inout_stream >> m_fShadowContrast;
  return EZ_SUCCESS;
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_ScreenSpaceShadowPass);
