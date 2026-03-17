#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Lights/ClusteredDataProvider.h>
#include <RendererCore/Pipeline/Passes/VolumetricFogPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/Texture.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezVolumetricFogPass, 1, ezRTTIDefaultAllocator<ezVolumetricFogPass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("DepthInput", m_PinDepthInput),
    EZ_MEMBER_PROPERTY("Color", m_PinColor),
    EZ_ACCESSOR_PROPERTY("FroxelGridSizeX", GetFroxelGridSizeX, SetFroxelGridSizeX)->AddAttributes(new ezDefaultValueAttribute(160), new ezClampValueAttribute(16, 512)),
    EZ_ACCESSOR_PROPERTY("FroxelGridSizeY", GetFroxelGridSizeY, SetFroxelGridSizeY)->AddAttributes(new ezDefaultValueAttribute(90), new ezClampValueAttribute(16, 512)),
    EZ_ACCESSOR_PROPERTY("FroxelGridSizeZ", GetFroxelGridSizeZ, SetFroxelGridSizeZ)->AddAttributes(new ezDefaultValueAttribute(64), new ezClampValueAttribute(16, 256)),
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

ezVolumetricFogPass::ezVolumetricFogPass()
  : ezRenderPipelinePass("VolumetricFogPass", true)
{
  m_hShaderInject = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/VolumetricFogInject.ezShader");
  m_hShaderTemporal = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/VolumetricFogTemporal.ezShader");
  m_hShaderIntegrate = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/VolumetricFogIntegrate.ezShader");
  m_hShaderApply = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Pipeline/VolumetricFogApply.ezShader");

  m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezVolumetricFogConstants>();
}

ezVolumetricFogPass::~ezVolumetricFogPass()
{
  DestroyFroxelTextures();
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
}

bool ezVolumetricFogPass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
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

void ezVolumetricFogPass::InitRenderPipelinePass(const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  EnsureFroxelTextures();
}

void ezVolumetricFogPass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  auto pColorOutput = outputs[m_PinColor.m_uiOutputIndex];
  if (pColorOutput == nullptr)
    return;

  if (m_bFroxelTexturesDirty)
  {
    EnsureFroxelTextures();
    m_bFirstFrame = true;
  }

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  // Get clustered data for light information
  auto pClusteredData = GetPipeline()->GetFrameDataProvider<ezClusteredDataProvider>()->GetData(renderViewContext);

  // Early out: no fog volumes in the scene
  if (pClusteredData->m_uiNumFogVolumes == 0)
  {
    m_PrevViewProjectionMatrix = renderViewContext.m_pViewData->m_ViewProjectionMatrix[0];
    m_bFirstFrame = true;
    return;
  }

  // Determine current / history froxel grids
  ezGALTextureHandle hCurrentGrid = m_bUseGridA ? m_hFroxelGridA : m_hFroxelGridB;
  ezGALTextureHandle hHistoryGrid = m_bUseGridA ? m_hFroxelGridB : m_hFroxelGridA;

  // Update constant buffer - use parameters from the first fog volume, or defaults
  {
    ezVolumetricFogConstants* cb = ezRenderContext::GetConstantBufferData<ezVolumetricFogConstants>(m_hConstantBuffer);
    cb->FroxelGridSizeX = m_uiFroxelGridSizeX;
    cb->FroxelGridSizeY = m_uiFroxelGridSizeY;
    cb->FroxelGridSizeZ = m_uiFroxelGridSizeZ;

    cb->FroxelNearPlane = pClusteredData->m_fFogNearPlane;
    cb->FroxelFarPlane = pClusteredData->m_fFogFarPlane;
    cb->FroxelDepthSliceScale = 0.0f;
    cb->FroxelDepthSliceBias = 0.0f;
    cb->FogDensityScale = 0.0f;
    cb->FogAlbedo = ezVec3(1.0f);
    cb->FogAnisotropy = 0.0f;

    // On first frame, use full weight to avoid blending with uninitialized history
    cb->TemporalBlendWeight = m_bFirstFrame ? 1.0f : pClusteredData->m_fFogTemporalBlendWeight;

    cb->FogHeightDensityFalloff = 0.0f;
    cb->FogBaseHeight = 0.0f;
    cb->VFogStartDistance = 0.0f;
    cb->AmbientLight = ezVec3(0.0f);

    // Store previous frame's view-projection matrix for temporal reprojection
    cb->PrevWorldToClipMatrix = m_PrevViewProjectionMatrix;
  }

  // Compute dispatch sizes
  ezUInt32 dispatchX = (m_uiFroxelGridSizeX + VOLUMETRIC_FOG_THREAD_GROUP_X - 1) / VOLUMETRIC_FOG_THREAD_GROUP_X;
  ezUInt32 dispatchY = (m_uiFroxelGridSizeY + VOLUMETRIC_FOG_THREAD_GROUP_Y - 1) / VOLUMETRIC_FOG_THREAD_GROUP_Y;

  // Pass 1: Inject light scattering into froxels (compute)
  {
    EZ_PROFILE_SCOPE("Volumetric Fog Inject");
    auto computeScope = renderViewContext.m_pRenderContext->BeginComputeScope(renderViewContext, "VFog Inject");

    ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
    bindGroup.BindBuffer("ezVolumetricFogConstants", m_hConstantBuffer);
    bindGroup.BindTexture("FroxelGrid", hCurrentGrid);

    // Bind clustered light data
    pClusteredData->BindResources(renderViewContext.m_pRenderContext);

    renderViewContext.m_pRenderContext->BindShader(m_hShaderInject);
    renderViewContext.m_pRenderContext->Dispatch(dispatchX, dispatchY, 1).IgnoreResult();
  }

  // Pass 2: Temporal reprojection (compute)
  {
    EZ_PROFILE_SCOPE("Volumetric Fog Temporal");
    auto computeScope = renderViewContext.m_pRenderContext->BeginComputeScope(renderViewContext, "VFog Temporal");

    ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
    bindGroup.BindBuffer("ezVolumetricFogConstants", m_hConstantBuffer);
    bindGroup.BindTexture("FroxelGrid", hCurrentGrid);
    bindGroup.BindTexture("HistoryFroxelGrid", hHistoryGrid);

    renderViewContext.m_pRenderContext->BindShader(m_hShaderTemporal);
    renderViewContext.m_pRenderContext->Dispatch(dispatchX, dispatchY, 1).IgnoreResult();
  }

  // Pass 3: Front-to-back integration (compute)
  {
    EZ_PROFILE_SCOPE("Volumetric Fog Integrate");
    auto computeScope = renderViewContext.m_pRenderContext->BeginComputeScope(renderViewContext, "VFog Integrate");

    ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
    bindGroup.BindBuffer("ezVolumetricFogConstants", m_hConstantBuffer);
    bindGroup.BindTexture("FroxelGridInput", hCurrentGrid);
    bindGroup.BindTexture("IntegratedFroxelGrid", m_hIntegratedFroxelGrid);

    renderViewContext.m_pRenderContext->BindShader(m_hShaderIntegrate);
    renderViewContext.m_pRenderContext->Dispatch(dispatchX, dispatchY, 1).IgnoreResult();
  }

  // Pass 4: Apply volumetric fog to scene
  // Copy the passthrough color to a temp texture so we can read it as SceneColor,
  // then render the composite result back into the passthrough target.
  {
    EZ_PROFILE_SCOPE("Volumetric Fog Apply");

    ezGALTextureHandle hColorTarget = pColorOutput->m_TextureHandle;

    // Allocate temp copy of the scene color
    ezGALTextureCreationDescription tempDesc = pColorOutput->m_Desc;
    tempDesc.m_TextureFlags.Add(ezGALTextureUsageFlags::ShaderResource | ezGALTextureUsageFlags::RenderTarget);
    ezGALTextureHandle hTempSceneColor = ezGPUResourcePool::GetDefaultInstance()->GetRenderTarget(tempDesc);

    // Copy current color to temp
    renderViewContext.m_pRenderContext->GetCommandEncoder()->CopyTexture(hTempSceneColor, hColorTarget);

    // Render fog composite into the passthrough color target
    ezGALRenderingSetup renderingSetup;
    renderingSetup.SetColorTarget(0, pDevice->GetDefaultRenderTargetView(hColorTarget));

    auto pCommandEncoder = renderViewContext.m_pRenderContext->BeginRenderingScope(renderViewContext, renderingSetup, "VFog Apply", renderViewContext.m_pCamera->IsStereoscopic());

    ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
    bindGroup.BindBuffer("ezVolumetricFogConstants", m_hConstantBuffer);
    bindGroup.BindTexture("VolumetricFogTexture", m_hIntegratedFroxelGrid);
    bindGroup.BindTexture("SceneDepth", inputs[m_PinDepthInput.m_uiInputIndex]->m_TextureHandle);
    bindGroup.BindTexture("SceneColor", hTempSceneColor);

    renderViewContext.m_pRenderContext->BindShader(m_hShaderApply);
    renderViewContext.m_pRenderContext->BindNullMeshBuffer(ezGALPrimitiveTopology::Triangles, 1);
    renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();

    ezGPUResourcePool::GetDefaultInstance()->ReturnRenderTarget(hTempSceneColor);
  }

  // Store current view-projection for next frame's temporal reprojection
  m_PrevViewProjectionMatrix = renderViewContext.m_pViewData->m_ViewProjectionMatrix[0];

  // Swap ping-pong
  m_bUseGridA = !m_bUseGridA;
  m_bFirstFrame = false;
}

void ezVolumetricFogPass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  // PassThrough: nothing to do, the color buffer is already passed through
}

void ezVolumetricFogPass::EnsureFroxelTextures()
{
  DestroyFroxelTextures();

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  ezGALTextureCreationDescription desc;
  desc.m_uiWidth = m_uiFroxelGridSizeX;
  desc.m_uiHeight = m_uiFroxelGridSizeY;
  desc.m_uiDepth = m_uiFroxelGridSizeZ;
  desc.m_Type = ezGALTextureType::Texture3D;
  desc.m_Format = ezGALResourceFormat::RGBAHalf;
  desc.m_TextureFlags = ezGALTextureUsageFlags::ShaderResource | ezGALTextureUsageFlags::UnorderedAccess;
  desc.m_ResourceAccess.m_bImmutable = false;

  m_hFroxelGridA = pDevice->CreateTexture(desc);
  m_hFroxelGridB = pDevice->CreateTexture(desc);
  m_hIntegratedFroxelGrid = pDevice->CreateTexture(desc);

  m_bFroxelTexturesDirty = false;
}

void ezVolumetricFogPass::DestroyFroxelTextures()
{
  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  if (!m_hFroxelGridA.IsInvalidated())
    pDevice->DestroyTexture(m_hFroxelGridA);
  if (!m_hFroxelGridB.IsInvalidated())
    pDevice->DestroyTexture(m_hFroxelGridB);
  if (!m_hIntegratedFroxelGrid.IsInvalidated())
    pDevice->DestroyTexture(m_hIntegratedFroxelGrid);

  m_hFroxelGridA.Invalidate();
  m_hFroxelGridB.Invalidate();
  m_hIntegratedFroxelGrid.Invalidate();
}

ezResult ezVolumetricFogPass::Serialize(ezStreamWriter& inout_stream) const
{
  EZ_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << m_uiFroxelGridSizeX;
  inout_stream << m_uiFroxelGridSizeY;
  inout_stream << m_uiFroxelGridSizeZ;
  return EZ_SUCCESS;
}

ezResult ezVolumetricFogPass::Deserialize(ezStreamReader& inout_stream)
{
  EZ_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const ezUInt32 uiVersion = ezTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  EZ_IGNORE_UNUSED(uiVersion);
  inout_stream >> m_uiFroxelGridSizeX;
  inout_stream >> m_uiFroxelGridSizeY;
  inout_stream >> m_uiFroxelGridSizeZ;
  return EZ_SUCCESS;
}

void ezVolumetricFogPass::SetFroxelGridSizeX(ezUInt32 uiSize)
{
  m_uiFroxelGridSizeX = ezMath::Clamp(uiSize, 16u, 512u);
  m_bFroxelTexturesDirty = true;
}

ezUInt32 ezVolumetricFogPass::GetFroxelGridSizeX() const { return m_uiFroxelGridSizeX; }

void ezVolumetricFogPass::SetFroxelGridSizeY(ezUInt32 uiSize)
{
  m_uiFroxelGridSizeY = ezMath::Clamp(uiSize, 16u, 512u);
  m_bFroxelTexturesDirty = true;
}

ezUInt32 ezVolumetricFogPass::GetFroxelGridSizeY() const { return m_uiFroxelGridSizeY; }

void ezVolumetricFogPass::SetFroxelGridSizeZ(ezUInt32 uiSize)
{
  m_uiFroxelGridSizeZ = ezMath::Clamp(uiSize, 16u, 256u);
  m_bFroxelTexturesDirty = true;
}

ezUInt32 ezVolumetricFogPass::GetFroxelGridSizeZ() const { return m_uiFroxelGridSizeZ; }

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_VolumetricFogPass);
