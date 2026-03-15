#pragma once

#include <ParticlePlugin/Behavior/ParticleBehavior.h>
#include <ParticlePlugin/Behavior/ParticleConditionalCommon.h>

/// Per-particle IF: compares an input attribute against a threshold
/// and writes one of two values to an output attribute.
///
/// Example: If Speed > 5, set Size to 2.0, else set Size to 0.5.
/// This allows particles to change appearance or behavior based on their state.
class EZ_PARTICLEPLUGIN_DLL ezParticleBehaviorFactory_If final : public ezParticleBehaviorFactory
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehaviorFactory_If, ezParticleBehaviorFactory);

public:
  ezParticleBehaviorFactory_If();

  virtual const ezRTTI* GetBehaviorType() const override;
  virtual void CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const override;
  virtual void QueryFinalizerDependencies(ezSet<const ezRTTI*>& inout_finalizerDeps) const override;

  virtual void Save(ezStreamWriter& inout_stream) const override;
  virtual void Load(ezStreamReader& inout_stream) override;

  /// The attribute to test the condition against
  ezEnum<ezParticleAttribute> m_ConditionInput;
  ezEnum<ezParticleConditionOp> m_Comparison;
  float m_fThreshold = 0.0f;

  /// The attribute to write the result to
  ezEnum<ezParticleAttribute> m_OutputAttribute;
  float m_fTrueValue = 1.0f;
  float m_fFalseValue = 0.0f;
};

class EZ_PARTICLEPLUGIN_DLL ezParticleBehavior_If final : public ezParticleBehavior
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehavior_If, ezParticleBehavior);

public:
  ezEnum<ezParticleAttribute> m_ConditionInput;
  ezEnum<ezParticleConditionOp> m_Comparison;
  float m_fThreshold = 0.0f;

  ezEnum<ezParticleAttribute> m_OutputAttribute;
  float m_fTrueValue = 1.0f;
  float m_fFalseValue = 0.0f;

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
