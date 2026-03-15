#pragma once

#include <Foundation/Threading/Mutex.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/FrameDataProvider.h>
#include <RendererFoundation/RendererFoundationDLL.h>

#include <RendererCore/../../../Data/Base/Shaders/Particles/GPUParticleData.h>

struct EZ_RENDERERCORE_DLL ezGPUParticleSystemInfo
{
  ezGALBufferHandle m_hParticleBuffer;
  ezGALBufferHandle m_hCounterBuffer;
  ezUInt32 m_uiMaxParticles = 0;

  ezArrayPtr<ezGPUParticle> m_NewParticles;
  ezUInt32 m_uiEmitStartIndex = 0;

  float m_fGravity = 9.81f;
  float m_fDragCoefficient = 0.0f;
  float m_fWindStrength = 0.0f;
  ezVec3 m_vWindDirection = ezVec3::MakeZero();
  bool m_bEnableDepthCollision = false;
  bool m_bEnableSDFCollision = false;
  ezUInt8 m_uiCollisionReaction = 0;
  float m_fCollisionBounceFactor = 0.5f;
  float m_fCollisionSlideFactor = 0.5f;
  float m_fCollisionThickness = 0.5f;
  ezColor m_ColorStart = ezColor::White;
  ezColor m_ColorEnd = ezColor(1, 1, 1, 0);

  ezUInt8 m_uiGPURenderType = 0;
  ezUInt32 m_uiMaxTrailPoints = 16;
  ezUInt32 m_uiTrailWriteIndex = 0;
  ezGALBufferHandle m_hTrailPositionBuffer;
  float m_fVelocityStretch = 1.0f;
};

struct EZ_RENDERERCORE_DLL ezGPUParticleData
{
  ezDynamicArray<ezGPUParticleSystemInfo> m_Systems;
};

/// Bridges GPU particle system data from extraction (ParticlePlugin) to simulation (RendererCore).
///
/// During render data extraction, particle types call QueueSystem() to register their GPU buffers.
/// When the compute pass queries this provider, UpdateData() copies the queued systems into the
/// per-frame data that the pass iterates. The queue persists for the entire frame so that
/// multiple views/pipelines all see the same data.
class EZ_RENDERERCORE_DLL ezGPUParticleDataProvider : public ezFrameDataProvider<ezGPUParticleData>
{
  EZ_ADD_DYNAMIC_REFLECTION(ezGPUParticleDataProvider, ezFrameDataProviderBase);

public:
  ezGPUParticleDataProvider();
  ~ezGPUParticleDataProvider();

  /// Called from ParticleTypeGPU::ExtractTypeRenderData() to register a GPU particle system.
  /// Thread-safe. Queued data is consumed by UpdateData on the next provider query.
  static void QueueSystem(const ezGPUParticleSystemInfo& info);

private:
  virtual void* UpdateData(const ezRenderViewContext& renderViewContext, const ezExtractedRenderData& extractedData) override;

  ezGPUParticleData m_Data;

  static ezMutex s_QueueMutex;
  static ezDynamicArray<ezGPUParticleSystemInfo> s_PendingSystems;
  static ezUInt64 s_uiLastQueueFrame;
};
