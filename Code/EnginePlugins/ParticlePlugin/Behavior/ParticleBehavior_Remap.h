#pragma once

#include <ParticlePlugin/Behavior/ParticleBehavior.h>
#include <ParticlePlugin/Behavior/ParticleConditionalCommon.h>

/// Per-particle Remap: reads an input attribute and linearly maps it from
/// one range to another, writing the result to an output attribute.
///
/// Example: Map LifeFraction [0..1] to Size [2.0..0.1] to make particles
/// shrink over their lifetime.
/// Values outside InputMin..InputMax are clamped before mapping.
class EZ_PARTICLEPLUGIN_DLL ezParticleBehaviorFactory_Remap final : public ezParticleBehaviorFactory
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehaviorFactory_Remap, ezParticleBehaviorFactory);

public:
  ezParticleBehaviorFactory_Remap();

  virtual const ezRTTI* GetBehaviorType() const override;
  virtual void CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const override;
  virtual void QueryFinalizerDependencies(ezSet<const ezRTTI*>& inout_finalizerDeps) const override;

  virtual void Save(ezStreamWriter& inout_stream) const override;
  virtual void Load(ezStreamReader& inout_stream) override;

  ezEnum<ezParticleAttribute> m_InputAttribute;
  ezEnum<ezParticleAttribute> m_OutputAttribute;
  float m_fInputMin = 0.0f;
  float m_fInputMax = 1.0f;
  float m_fOutputMin = 0.0f;
  float m_fOutputMax = 1.0f;
  bool m_bClampOutput = true;
};

class EZ_PARTICLEPLUGIN_DLL ezParticleBehavior_Remap final : public ezParticleBehavior
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehavior_Remap, ezParticleBehavior);

public:
  ezEnum<ezParticleAttribute> m_InputAttribute;
  ezEnum<ezParticleAttribute> m_OutputAttribute;
  float m_fInputMin = 0.0f;
  float m_fInputMax = 1.0f;
  float m_fOutputMin = 0.0f;
  float m_fOutputMax = 1.0f;
  bool m_bClampOutput = true;

protected:
  virtual void CreateRequiredStreams() override;
  virtual void QueryOptionalStreams() override;
  virtual void Process(ezUInt64 uiNumElements) override;

  ezProcessingStream* m_pStreamPosition = nullptr;
  ezProcessingStream* m_pStreamVelocity = nullptr;
  ezProcessingStream* m_pStreamSize = nullptr;
  ezProcessingStream* m_pStreamColor = nullptr;
  ezProcessingStream* m_pStreamLifeTime = nullptr;
};
