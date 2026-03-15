#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Time/Clock.h>
#include <RendererCore/Lights/GPUParticleDataProvider.h>
#include <RendererCore/Pipeline/Passes/GPUParticlePass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Resources/Buffer.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezGPUParticlePass, 1, ezRTTIDefaultAllocator<ezGPUParticlePass>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("DepthInput", m_PinDepthInput),
    EZ_MEMBER_PROPERTY("Color", m_PinColor),
    EZ_MEMBER_PROPERTY("SDFResolution", m_uiSDFResolution)->AddAttributes(new ezDefaultValueAttribute(64), new ezClampValueAttribute(16, 128)),
    EZ_MEMBER_PROPERTY("SDFWorldExtent", m_fSDFWorldExtent)->AddAttributes(new ezDefaultValueAttribute(50.0f), new ezClampValueAttribute(5.0f, 500.0f)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezGPUParticlePass::ezGPUParticlePass()
  : ezRenderPipelinePass("GPUParticlePass", true)
{
  m_hEmitShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Particles/GPUParticleEmit.ezShader");
  m_hSimulateShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Particles/GPUParticleSimulate.ezShader");
  m_hSDFShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Particles/GPUParticleSDF.ezShader");

  m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezGPUParticleConstants>();
}

ezGPUParticlePass::~ezGPUParticlePass()
{
  DestroySDFTexture();
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
}

bool ezGPUParticlePass::GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs)
{
  if (inputs[m_PinColor.m_uiInputIndex])
  {
    outputs[m_PinColor.m_uiOutputIndex] = *inputs[m_PinColor.m_uiInputIndex];
  }
  else
  {
    ezLog::Error("No color input connected to pass '{0}'!", GetName());
    return false;
  }

  return true;
}

void ezGPUParticlePass::InitRenderPipelinePass(const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
}

void ezGPUParticlePass::Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
  auto pDepthInput = inputs[m_PinDepthInput.m_uiInputIndex];
  if (pDepthInput == nullptr)
    return;

  // Get all GPU particle systems registered this frame
  auto pDataProvider = GetPipeline()->GetFrameDataProvider<ezGPUParticleDataProvider>();
  if (pDataProvider == nullptr)
    return;

  const ezGPUParticleData* pData = pDataProvider->GetData(renderViewContext);
  if (pData == nullptr || pData->m_Systems.IsEmpty())
    return;

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
  const auto* pCamera = renderViewContext.m_pCamera;

  // Check if any system needs SDF
  bool bAnySDF = false;
  for (const auto& sys : pData->m_Systems)
  {
    if (sys.m_bEnableSDFCollision)
    {
      bAnySDF = true;
      break;
    }
  }

  // Get depth buffer dimensions
  const auto* pDepthTex = pDevice->GetTexture(pDepthInput->m_TextureHandle);
  float fDepthWidth = 1920.0f;
  float fDepthHeight = 1080.0f;
  if (pDepthTex != nullptr)
  {
    fDepthWidth = (float)pDepthTex->GetDescription().m_uiWidth;
    fDepthHeight = (float)pDepthTex->GetDescription().m_uiHeight;
  }

  // Build view-projection matrix
  ezMat4 viewMat = renderViewContext.m_pCamera->GetViewMatrix(ezCameraEye::Left);
  ezMat4 projMat;
  renderViewContext.m_pCamera->GetProjectionMatrix((float)fDepthWidth / fDepthHeight, projMat, ezCameraEye::Left, ezClipSpaceDepthRange::ZeroToOne);
  ezMat4 vpMat = projMat * viewMat;
  ezMat4 invVpMat = vpMat.GetInverse();

  // ---- SDF Generation (once per frame) ----
  if (bAnySDF)
  {
    EnsureSDFTexture();

    EZ_PROFILE_SCOPE("GPU Particle SDF");
    auto computeScope = renderViewContext.m_pRenderContext->BeginComputeScope(renderViewContext, "GPU Particle SDF");

    ezVec3 camPos = pCamera->GetCenterPosition();
    ezVec3 sdfMin = camPos - ezVec3(m_fSDFWorldExtent);
    ezVec3 sdfMax = camPos + ezVec3(m_fSDFWorldExtent);
    float voxelSize = (m_fSDFWorldExtent * 2.0f) / (float)m_uiSDFResolution;

    {
      auto* cb = ezRenderContext::GetConstantBufferData<ezGPUParticleConstants>(m_hConstantBuffer);
      cb->GPUPartSDFWorldMin.Set(sdfMin.x, sdfMin.y, sdfMin.z);
      cb->GPUPartSDFWorldMax.Set(sdfMax.x, sdfMax.y, sdfMax.z);
      cb->GPUPartSDFVoxelSize = voxelSize;
      cb->GPUPartSDFResolution = m_uiSDFResolution;
      cb->GPUPartDepthBufferWidth = fDepthWidth;
      cb->GPUPartDepthBufferHeight = fDepthHeight;
      cb->GPUPartViewProjectionMatrix = vpMat;
      cb->GPUPartInverseViewProjectionMatrix = invVpMat;
    }

    ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
    bindGroup.BindBuffer("ezGPUParticleConstants", m_hConstantBuffer);
    bindGroup.BindTexture("SceneDepth", pDepthInput->m_TextureHandle);
    bindGroup.BindTexture("SDFTexture", m_hSDFTexture);

    renderViewContext.m_pRenderContext->BindShader(m_hSDFShader);

    ezUInt32 groups = (m_uiSDFResolution + GPU_PARTICLE_SDF_THREAD_GROUP_SIZE - 1) / GPU_PARTICLE_SDF_THREAD_GROUP_SIZE;
    renderViewContext.m_pRenderContext->Dispatch(groups, groups, groups).IgnoreResult();
  }

  // ---- Per-system simulation ----
  for (const auto& sys : pData->m_Systems)
  {
    // Skip systems with invalid buffers
    if (sys.m_hParticleBuffer.IsInvalidated() || sys.m_hCounterBuffer.IsInvalidated() || sys.m_uiMaxParticles == 0)
      continue;

    // Helper to set GPU_PARTICLE_TYPE permutation using string literals
    auto SetGPUParticleType = [&]() {
      switch (sys.m_uiGPURenderType)
      {
        case 1: renderViewContext.m_pRenderContext->SetShaderPermutationVariable("GPU_PARTICLE_TYPE", "GPU_PARTICLE_TYPE_POINT"); break;
        case 2: renderViewContext.m_pRenderContext->SetShaderPermutationVariable("GPU_PARTICLE_TYPE", "GPU_PARTICLE_TYPE_VELOCITY_ALIGNED"); break;
        case 3: renderViewContext.m_pRenderContext->SetShaderPermutationVariable("GPU_PARTICLE_TYPE", "GPU_PARTICLE_TYPE_TRAIL"); break;
        default: renderViewContext.m_pRenderContext->SetShaderPermutationVariable("GPU_PARTICLE_TYPE", "GPU_PARTICLE_TYPE_BILLBOARD"); break;
      }
    };

    // ---- Emit new particles ----
    if (sys.m_NewParticles.GetCount() > 0 && sys.m_NewParticles.GetPtr() != nullptr)
    {
      // Upload new particle data to the write buffer, handling wrap-around
      const ezUInt32 uiEmitCount = sys.m_NewParticles.GetCount();
      const ezUInt32 uiEmitStart = sys.m_uiEmitStartIndex;
      const ezUInt32 uiMax = sys.m_uiMaxParticles;

      if (uiEmitStart + uiEmitCount <= uiMax)
      {
        // No wrap — single upload
        renderViewContext.m_pRenderContext->GetCommandEncoder()->UpdateBuffer(
          sys.m_hParticleBuffer,
          uiEmitStart * sizeof(ezGPUParticle),
          sys.m_NewParticles.ToByteArray(),
          ezGALUpdateMode::AheadOfTime);
      }
      else
      {
        // Wrap-around — split into two uploads
        const ezUInt32 uiFirstBatch = uiMax - uiEmitStart;
        const ezUInt32 uiSecondBatch = uiEmitCount - uiFirstBatch;

        if (uiFirstBatch > 0)
        {
          renderViewContext.m_pRenderContext->GetCommandEncoder()->UpdateBuffer(
            sys.m_hParticleBuffer,
            uiEmitStart * sizeof(ezGPUParticle),
            ezArrayPtr<const ezUInt8>(reinterpret_cast<const ezUInt8*>(sys.m_NewParticles.GetPtr()), uiFirstBatch * sizeof(ezGPUParticle)),
            ezGALUpdateMode::AheadOfTime);
        }

        if (uiSecondBatch > 0)
        {
          renderViewContext.m_pRenderContext->GetCommandEncoder()->UpdateBuffer(
            sys.m_hParticleBuffer,
            0,
            ezArrayPtr<const ezUInt8>(reinterpret_cast<const ezUInt8*>(sys.m_NewParticles.GetPtr() + uiFirstBatch), uiSecondBatch * sizeof(ezGPUParticle)),
            ezGALUpdateMode::AheadOfTime);
        }
      }

      {
        EZ_PROFILE_SCOPE("GPU Particle Emit");
        auto computeScope = renderViewContext.m_pRenderContext->BeginComputeScope(renderViewContext, "GPU Particle Emit");

        auto* cb = ezRenderContext::GetConstantBufferData<ezGPUParticleConstants>(m_hConstantBuffer);
        cb->GPUPartNumToEmit = sys.m_NewParticles.GetCount();
        cb->GPUPartEmitStartIndex = sys.m_uiEmitStartIndex;
        cb->GPUPartMaxParticles = sys.m_uiMaxParticles;
        cb->GPUPartMaxTrailPoints = sys.m_uiMaxTrailPoints;

        ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
        bindGroup.BindBuffer("ezGPUParticleConstants", m_hConstantBuffer);
        bindGroup.BindBuffer("gpuParticlesCurrent", sys.m_hParticleBuffer);
        bindGroup.BindBuffer("gpuParticleCounters", sys.m_hCounterBuffer);

        if (sys.m_uiGPURenderType == 3 && !sys.m_hTrailPositionBuffer.IsInvalidated()) // Trail
        {
          bindGroup.BindBuffer("gpuTrailPositions", sys.m_hTrailPositionBuffer);
        }

        SetGPUParticleType();
        renderViewContext.m_pRenderContext->BindShader(m_hEmitShader);

        ezUInt32 emitGroups = (sys.m_NewParticles.GetCount() + GPU_PARTICLE_EMIT_THREAD_GROUP_SIZE - 1) / GPU_PARTICLE_EMIT_THREAD_GROUP_SIZE;
        renderViewContext.m_pRenderContext->Dispatch(emitGroups, 1, 1).IgnoreResult();
      }
    }

    // ---- Simulate ----
    {
      EZ_PROFILE_SCOPE("GPU Particle Simulate");
      auto computeScope = renderViewContext.m_pRenderContext->BeginComputeScope(renderViewContext, "GPU Particle Simulate");

      {
        auto* cb = ezRenderContext::GetConstantBufferData<ezGPUParticleConstants>(m_hConstantBuffer);
        cb->GPUPartDeltaTime = (float)ezClock::GetGlobalClock()->GetTimeDiff().GetSeconds();
        cb->GPUPartMaxParticles = sys.m_uiMaxParticles;
        cb->GPUPartGravity = sys.m_fGravity;
        cb->GPUPartDragCoefficient = sys.m_fDragCoefficient;
        cb->GPUPartWindDirection = sys.m_vWindDirection;
        cb->GPUPartWindStrength = sys.m_fWindStrength;
        cb->GPUPartCollisionBounceFactor = sys.m_fCollisionBounceFactor;
        cb->GPUPartCollisionSlideFactor = sys.m_fCollisionSlideFactor;
        cb->GPUPartCollisionReaction = sys.m_uiCollisionReaction;
        cb->GPUPartCollisionThickness = sys.m_fCollisionThickness;
        cb->GPUPartEnableDepthCollision = sys.m_bEnableDepthCollision ? 1 : 0;
        cb->GPUPartEnableSDFCollision = sys.m_bEnableSDFCollision ? 1 : 0;
        cb->GPUPartDepthBufferWidth = fDepthWidth;
        cb->GPUPartDepthBufferHeight = fDepthHeight;
        cb->GPUPartViewProjectionMatrix = vpMat;
        cb->GPUPartInverseViewProjectionMatrix = invVpMat;
        cb->GPUPartColorOverLifeStart.Set(sys.m_ColorStart.r, sys.m_ColorStart.g, sys.m_ColorStart.b, sys.m_ColorStart.a);
        cb->GPUPartColorOverLifeEnd.Set(sys.m_ColorEnd.r, sys.m_ColorEnd.g, sys.m_ColorEnd.b, sys.m_ColorEnd.a);
        cb->GPUPartMaxTrailPoints = sys.m_uiMaxTrailPoints;
        cb->GPUPartTrailWriteIndex = sys.m_uiTrailWriteIndex;
        cb->GPUPartVelocityStretch = sys.m_fVelocityStretch;
      }

      ezBindGroupBuilder& bindGroup = renderViewContext.m_pRenderContext->GetBindGroup();
      bindGroup.BindBuffer("ezGPUParticleConstants", m_hConstantBuffer);
      bindGroup.BindBuffer("gpuParticlesCurrent", sys.m_hParticleBuffer);
      bindGroup.BindBuffer("gpuParticleCounters", sys.m_hCounterBuffer);
      bindGroup.BindTexture("SceneDepth", pDepthInput->m_TextureHandle);

      if (sys.m_bEnableSDFCollision && !m_hSDFTexture.IsInvalidated())
      {
        bindGroup.BindTexture("SDFTexture", m_hSDFTexture);
      }

      if (sys.m_uiGPURenderType == 3 && !sys.m_hTrailPositionBuffer.IsInvalidated()) // Trail
      {
        bindGroup.BindBuffer("gpuTrailPositions", sys.m_hTrailPositionBuffer);
      }

      SetGPUParticleType();
      renderViewContext.m_pRenderContext->BindShader(m_hSimulateShader);

      ezUInt32 simGroups = (sys.m_uiMaxParticles + GPU_PARTICLE_SIM_THREAD_GROUP_SIZE - 1) / GPU_PARTICLE_SIM_THREAD_GROUP_SIZE;
      renderViewContext.m_pRenderContext->Dispatch(simGroups, 1, 1).IgnoreResult();
    }
  }
}

void ezGPUParticlePass::ExecuteInactive(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs)
{
}

void ezGPUParticlePass::EnsureSDFTexture()
{
  if (!m_bSDFDirty)
    return;

  DestroySDFTexture();

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  ezGALTextureCreationDescription desc;
  desc.m_uiWidth = m_uiSDFResolution;
  desc.m_uiHeight = m_uiSDFResolution;
  desc.m_uiDepth = m_uiSDFResolution;
  desc.m_Type = ezGALTextureType::Texture3D;
  desc.m_Format = ezGALResourceFormat::RFloat;
  desc.m_TextureFlags = ezGALTextureUsageFlags::ShaderResource | ezGALTextureUsageFlags::UnorderedAccess;
  desc.m_ResourceAccess.m_bImmutable = false;

  m_hSDFTexture = pDevice->CreateTexture(desc);
  m_bSDFDirty = false;
}

void ezGPUParticlePass::DestroySDFTexture()
{
  if (!m_hSDFTexture.IsInvalidated())
  {
    ezGALDevice::GetDefaultDevice()->DestroyTexture(m_hSDFTexture);
    m_hSDFTexture.Invalidate();
  }
}

void ezGPUParticlePass::SetSDFResolution(ezUInt32 uiRes)
{
  if (m_uiSDFResolution != uiRes)
  {
    m_uiSDFResolution = uiRes;
    m_bSDFDirty = true;
  }
}

ezUInt32 ezGPUParticlePass::GetSDFResolution() const
{
  return m_uiSDFResolution;
}

void ezGPUParticlePass::SetSDFWorldExtent(float fExtent)
{
  m_fSDFWorldExtent = fExtent;
}

float ezGPUParticlePass::GetSDFWorldExtent() const
{
  return m_fSDFWorldExtent;
}

ezResult ezGPUParticlePass::Serialize(ezStreamWriter& inout_stream) const
{
  EZ_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << m_uiSDFResolution;
  inout_stream << m_fSDFWorldExtent;
  return EZ_SUCCESS;
}

ezResult ezGPUParticlePass::Deserialize(ezStreamReader& inout_stream)
{
  EZ_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  inout_stream >> m_uiSDFResolution;
  inout_stream >> m_fSDFWorldExtent;
  return EZ_SUCCESS;
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_GPUParticlePass);
