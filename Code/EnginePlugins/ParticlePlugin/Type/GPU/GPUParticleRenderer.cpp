#include <ParticlePlugin/ParticlePluginPCH.h>

#include <ParticlePlugin/Type/GPU/GPUParticleRenderer.h>
#include <RendererCore/Pipeline/RenderDataBatch.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Device/Device.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezGPUParticleRenderData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezGPUParticleRenderer, 1, ezRTTIDefaultAllocator<ezGPUParticleRenderer>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

bool ezGPUParticleRenderData::CanBatch(const ezRenderData& other0) const
{
  const auto& other = ezStaticCast<const ezGPUParticleRenderData&>(other0);
  return m_RenderMode == other.m_RenderMode && m_hTexture == other.m_hTexture && m_uiGPURenderType == other.m_uiGPURenderType;
}

ezGPUParticleRenderer::ezGPUParticleRenderer()
{
  m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Particles/GPUParticleRender.ezShader");
}

ezGPUParticleRenderer::~ezGPUParticleRenderer() = default;

void ezGPUParticleRenderer::GetSupportedRenderDataTypes(ezDynamicArray<const ezRTTI*>& out_types) const
{
  out_types.PushBack(ezGetStaticRTTI<ezGPUParticleRenderData>());
}

void ezGPUParticleRenderer::RenderBatch(const ezRenderViewContext& renderViewContext, const ezRenderPipelinePass* pPass, const ezRenderDataBatch& batch) const
{
  ezRenderContext* pRenderContext = renderViewContext.m_pRenderContext;
  ezGALCommandEncoder* pGALCommandEncoder = pRenderContext->GetCommandEncoder();

  TempSystemCB systemConstants(pRenderContext);

  pRenderContext->BindShader(m_hShader);

  for (auto it = batch.GetIterator<ezGPUParticleRenderData>(0, batch.GetDataCount()); it.IsValid(); ++it)
  {
    const ezGPUParticleRenderData* pRenderData = it;

    if (!pRenderData->m_hParticleBuffer.IsInvalidated())
    {
      systemConstants.SetGenericData(pRenderData->m_GlobalTransform, pRenderData->m_TotalEffectLifeTime, 1, 1, 1, 1, 0, 0);

      ConfigureRenderMode(pRenderData, pRenderContext);

      // Set GPU_PARTICLE_TYPE permutation
      switch (pRenderData->m_uiGPURenderType)
      {
        case ezGPUParticleRenderType::Point:
          pRenderContext->SetShaderPermutationVariable("GPU_PARTICLE_TYPE", "GPU_PARTICLE_TYPE_POINT");
          break;
        case ezGPUParticleRenderType::VelocityAligned:
          pRenderContext->SetShaderPermutationVariable("GPU_PARTICLE_TYPE", "GPU_PARTICLE_TYPE_VELOCITY_ALIGNED");
          break;
        case ezGPUParticleRenderType::Trail:
          pRenderContext->SetShaderPermutationVariable("GPU_PARTICLE_TYPE", "GPU_PARTICLE_TYPE_TRAIL");
          break;
        default: // Billboard
          pRenderContext->SetShaderPermutationVariable("GPU_PARTICLE_TYPE", "GPU_PARTICLE_TYPE_BILLBOARD");
          break;
      }

      // Determine topology and primitive count based on render type
      ezGALPrimitiveTopology::Enum topology = ezGALPrimitiveTopology::Triangles;
      ezUInt32 uiPrimitiveCount = pRenderData->m_uiMaxParticles * 2;

      switch (pRenderData->m_uiGPURenderType)
      {
        case ezGPUParticleRenderType::Point:
          topology = ezGALPrimitiveTopology::Points;
          uiPrimitiveCount = pRenderData->m_uiMaxParticles;
          break;
        case ezGPUParticleRenderType::Trail:
          uiPrimitiveCount = pRenderData->m_uiMaxParticles * (pRenderData->m_uiMaxTrailPoints - 1) * 2;
          break;
        default:
          break;
      }

      pRenderContext->BindNullMeshBuffer(topology, uiPrimitiveCount);

      ezBindGroupBuilder& bindGroup = pRenderContext->GetBindGroup(EZ_GAL_BIND_GROUP_DRAW_CALL);
      bindGroup.BindBuffer("gpuParticlesRead", pRenderData->m_hParticleBuffer);
      bindGroup.BindTexture("ParticleTexture", pRenderData->m_hTexture);

      if (pRenderData->m_uiGPURenderType == ezGPUParticleRenderType::Trail && !pRenderData->m_hTrailPositionBuffer.IsInvalidated())
      {
        bindGroup.BindBuffer("gpuTrailPositionsRead", pRenderData->m_hTrailPositionBuffer);
      }

      pRenderContext->DrawMeshBuffer(uiPrimitiveCount).IgnoreResult();
    }
  }
}

void ezGPUParticleRenderer::ConfigureRenderMode(const ezGPUParticleRenderData* pRenderData, ezRenderContext* pRenderContext) const
{
  if (pRenderData->m_RenderMode == ezParticleTypeRenderMode::Additive)
  {
    pRenderContext->SetShaderPermutationVariable("PARTICLE_RENDER_MODE", "PARTICLE_RENDER_MODE_ADDITIVE");
  }
  else if (pRenderData->m_RenderMode == ezParticleTypeRenderMode::Opaque)
  {
    pRenderContext->SetShaderPermutationVariable("PARTICLE_RENDER_MODE", "PARTICLE_RENDER_MODE_OPAQUE");
  }
  else
  {
    pRenderContext->SetShaderPermutationVariable("PARTICLE_RENDER_MODE", "PARTICLE_RENDER_MODE_BLENDED");
  }
}


EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Type_GPU_GPUParticleRenderer);
