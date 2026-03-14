#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Lights/ClusteredDataProvider.h>
#include <RendererCore/Pipeline/Passes/VolumetricCloudsPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/Texture.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezVolumetricCloudsPass, 2, ezRTTIDefaultAllocator<ezVolumetricCloudsPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("DepthInput", m_PinDepthInput),
    EZ_MEMBER_PROPERTY("Color", m_PinColor),
    EZ_ACCESSOR_PROPERTY("CloudLayerBottom", GetCloudLayerBottom, SetCloudLayerBottom)->AddAttributes(new ezDefaultValueAttribute(100.0f)),
    EZ_ACCESSOR_PROPERTY("CloudLayerTop", GetCloudLayerTop, SetCloudLayerTop)->AddAttributes(new ezDefaultValueAttribute(250.0f)),
    EZ_ACCESSOR_PROPERTY("CloudCoverage", GetCloudCoverage, SetCloudCoverage)->AddAttributes(new ezClampValueAttribute(0.0f, 1.0f), new ezDefaultValueAttribute(0.5f)),
    EZ_ACCESSOR_PROPERTY("CloudDensity", GetCloudDensity, SetCloudDensity)->AddAttributes(new ezClampValueAttribute(0.0f, ezVariant()), new ezDefaultValueAttribute(0.3f)),
    EZ_ACCESSOR_PROPERTY("CloudAbsorption", GetCloudAbsorption, SetCloudAbsorption)->AddAttributes(new ezClampValueAttribute(0.0f, ezVariant()), new ezDefaultValueAttribute(0.04f)),
    EZ_ACCESSOR_PROPERTY("WindSpeed", GetWindSpeed, SetWindSpeed)->AddAttributes(new ezClampValueAttribute(0.0f, ezVariant()), new ezDefaultValueAttribute(10.0f)),
    EZ_ACCESSOR_PROPERTY("PhaseG", GetPhaseG, SetPhaseG)->AddAttributes(new ezClampValueAttribute(0.0f, 0.99f), new ezDefaultValueAttribute(0.6f)),
    EZ_ACCESSOR_PROPERTY("SilverLiningIntensity", GetSilverLiningIntensity, SetSilverLiningIntensity)->AddAttributes(new ezClampValueAttribute(0.0f, ezVariant()), new ezDefaultValueAttribute(0.5f)),
    EZ_ACCESSOR_PROPERTY("SilverLiningSpread", GetSilverLiningSpread, SetSilverLiningSpread)->AddAttributes(new ezClampValueAttribute(1.0f, 32.0f), new ezDefaultValueAttribute(8.0f)),
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

ezVolumetricCloudsPass::ezVolumetricCloudsPass()
  : ezRenderPipelinePass("VolumetricCloudsPass", true)
{
  m_hShaderCloudRaymarch = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/VolumetricClouds.ezShader");
  m_hShaderCloudComposite = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/VolumetricCloudsComposite.ezShader");

  m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezVolumetricCloudsConstants>();
}

ezVolumetricCloudsPass::~ezVolumetricCloudsPass()
{
  DestroyHistoryTextures();
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
}

bool ezVolumetricCloudsPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  if (!inputs[m_PinDepthInput.m_uiInputIndex])
  {
    ezLog::Error("No depth input connected to '{0}'!", GetName());
    return false;
  }

  // PassThrough: copy input description to output so the pipeline knows they share the same texture
  if (inputs[m_PinColor.m_uiInputIndex])
  {
    outputs[m_PinColor.m_uiOutputIndex] = *inputs[m_PinColor.m_uiInputIndex];
  }

  return true;
}

void ezVolumetricCloudsPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  auto pColorOutput = outputs[m_PinColor.m_uiOutputIndex];
  if (pColorOutput == nullptr)
    return;

  auto pDepthInput = inputs[m_PinDepthInput.m_uiInputIndex];
  if (pDepthInput == nullptr)
    return;

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  // Get clustered data for sun direction
  auto pClusteredData = GetPipeline()->GetFrameDataProvider<ezClusteredDataProvider>()->GetData(renderViewContext);

  // Get quarter resolution for cloud ray-march
  const ezGALTexture* pColorTex = pDevice->GetTexture(pColorOutput->m_TextureHandle);
  ezUInt32 quarterWidth = ezMath::Max(pColorTex->GetDescription().m_uiWidth / 4, 1u);
  ezUInt32 quarterHeight = ezMath::Max(pColorTex->GetDescription().m_uiHeight / 4, 1u);

  // Acquire temporary quarter-res texture for cloud ray-march
  ezGALTextureCreationDescription quarterResDesc;
  quarterResDesc.m_uiWidth = quarterWidth;
  quarterResDesc.m_uiHeight = quarterHeight;
  quarterResDesc.m_Format = ezGALResourceFormat::RGBAHalf;
  quarterResDesc.m_TextureFlags = ezGALTextureUsageFlags::ShaderResource | ezGALTextureUsageFlags::UnorderedAccess | ezGALTextureUsageFlags::RenderTarget;
  quarterResDesc.m_ResourceAccess.m_bImmutable = false;

  ezGALTextureHandle hQuarterResTex = ezGPUResourcePool::GetDefaultInstance()->GetRenderTarget(quarterResDesc);

  // Update constant buffer
  {
    ezVolumetricCloudsConstants* cb = ezRenderContext::GetConstantBufferData<ezVolumetricCloudsConstants>(m_hConstantBuffer);
    cb->CloudLayerBottomAltitude = m_fCloudLayerBottom;
    cb->CloudLayerTopAltitude = m_fCloudLayerTop;
    cb->CloudCoverage = m_fCloudCoverage;
    cb->CloudDensity = m_fCloudDensity;
    cb->CloudWindDirection.Set(1.0f, 0.0f, 0.0f);
    cb->CloudWindSpeed = m_fWindSpeed;
    cb->CloudScatterColor.Set(1.0f, 1.0f, 1.0f);
    cb->CloudAbsorption = m_fCloudAbsorption;
    cb->CloudAmbientColor.Set(0.15f, 0.17f, 0.25f);
    cb->CloudPhaseG = m_fPhaseG;
    cb->CloudDetailScale = 3.0f;
    cb->CloudShapeScale = 1.0f;
    cb->SilverLiningIntensity = m_fSilverLiningIntensity;
    cb->SilverLiningSpread = m_fSilverLiningSpread;
    cb->CloudTextureSize.Set((float)quarterWidth, (float)quarterHeight);
    cb->TemporalBlendWeight = m_uiFrameIndex == 0 ? 1.0f : 0.05f;
    cb->FrameIndex = m_uiFrameIndex;
    cb->PrevWorldToClipMatrix = m_PrevViewProjectionMatrix;
  }

  // Pass 1: Ray-march clouds at quarter resolution (compute)
  {
    EZ_PROFILE_SCOPE("Cloud Ray-march");
    auto computeScope = renderViewContext.m_pRenderContext->BeginComputeScope(renderViewContext, "Cloud Raymarch");

    ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
    bindGroup.BindBuffer("ezVolumetricCloudsConstants", m_hConstantBuffer);
    bindGroup.BindTexture("CloudOutput", hQuarterResTex);
    bindGroup.BindTexture("SceneDepth", pDepthInput->m_TextureHandle);

    pClusteredData->BindResources(renderViewContext.m_pRenderContext);

    renderViewContext.m_pRenderContext->BindShader(m_hShaderCloudRaymarch);

    ezUInt32 dispatchX = (quarterWidth + 7) / 8;
    ezUInt32 dispatchY = (quarterHeight + 7) / 8;
    renderViewContext.m_pRenderContext->Dispatch(dispatchX, dispatchY, 1).IgnoreResult();
  }

  // Pass 2: Composite clouds with scene (fullscreen)
  {
    EZ_PROFILE_SCOPE("Cloud Composite");

    ezGALTextureHandle hColorTarget = pColorOutput->m_TextureHandle;

    // Ensure history textures exist at the correct resolution (quarter-res for temporal)
    EnsureHistoryTextures(quarterWidth, quarterHeight);

    // Determine which history texture to read from (previous frame)
    ezGALTextureHandle hHistoryRead = m_bUseHistoryA ? m_hCloudHistoryB : m_hCloudHistoryA;

    // Allocate temp copy of the scene color
    ezGALTextureCreationDescription tempDesc = pColorOutput->m_Desc;
    tempDesc.m_TextureFlags.Add(ezGALTextureUsageFlags::ShaderResource | ezGALTextureUsageFlags::RenderTarget);
    ezGALTextureHandle hTempSceneColor = ezGPUResourcePool::GetDefaultInstance()->GetRenderTarget(tempDesc);

    // Copy current color to temp
    renderViewContext.m_pRenderContext->GetCommandEncoder()->CopyTexture(hTempSceneColor, hColorTarget);

    // Single render target: composited scene only
    ezGALRenderingSetup renderingSetup;
    renderingSetup.SetColorTarget(0, pDevice->GetDefaultRenderTargetView(hColorTarget));

    auto pCommandEncoder = renderViewContext.m_pRenderContext->BeginRenderingScope(renderViewContext, renderingSetup, "Cloud Composite", renderViewContext.m_pCamera->IsStereoscopic());

    ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
    bindGroup.BindBuffer("ezVolumetricCloudsConstants", m_hConstantBuffer);
    bindGroup.BindTexture("CurrentCloudTexture", hQuarterResTex);
    bindGroup.BindTexture("SceneColor", hTempSceneColor);
    bindGroup.BindTexture("HistoryCloudTexture", hHistoryRead);

    renderViewContext.m_pRenderContext->BindShader(m_hShaderCloudComposite);
    renderViewContext.m_pRenderContext->BindNullMeshBuffer(ezGALPrimitiveTopology::Triangles, 1);
    renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();

    ezGPUResourcePool::GetDefaultInstance()->ReturnRenderTarget(hTempSceneColor);
  }

  // Copy quarter-res cloud result to history for next frame's temporal reprojection
  renderViewContext.m_pRenderContext->GetCommandEncoder()->CopyTexture(
    m_bUseHistoryA ? m_hCloudHistoryA : m_hCloudHistoryB, hQuarterResTex);

  // Return temporary texture
  ezGPUResourcePool::GetDefaultInstance()->ReturnRenderTarget(hQuarterResTex);

  // Store current frame state
  m_PrevViewProjectionMatrix = renderViewContext.m_pViewData->m_ViewProjectionMatrix[0];
  m_uiFrameIndex++;
  m_bUseHistoryA = !m_bUseHistoryA;
}

void ezVolumetricCloudsPass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  // PassThrough: nothing to do, the color buffer is already passed through
}

void ezVolumetricCloudsPass::EnsureHistoryTextures(ezUInt32 uiWidth, ezUInt32 uiHeight)
{
  if (m_uiHistoryWidth == uiWidth && m_uiHistoryHeight == uiHeight && !m_hCloudHistoryA.IsInvalidated())
    return;

  DestroyHistoryTextures();

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  ezGALTextureCreationDescription desc;
  desc.m_uiWidth = uiWidth;
  desc.m_uiHeight = uiHeight;
  desc.m_Format = ezGALResourceFormat::RGBAHalf;
  desc.m_TextureFlags = ezGALTextureUsageFlags::ShaderResource | ezGALTextureUsageFlags::RenderTarget;
  desc.m_ResourceAccess.m_bImmutable = false;

  m_hCloudHistoryA = pDevice->CreateTexture(desc);
  m_hCloudHistoryB = pDevice->CreateTexture(desc);
  m_uiHistoryWidth = uiWidth;
  m_uiHistoryHeight = uiHeight;
}

void ezVolumetricCloudsPass::DestroyHistoryTextures()
{
  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  if (!m_hCloudHistoryA.IsInvalidated())
    pDevice->DestroyTexture(m_hCloudHistoryA);
  if (!m_hCloudHistoryB.IsInvalidated())
    pDevice->DestroyTexture(m_hCloudHistoryB);

  m_hCloudHistoryA.Invalidate();
  m_hCloudHistoryB.Invalidate();
  m_uiHistoryWidth = 0;
  m_uiHistoryHeight = 0;
}

ezResult ezVolumetricCloudsPass::Serialize(ezStreamWriter& inout_stream) const
{
  EZ_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << m_fCloudLayerBottom;
  inout_stream << m_fCloudLayerTop;
  inout_stream << m_fCloudCoverage;
  inout_stream << m_fCloudDensity;
  inout_stream << m_fCloudAbsorption;
  inout_stream << m_fWindSpeed;
  inout_stream << m_fPhaseG;
  inout_stream << m_fSilverLiningIntensity;
  inout_stream << m_fSilverLiningSpread;
  return EZ_SUCCESS;
}

ezResult ezVolumetricCloudsPass::Deserialize(ezStreamReader& inout_stream)
{
  EZ_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const ezUInt32 uiVersion = ezTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  inout_stream >> m_fCloudLayerBottom;
  inout_stream >> m_fCloudLayerTop;
  inout_stream >> m_fCloudCoverage;
  inout_stream >> m_fCloudDensity;

  if (uiVersion >= 2)
  {
    inout_stream >> m_fCloudAbsorption;
    inout_stream >> m_fWindSpeed;
    inout_stream >> m_fPhaseG;
    inout_stream >> m_fSilverLiningIntensity;
    inout_stream >> m_fSilverLiningSpread;
  }
  return EZ_SUCCESS;
}

void ezVolumetricCloudsPass::SetCloudLayerBottom(float fAltitude) { m_fCloudLayerBottom = fAltitude; }
float ezVolumetricCloudsPass::GetCloudLayerBottom() const { return m_fCloudLayerBottom; }

void ezVolumetricCloudsPass::SetCloudLayerTop(float fAltitude) { m_fCloudLayerTop = fAltitude; }
float ezVolumetricCloudsPass::GetCloudLayerTop() const { return m_fCloudLayerTop; }

void ezVolumetricCloudsPass::SetCloudCoverage(float fCoverage) { m_fCloudCoverage = ezMath::Clamp(fCoverage, 0.0f, 1.0f); }
float ezVolumetricCloudsPass::GetCloudCoverage() const { return m_fCloudCoverage; }

void ezVolumetricCloudsPass::SetCloudDensity(float fDensity) { m_fCloudDensity = ezMath::Max(fDensity, 0.0f); }
float ezVolumetricCloudsPass::GetCloudDensity() const { return m_fCloudDensity; }

void ezVolumetricCloudsPass::SetCloudAbsorption(float fAbsorption) { m_fCloudAbsorption = ezMath::Max(fAbsorption, 0.0f); }
float ezVolumetricCloudsPass::GetCloudAbsorption() const { return m_fCloudAbsorption; }

void ezVolumetricCloudsPass::SetWindSpeed(float fSpeed) { m_fWindSpeed = ezMath::Max(fSpeed, 0.0f); }
float ezVolumetricCloudsPass::GetWindSpeed() const { return m_fWindSpeed; }

void ezVolumetricCloudsPass::SetPhaseG(float fG) { m_fPhaseG = ezMath::Clamp(fG, 0.0f, 0.99f); }
float ezVolumetricCloudsPass::GetPhaseG() const { return m_fPhaseG; }

void ezVolumetricCloudsPass::SetSilverLiningIntensity(float fIntensity) { m_fSilverLiningIntensity = ezMath::Max(fIntensity, 0.0f); }
float ezVolumetricCloudsPass::GetSilverLiningIntensity() const { return m_fSilverLiningIntensity; }

void ezVolumetricCloudsPass::SetSilverLiningSpread(float fSpread) { m_fSilverLiningSpread = ezMath::Clamp(fSpread, 1.0f, 32.0f); }
float ezVolumetricCloudsPass::GetSilverLiningSpread() const { return m_fSilverLiningSpread; }

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_VolumetricCloudsPass);
