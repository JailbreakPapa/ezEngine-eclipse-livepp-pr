#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/GPUParticleDataProvider.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezGPUParticleDataProvider, 1, ezRTTIDefaultAllocator<ezGPUParticleDataProvider>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezMutex ezGPUParticleDataProvider::s_QueueMutex;
ezDynamicArray<ezGPUParticleSystemInfo> ezGPUParticleDataProvider::s_PendingSystems;

ezGPUParticleDataProvider::ezGPUParticleDataProvider() = default;
ezGPUParticleDataProvider::~ezGPUParticleDataProvider() = default;

void ezGPUParticleDataProvider::QueueSystem(const ezGPUParticleSystemInfo& info)
{
  EZ_LOCK(s_QueueMutex);
  s_PendingSystems.PushBack(info);
}

void* ezGPUParticleDataProvider::UpdateData(const ezRenderViewContext& renderViewContext, const ezExtractedRenderData& extractedData)
{
  m_Data.m_Systems.Clear();

  {
    EZ_LOCK(s_QueueMutex);
    m_Data.m_Systems = std::move(s_PendingSystems);
    s_PendingSystems.Clear();
  }

  return &m_Data;
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_GPUParticleDataProvider);
