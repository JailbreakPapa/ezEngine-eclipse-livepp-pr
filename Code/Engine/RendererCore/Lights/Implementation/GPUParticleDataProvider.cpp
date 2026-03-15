#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/GPUParticleDataProvider.h>
#include <RendererCore/RenderWorld/RenderWorld.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezGPUParticleDataProvider, 1, ezRTTIDefaultAllocator<ezGPUParticleDataProvider>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezMutex ezGPUParticleDataProvider::s_QueueMutex;
ezDynamicArray<ezGPUParticleSystemInfo> ezGPUParticleDataProvider::s_PendingSystems;
ezUInt64 ezGPUParticleDataProvider::s_uiLastQueueFrame = ezUInt64(-1);

ezGPUParticleDataProvider::ezGPUParticleDataProvider() = default;
ezGPUParticleDataProvider::~ezGPUParticleDataProvider() = default;

void ezGPUParticleDataProvider::QueueSystem(const ezGPUParticleSystemInfo& info)
{
  EZ_LOCK(s_QueueMutex);

  const ezUInt64 uiFrame = ezRenderWorld::GetFrameCounter();
  if (s_uiLastQueueFrame != uiFrame)
  {
    s_PendingSystems.Clear();
    s_uiLastQueueFrame = uiFrame;
  }

  s_PendingSystems.PushBack(info);
}

void* ezGPUParticleDataProvider::UpdateData(const ezRenderViewContext& renderViewContext, const ezExtractedRenderData& extractedData)
{
  EZ_LOCK(s_QueueMutex);

  // Only serve data from the current frame. If no extraction happened this frame
  // (e.g. camera out of bounds), return an empty list to avoid accessing stale
  // frame-allocator pointers from a previous frame.
  const ezUInt64 uiCurrentFrame = ezRenderWorld::GetFrameCounter();
  if (s_uiLastQueueFrame == uiCurrentFrame)
  {
    m_Data.m_Systems = s_PendingSystems;
  }
  else
  {
    m_Data.m_Systems.Clear();
  }

  return &m_Data;
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_GPUParticleDataProvider);
