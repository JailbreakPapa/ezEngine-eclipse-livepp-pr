#pragma once

#include <ParticlePlugin/Behavior/ParticleBehavior.h>

/// Which particle attribute to evaluate for the condition
struct EZ_PARTICLEPLUGIN_DLL ezParticleKillAttribute
{
  using StorageType = ezUInt8;

  enum Enum
  {
    PositionX,
    PositionY,
    PositionZ,
    Speed,
    Size,
    ColorAlpha,

    Default = PositionZ
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_PARTICLEPLUGIN_DLL, ezParticleKillAttribute);

/// Comparison operator for the conditional kill
struct EZ_PARTICLEPLUGIN_DLL ezParticleComparisonOp
{
  using StorageType = ezUInt8;

  enum Enum
  {
    Less,
    LessEqual,
    Greater,
    GreaterEqual,

    Default = Less
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_PARTICLEPLUGIN_DLL, ezParticleComparisonOp);

/// Kills particles when a chosen attribute passes a threshold.
///
/// For example, kill all particles below a certain height (PositionZ < -5)
/// or all particles that have slowed below a minimum speed (Speed < 0.1).
class EZ_PARTICLEPLUGIN_DLL ezParticleBehaviorFactory_ConditionalKill final : public ezParticleBehaviorFactory
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehaviorFactory_ConditionalKill, ezParticleBehaviorFactory);

public:
  ezParticleBehaviorFactory_ConditionalKill();

  virtual const ezRTTI* GetBehaviorType() const override;
  virtual void CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const override;

  virtual void Save(ezStreamWriter& inout_stream) const override;
  virtual void Load(ezStreamReader& inout_stream) override;

  ezEnum<ezParticleKillAttribute> m_Attribute;
  ezEnum<ezParticleComparisonOp> m_Comparison;
  float m_fThreshold = 0.0f;
};

class EZ_PARTICLEPLUGIN_DLL ezParticleBehavior_ConditionalKill final : public ezParticleBehavior
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehavior_ConditionalKill, ezParticleBehavior);

public:
  ezEnum<ezParticleKillAttribute> m_Attribute;
  ezEnum<ezParticleComparisonOp> m_Comparison;
  float m_fThreshold = 0.0f;

protected:
  virtual void CreateRequiredStreams() override;
  virtual void QueryOptionalStreams() override;
  virtual void Process(ezUInt64 uiNumElements) override;

  ezProcessingStream* m_pStreamPosition = nullptr;
  ezProcessingStream* m_pStreamVelocity = nullptr;
  ezProcessingStream* m_pStreamSize = nullptr;
  ezProcessingStream* m_pStreamColor = nullptr;
};
