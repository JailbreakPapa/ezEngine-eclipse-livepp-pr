#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Lights/ClusteredDataProvider.h>
#include <RendererCore/Pipeline/Passes/GodRaysPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/Texture.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezGodRaysPass, 1, ezRTTIDefaultAllocator<ezGodRaysPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("DepthInput", m_PinDepthInput),
    EZ_MEMBER_PROPERTY("Color", m_PinColor),
    EZ_ACCESSOR_PROPERTY("NumSamples", GetNumSamples, SetNumSamples)
      ->AddAttributes(new ezDefaultValueAttribute(64), new ezClampValueAttribute(16, 256)),
    EZ_MEMBER_PROPERTY("Density", m_fDensity)
      ->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.1f, 3.0f)),
    EZ_MEMBER_PROPERTY("Weight", m_fWeight)
      ->AddAttributes(new ezDefaultValueAttribute(0.01f), new ezClampValueAttribute(0.001f, 0.1f)),
    EZ_MEMBER_PROPERTY("Decay", m_fDecay)
      ->AddAttributes(new ezDefaultValueAttribute(0.97f), new ezClampValueAttribute(0.8f, 1.0f)),
    EZ_MEMBER_PROPERTY("Exposure", m_fExposure)
      ->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.0f, 10.0f)),
    EZ_ACCESSOR_PROPERTY("Intensity", GetIntensity, SetIntensity)
      ->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.0f, 10.0f)),
    EZ_MEMBER_PROPERTY("MaxRayLength", m_fMaxRayLength)
      ->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.1f, 2.0f)),
    EZ_MEMBER_PROPERTY("DepthThreshold", m_fDepthThreshold)
      ->AddAttributes(new ezDefaultValueAttribute(0.9999f), new ezClampValueAttribute(0.99f, 1.0f)),
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

ezGodRaysPass::ezGodRaysPass()
  : ezRenderPipelinePass("GodRaysPass", true)
{
  m_hOcclusionShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/GodRaysOcclusion.ezShader");
  m_hRadialBlurShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/GodRaysRadialBlur.ezShader");
  m_hCompositeShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/GodRaysComposite.ezShader");

  m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezGodRaysConstants>();
}

ezGodRaysPass::~ezGodRaysPass()
{
  DestroyResources();
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
}

bool ezGodRaysPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  if (!inputs[m_PinDepthInput.m_uiInputIndex])
  {
    ezLog::Error("No depth input connected to '{0}'!", GetName());
    return false;
  }

  // PassThrough: copy input description to output
  if (inputs[m_PinColor.m_uiInputIndex])
  {
    outputs[m_PinColor.m_uiOutputIndex] = *inputs[m_PinColor.m_uiInputIndex];
  }

  return true;
}

void ezGodRaysPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  auto pColorOutput = outputs[m_PinColor.m_uiOutputIndex];
  if (pColorOutput == nullptr)
    return;

  auto pDepthInput = inputs[m_PinDepthInput.m_uiInputIndex];
  if (pDepthInput == nullptr)
    return;

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  // Bind clustered data so shaders can access BrightestDirectionalLightIndex
  auto pClusteredData = GetPipeline()->GetFrameDataProvider<ezClusteredDataProvider>()->GetData(renderViewContext);

  const ezGALTexture* pColorTex = pDevice->GetTexture(pColorOutput->m_TextureHandle);
  ezUInt32 halfWidth = ezMath::Max(pColorTex->GetDescription().m_uiWidth / 2, 1u);
  ezUInt32 halfHeight = ezMath::Max(pColorTex->GetDescription().m_uiHeight / 2, 1u);

  EnsureResources(halfWidth, halfHeight);

  // Update constant buffer
  {
    ezGodRaysConstants* cb = ezRenderContext::GetConstantBufferData<ezGodRaysConstants>(m_hConstantBuffer);
    cb->GodRaysResolutionX = halfWidth;
    cb->GodRaysResolutionY = halfHeight;
    cb->GodRaysNumSamples = m_uiNumSamples;
    cb->GodRaysDensity = m_fDensity;
    cb->GodRaysWeight = m_fWeight;
    cb->GodRaysDecay = m_fDecay;
    cb->GodRaysExposure = m_fExposure;
    cb->GodRaysIntensity = m_fIntensity;
    cb->GodRaysMaxRayLength = m_fMaxRayLength;
    cb->GodRaysDepthThreshold = m_fDepthThreshold;
  }

  // Pass 1: Generate occlusion mask at half resolution
  {
    EZ_PROFILE_SCOPE("GodRays Occlusion");
    auto computeScope = renderViewContext.m_pRenderContext->BeginComputeScope(renderViewContext, "GodRays Occlusion");

    ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
    bindGroup.BindBuffer("ezGodRaysConstants", m_hConstantBuffer);
    bindGroup.BindTexture("OcclusionOutput", m_hOcclusionTexture);
    bindGroup.BindTexture("SceneDepth", pDepthInput->m_TextureHandle);

    pClusteredData->BindResources(renderViewContext.m_pRenderContext);

    renderViewContext.m_pRenderContext->BindShader(m_hOcclusionShader);

    ezUInt32 dispatchX = (halfWidth + GODRAYS_THREAD_GROUP_X - 1) / GODRAYS_THREAD_GROUP_X;
    ezUInt32 dispatchY = (halfHeight + GODRAYS_THREAD_GROUP_Y - 1) / GODRAYS_THREAD_GROUP_Y;
    renderViewContext.m_pRenderContext->Dispatch(dispatchX, dispatchY, 1).IgnoreResult();
  }

  // Pass 2: Radial blur at half resolution
  {
    EZ_PROFILE_SCOPE("GodRays RadialBlur");
    auto computeScope = renderViewContext.m_pRenderContext->BeginComputeScope(renderViewContext, "GodRays RadialBlur");

    ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
    bindGroup.BindBuffer("ezGodRaysConstants", m_hConstantBuffer);
    bindGroup.BindTexture("OcclusionInput", m_hOcclusionTexture);
    bindGroup.BindTexture("GodRaysOutput", m_hGodRaysTexture);

    pClusteredData->BindResources(renderViewContext.m_pRenderContext);

    renderViewContext.m_pRenderContext->BindShader(m_hRadialBlurShader);

    ezUInt32 dispatchX = (halfWidth + GODRAYS_THREAD_GROUP_X - 1) / GODRAYS_THREAD_GROUP_X;
    ezUInt32 dispatchY = (halfHeight + GODRAYS_THREAD_GROUP_Y - 1) / GODRAYS_THREAD_GROUP_Y;
    renderViewContext.m_pRenderContext->Dispatch(dispatchX, dispatchY, 1).IgnoreResult();
  }

  // Pass 3: Composite into scene color (fullscreen)
  {
    EZ_PROFILE_SCOPE("GodRays Composite");

    ezGALTextureHandle hColorTarget = pColorOutput->m_TextureHandle;

    // Copy current scene color to temp to avoid read/write conflict
    ezGALTextureCreationDescription tempDesc = pColorOutput->m_Desc;
    tempDesc.m_TextureFlags.Add(ezGALTextureUsageFlags::ShaderResource | ezGALTextureUsageFlags::RenderTarget);
    ezGALTextureHandle hTempSceneColor = ezGPUResourcePool::GetDefaultInstance()->GetRenderTarget(tempDesc);

    renderViewContext.m_pRenderContext->GetCommandEncoder()->CopyTexture(hTempSceneColor, hColorTarget);

    ezGALRenderingSetup renderingSetup;
    renderingSetup.SetColorTarget(0, pDevice->GetDefaultRenderTargetView(hColorTarget));

    auto pCommandEncoder = renderViewContext.m_pRenderContext->BeginRenderingScope(renderViewContext, renderingSetup, "GodRays Composite", renderViewContext.m_pCamera->IsStereoscopic());

    ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
    bindGroup.BindBuffer("ezGodRaysConstants", m_hConstantBuffer);
    bindGroup.BindTexture("SceneColor", hTempSceneColor);
    bindGroup.BindTexture("GodRaysTexture", m_hGodRaysTexture);

    renderViewContext.m_pRenderContext->BindShader(m_hCompositeShader);
    renderViewContext.m_pRenderContext->BindNullMeshBuffer(ezGALPrimitiveTopology::Triangles, 1);
    renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();

    ezGPUResourcePool::GetDefaultInstance()->ReturnRenderTarget(hTempSceneColor);
  }
}

void ezGodRaysPass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  // PassThrough: nothing to do
}

void ezGodRaysPass::EnsureResources(ezUInt32 uiHalfWidth, ezUInt32 uiHalfHeight)
{
  if (m_uiHalfWidth == uiHalfWidth && m_uiHalfHeight == uiHalfHeight && !m_hOcclusionTexture.IsInvalidated())
    return;

  DestroyResources();

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  // Occlusion mask (single-channel float)
  ezGALTextureCreationDescription occDesc;
  occDesc.m_uiWidth = uiHalfWidth;
  occDesc.m_uiHeight = uiHalfHeight;
  occDesc.m_Format = ezGALResourceFormat::RFloat;
  occDesc.m_TextureFlags = ezGALTextureUsageFlags::ShaderResource | ezGALTextureUsageFlags::UnorderedAccess;
  occDesc.m_ResourceAccess.m_bImmutable = false;

  m_hOcclusionTexture = pDevice->CreateTexture(occDesc);

  // God rays result (RGBA half-float for colored rays)
  ezGALTextureCreationDescription raysDesc;
  raysDesc.m_uiWidth = uiHalfWidth;
  raysDesc.m_uiHeight = uiHalfHeight;
  raysDesc.m_Format = ezGALResourceFormat::RGBAHalf;
  raysDesc.m_TextureFlags = ezGALTextureUsageFlags::ShaderResource | ezGALTextureUsageFlags::UnorderedAccess;
  raysDesc.m_ResourceAccess.m_bImmutable = false;

  m_hGodRaysTexture = pDevice->CreateTexture(raysDesc);

  m_uiHalfWidth = uiHalfWidth;
  m_uiHalfHeight = uiHalfHeight;
}

void ezGodRaysPass::DestroyResources()
{
  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  if (!m_hOcclusionTexture.IsInvalidated())
    pDevice->DestroyTexture(m_hOcclusionTexture);
  if (!m_hGodRaysTexture.IsInvalidated())
    pDevice->DestroyTexture(m_hGodRaysTexture);

  m_hOcclusionTexture.Invalidate();
  m_hGodRaysTexture.Invalidate();
  m_uiHalfWidth = 0;
  m_uiHalfHeight = 0;
}

ezResult ezGodRaysPass::Serialize(ezStreamWriter& inout_stream) const
{
  EZ_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << m_uiNumSamples;
  inout_stream << m_fDensity;
  inout_stream << m_fWeight;
  inout_stream << m_fDecay;
  inout_stream << m_fExposure;
  inout_stream << m_fIntensity;
  inout_stream << m_fMaxRayLength;
  inout_stream << m_fDepthThreshold;
  return EZ_SUCCESS;
}

ezResult ezGodRaysPass::Deserialize(ezStreamReader& inout_stream)
{
  EZ_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  inout_stream >> m_uiNumSamples;
  inout_stream >> m_fDensity;
  inout_stream >> m_fWeight;
  inout_stream >> m_fDecay;
  inout_stream >> m_fExposure;
  inout_stream >> m_fIntensity;
  inout_stream >> m_fMaxRayLength;
  inout_stream >> m_fDepthThreshold;
  return EZ_SUCCESS;
}

void ezGodRaysPass::SetNumSamples(ezUInt32 uiSamples) { m_uiNumSamples = ezMath::Clamp(uiSamples, 16u, 256u); }
ezUInt32 ezGodRaysPass::GetNumSamples() const { return m_uiNumSamples; }

void ezGodRaysPass::SetIntensity(float fIntensity) { m_fIntensity = ezMath::Max(fIntensity, 0.0f); }
float ezGodRaysPass::GetIntensity() const { return m_fIntensity; }

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_GodRaysPass);
