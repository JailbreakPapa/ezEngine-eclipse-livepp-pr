#pragma once

#include <ParticlePlugin/Behavior/ParticleBehavior.h>
#include <ParticlePlugin/Behavior/ParticleConditionalCommon.h>

/// Per-particle Switch: divides an input attribute's range into N+1 buckets
/// using N thresholds and writes a different value to the output for each bucket.
///
/// The number of cases is dynamic (2 to 8). N thresholds define N+1 buckets:
///   input < Threshold[0] -> Values[0]
///   Threshold[0] <= input < Threshold[1] -> Values[1]
///   ...
///   input >= Threshold[N-1] -> Values[N]
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

  ezHybridArray<float, 4> m_Thresholds;
  ezHybridArray<float, 4> m_Values;
};

class EZ_PARTICLEPLUGIN_DLL ezParticleBehavior_Switch final : public ezParticleBehavior
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehavior_Switch, ezParticleBehavior);

public:
  ezEnum<ezParticleAttribute> m_InputAttribute;
  ezEnum<ezParticleAttribute> m_OutputAttribute;
  ezHybridArray<float, 4> m_Thresholds;
  ezHybridArray<float, 4> m_Values;

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
