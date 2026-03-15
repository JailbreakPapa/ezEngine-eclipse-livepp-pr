#pragma once

#include <ParticlePlugin/Type/ParticleType.h>
#include <RendererFoundation/RendererFoundationDLL.h>

#include <RendererCore/../../../Data/Base/Shaders/Particles/GPUParticleData.h>

using ezTexture2DResourceHandle = ezTypedResourceHandle<class ezTexture2DResource>;

class EZ_PARTICLEPLUGIN_DLL ezParticleTypeGPUFactory final : public ezParticleTypeFactory
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleTypeGPUFactory, ezParticleTypeFactory);

public:
  virtual const ezRTTI* GetTypeType() const override;
  virtual void CopyTypeProperties(ezParticleType* pObject, bool bFirstTime) const override;

  virtual void Save(ezStreamWriter& inout_stream) const override;
  virtual void Load(ezStreamReader& inout_stream) override;

  ezUInt32 m_uiMaxParticles = 65536;
  ezEnum<ezParticleTypeRenderMode> m_RenderMode;
  ezString m_sTexture;

  float m_fGravity = 9.81f;
  float m_fDragCoefficient = 0.0f;
  float m_fWindStrength = 0.0f;

  bool m_bEnableDepthCollision = false;
  bool m_bEnableSDFCollision = false;
  ezEnum<ezParticleRaycastHitReaction> m_CollisionReaction;
  float m_fCollisionBounceFactor = 0.5f;
  float m_fCollisionSlideFactor = 0.5f;
  float m_fCollisionThickness = 0.5f;

  ezColor m_ColorStart = ezColor::White;
  ezColor m_ColorEnd = ezColor(1, 1, 1, 0);

  ezEnum<ezGPUParticleRenderType> m_GPURenderType;
  ezUInt32 m_uiMaxTrailPoints = 16;
  float m_fVelocityStretch = 1.0f;
};

class EZ_PARTICLEPLUGIN_DLL ezParticleTypeGPU final : public ezParticleType
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleTypeGPU, ezParticleType);

public:
  ezParticleTypeGPU();
  ~ezParticleTypeGPU();

  virtual void CreateRequiredStreams() override;
  virtual void ExtractTypeRenderData(ezMsgExtractRenderData& ref_msg, const ezTransform& instanceTransform) const override;

  ezUInt32 m_uiMaxGPUParticles = 65536;
  ezEnum<ezParticleTypeRenderMode> m_RenderMode;
  ezTexture2DResourceHandle m_hTexture;

  float m_fGravity = 9.81f;
  float m_fDragCoefficient = 0.0f;
  float m_fWindStrength = 0.0f;

  bool m_bEnableDepthCollision = false;
  bool m_bEnableSDFCollision = false;
  ezEnum<ezParticleRaycastHitReaction> m_CollisionReaction;
  float m_fCollisionBounceFactor = 0.5f;
  float m_fCollisionSlideFactor = 0.5f;
  float m_fCollisionThickness = 0.5f;

  ezColor m_ColorStart = ezColor::White;
  ezColor m_ColorEnd = ezColor(1, 1, 1, 0);

  ezEnum<ezGPUParticleRenderType> m_GPURenderType;
  ezUInt32 m_uiMaxTrailPoints = 16;
  float m_fVelocityStretch = 1.0f;

protected:
  friend class ezParticleTypeGPUFactory;

  virtual void Process(ezUInt64 uiNumElements) override {}

  void EnsureGPUBuffers() const;
  void DestroyGPUBuffers();

  ezProcessingStream* m_pStreamPosition = nullptr;
  ezProcessingStream* m_pStreamVelocity = nullptr;
  ezProcessingStream* m_pStreamLifeTime = nullptr;
  ezProcessingStream* m_pStreamSize = nullptr;
  ezProcessingStream* m_pStreamColor = nullptr;
  ezProcessingStream* m_pStreamRotationSpeed = nullptr;
  ezProcessingStream* m_pStreamRotationOffset = nullptr;

  mutable ezGALBufferHandle m_hParticleBuffer;
  mutable ezGALBufferHandle m_hCounterBuffer;
  mutable ezGALBufferHandle m_hTrailPositionBuffer;
  mutable bool m_bGPUBuffersCreated = false;
  mutable ezUInt32 m_uiGPUEmitIndex = 0;
  mutable ezUInt32 m_uiTrailWriteIndex = 0;
};
