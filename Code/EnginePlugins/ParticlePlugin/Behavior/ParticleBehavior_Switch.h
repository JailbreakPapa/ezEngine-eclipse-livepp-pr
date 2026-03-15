#pragma once

#include <ParticlePlugin/Behavior/ParticleBehavior.h>
#include <ParticlePlugin/Behavior/ParticleConditionalCommon.h>

/// Per-particle Switch: divides an input attribute's range into up to 4 buckets
/// and writes a different value to the output for each bucket.
///
/// Example: Based on LifeFraction, set Size to different values at different
/// stages of the particle's life:
///   [0.0 - 0.25] -> 0.5 (small at birth)
///   [0.25 - 0.5] -> 2.0 (grows)
///   [0.5 - 0.75] -> 2.0 (stays large)
///   [0.75 - 1.0] -> 0.1 (shrinks before death)
class EZ_PARTICLEPLUGIN_DLL ezParticleBehaviorFactory_Switch final : public ezParticleBehaviorFactory
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehaviorFactory_Switch, ezParticleBehaviorFactory);

public:
  ezParticleBehaviorFactory_Switch();

  virtual const ezRTTI* GetBehaviorType() const override;
  virtual void CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const override;
  virtual void QueryFinalizerDependencies(ezSet<const ezRTTI*>& inout_finalizerDeps) const override;

  virtual void Save(ezStreamWriter& inout_stream) const override;
  virtual void Load(ezStreamReader& inout_stream) override;

  ezEnum<ezParticleAttribute> m_InputAttribute;
  ezEnum<ezParticleAttribute> m_OutputAttribute;

  /// Thresholds dividing the input range into buckets.
  /// Bucket 0: input < Threshold1
  /// Bucket 1: Threshold1 <= input < Threshold2
  /// Bucket 2: Threshold2 <= input < Threshold3
  /// Bucket 3: input >= Threshold3
  float m_fThreshold1 = 0.25f;
  float m_fThreshold2 = 0.5f;
  float m_fThreshold3 = 0.75f;

  float m_fValue0 = 0.0f;
  float m_fValue1 = 1.0f;
  float m_fValue2 = 2.0f;
  float m_fValue3 = 3.0f;
};

class EZ_PARTICLEPLUGIN_DLL ezParticleBehavior_Switch final : public ezParticleBehavior
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehavior_Switch, ezParticleBehavior);

public:
  ezEnum<ezParticleAttribute> m_InputAttribute;
  ezEnum<ezParticleAttribute> m_OutputAttribute;
  float m_fThreshold1 = 0.25f;
  float m_fThreshold2 = 0.5f;
  float m_fThreshold3 = 0.75f;
  float m_fValue0 = 0.0f;
  float m_fValue1 = 1.0f;
  float m_fValue2 = 2.0f;
  float m_fValue3 = 3.0f;

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
