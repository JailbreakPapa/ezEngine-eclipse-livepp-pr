#pragma once

#include <ParticlePlugin/Behavior/ParticleBehavior.h>

/// Where the attractor pulls particles toward
struct EZ_PARTICLEPLUGIN_DLL ezParticleAttractorTarget
{
  using StorageType = ezUInt8;

  enum Enum
  {
    EffectOrigin, ///< Attract toward the effect's world position
    CustomPosition, ///< Attract toward a specified world-space point

    Default = EffectOrigin
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_PARTICLEPLUGIN_DLL, ezParticleAttractorTarget);

/// Pulls particles toward a point with distance-based falloff.
///
/// Can modify velocity (physically correct acceleration) or position directly (snapping).
/// MinDistance prevents singularity at the attractor center.
class EZ_PARTICLEPLUGIN_DLL ezParticleBehaviorFactory_AttractToPosition final : public ezParticleBehaviorFactory
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehaviorFactory_AttractToPosition, ezParticleBehaviorFactory);

public:
  ezParticleBehaviorFactory_AttractToPosition();

  virtual const ezRTTI* GetBehaviorType() const override;
  virtual void CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const override;
  virtual void QueryFinalizerDependencies(ezSet<const ezRTTI*>& inout_finalizerDeps) const override;

  virtual void Save(ezStreamWriter& inout_stream) const override;
  virtual void Load(ezStreamReader& inout_stream) override;

  ezEnum<ezParticleAttractorTarget> m_Target;
  ezVec3 m_vCustomPosition = ezVec3::MakeZero();
  float m_fForce = 5.0f;
  float m_fMaxDistance = 10.0f;
  float m_fMinDistance = 0.1f;
  bool m_bAffectVelocity = true;
};

class EZ_PARTICLEPLUGIN_DLL ezParticleBehavior_AttractToPosition final : public ezParticleBehavior
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehavior_AttractToPosition, ezParticleBehavior);

public:
  ezEnum<ezParticleAttractorTarget> m_Target;
  ezVec3 m_vCustomPosition = ezVec3::MakeZero();
  float m_fForce = 5.0f;
  float m_fMaxDistance = 10.0f;
  float m_fMinDistance = 0.1f;
  bool m_bAffectVelocity = true;

protected:
  virtual void CreateRequiredStreams() override;
  virtual void Process(ezUInt64 uiNumElements) override;

  ezProcessingStream* m_pStreamPosition = nullptr;
  ezProcessingStream* m_pStreamVelocity = nullptr;
};
