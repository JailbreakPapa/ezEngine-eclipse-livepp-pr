#pragma once

#include <ParticlePlugin/Declarations.h>
#include <ParticlePlugin/ParticlePluginDLL.h>
#include <ParticlePlugin/Renderer/ParticleRenderer.h>
#include <RendererFoundation/RendererFoundationDLL.h>

#include <RendererCore/../../../Data/Base/Shaders/Particles/GPUParticleData.h>

class EZ_PARTICLEPLUGIN_DLL ezGPUParticleRenderData final : public ezRenderData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezGPUParticleRenderData, ezRenderData);

public:
  virtual bool CanBatch(const ezRenderData& other) const override;

  ezTexture2DResourceHandle m_hTexture;
  ezTransform m_GlobalTransform;
  ezTime m_TotalEffectLifeTime;
  ezEnum<ezParticleTypeRenderMode> m_RenderMode;
  ezUInt32 m_uiMaxParticles = 0;

  // GPU buffer handles
  ezGALBufferHandle m_hParticleBuffer;
  ezGALBufferHandle m_hCounterBuffer;

  // Emission data
  ezArrayPtr<ezGPUParticle> m_NewParticles;
  ezUInt32 m_uiEmitStartIndex = 0;

  // Simulation parameters
  float m_fGravity = 9.81f;
  float m_fDragCoefficient = 0.0f;
  float m_fWindStrength = 0.0f;
  bool m_bEnableDepthCollision = false;
  bool m_bEnableSDFCollision = false;
  ezUInt8 m_uiCollisionReaction = 0;
  float m_fCollisionBounceFactor = 0.5f;
  float m_fCollisionSlideFactor = 0.5f;
  float m_fCollisionThickness = 0.5f;
  ezColor m_ColorStart = ezColor::White;
  ezColor m_ColorEnd = ezColor(1, 1, 1, 0);

  // Multi-type rendering
  ezUInt8 m_uiGPURenderType = 0;
  ezUInt32 m_uiMaxTrailPoints = 16;
  ezGALBufferHandle m_hTrailPositionBuffer;
  float m_fVelocityStretch = 1.0f;
};

class EZ_PARTICLEPLUGIN_DLL ezGPUParticleRenderer final : public ezParticleRenderer
{
  EZ_ADD_DYNAMIC_REFLECTION(ezGPUParticleRenderer, ezParticleRenderer);
  EZ_DISALLOW_COPY_AND_ASSIGN(ezGPUParticleRenderer);

public:
  ezGPUParticleRenderer();
  ~ezGPUParticleRenderer();

  virtual void GetSupportedRenderDataTypes(ezDynamicArray<const ezRTTI*>& out_types) const override;
  virtual void RenderBatch(const ezRenderViewContext& renderViewContext, const ezRenderPipelinePass* pPass, const ezRenderDataBatch& batch) const override;

protected:
  void ConfigureRenderMode(const ezGPUParticleRenderData* pRenderData, ezRenderContext* pRenderContext) const;
};
