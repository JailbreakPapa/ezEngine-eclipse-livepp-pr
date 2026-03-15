#include <ParticlePlugin/ParticlePluginPCH.h>

#include <Foundation/DataProcessing/Stream/ProcessingStreamIterator.h>
#include <Foundation/Profiling/Profiling.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_If.h>
#include <ParticlePlugin/Effect/ParticleEffectInstance.h>
#include <ParticlePlugin/Finalizer/ParticleFinalizer_ApplyVelocity.h>
#include <ParticlePlugin/System/ParticleSystemInstance.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehaviorFactory_If, 1, ezRTTIDefaultAllocator<ezParticleBehaviorFactory_If>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ENUM_MEMBER_PROPERTY("ConditionInput", ezParticleAttribute, m_ConditionInput),
    EZ_ENUM_MEMBER_PROPERTY("Comparison", ezParticleConditionOp, m_Comparison),
    EZ_MEMBER_PROPERTY("Threshold", m_fThreshold),
    EZ_ENUM_MEMBER_PROPERTY("OutputAttribute", ezParticleAttribute, m_OutputAttribute),
    EZ_MEMBER_PROPERTY("TrueValue", m_fTrueValue)->AddAttributes(new ezDefaultValueAttribute(1.0f)),
    EZ_MEMBER_PROPERTY("FalseValue", m_fFalseValue)->AddAttributes(new ezDefaultValueAttribute(0.0f)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehavior_If, 1, ezRTTIDefaultAllocator<ezParticleBehavior_If>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezParticleBehaviorFactory_If::ezParticleBehaviorFactory_If() = default;

const ezRTTI* ezParticleBehaviorFactory_If::GetBehaviorType() const
{
  return ezGetStaticRTTI<ezParticleBehavior_If>();
}

void ezParticleBehaviorFactory_If::CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const
{
  ezParticleBehavior_If* pBehavior = static_cast<ezParticleBehavior_If*>(pObject);

  pBehavior->m_ConditionInput = m_ConditionInput;
  pBehavior->m_Comparison = m_Comparison;
  pBehavior->m_fThreshold = m_fThreshold;
  pBehavior->m_OutputAttribute = m_OutputAttribute;
  pBehavior->m_fTrueValue = m_fTrueValue;
  pBehavior->m_fFalseValue = m_fFalseValue;
}

void ezParticleBehaviorFactory_If::QueryFinalizerDependencies(ezSet<const ezRTTI*>& inout_finalizerDeps) const
{
  if (m_OutputAttribute == ezParticleAttribute::Speed)
  {
    inout_finalizerDeps.Insert(ezGetStaticRTTI<ezParticleFinalizerFactory_ApplyVelocity>());
  }
}

void ezParticleBehaviorFactory_If::Save(ezStreamWriter& inout_stream) const
{
  const ezUInt8 uiVersion = 1;
  inout_stream << uiVersion;

  inout_stream << m_ConditionInput;
  inout_stream << m_Comparison;
  inout_stream << m_fThreshold;
  inout_stream << m_OutputAttribute;
  inout_stream << m_fTrueValue;
  inout_stream << m_fFalseValue;
}

void ezParticleBehaviorFactory_If::Load(ezStreamReader& inout_stream)
{
  ezUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  EZ_ASSERT_DEV(uiVersion <= 1, "Invalid version {0}", uiVersion);

  inout_stream >> m_ConditionInput;
  inout_stream >> m_Comparison;
  inout_stream >> m_fThreshold;
  inout_stream >> m_OutputAttribute;
  inout_stream >> m_fTrueValue;
  inout_stream >> m_fFalseValue;
}

void ezParticleBehavior_If::CreateRequiredStreams()
{
  CreateStream("Position", ezProcessingStream::DataType::Float4, &m_pStreamPosition, false);
  CreateStream("Velocity", ezProcessingStream::DataType::Half4, &m_pStreamVelocity, false);
}

void ezParticleBehavior_If::QueryOptionalStreams()
{
  m_pStreamSize = GetOwnerSystem()->QueryStream("Size", ezProcessingStream::DataType::Half);
  m_pStreamColor = GetOwnerSystem()->QueryStream("Color", ezProcessingStream::DataType::Half4);
  m_pStreamLifeTime = GetOwnerSystem()->QueryStream("LifeTime", ezProcessingStream::DataType::Float2);
}

void ezParticleBehavior_If::Process(ezUInt64 uiNumElements)
{
  EZ_PROFILE_SCOPE("PFX: If");

  for (ezUInt32 i = 0; i < (ezUInt32)uiNumElements; ++i)
  {
    const float fInput = ezReadParticleAttribute(m_ConditionInput, i,
      m_pStreamPosition, m_pStreamVelocity, m_pStreamSize, m_pStreamColor, m_pStreamLifeTime);

    const bool bResult = ezEvaluateParticleCondition(m_Comparison, fInput, m_fThreshold);
    const float fOutput = bResult ? m_fTrueValue : m_fFalseValue;

    ezWriteParticleAttribute(m_OutputAttribute, i,
      fOutput, m_pStreamPosition, m_pStreamVelocity, m_pStreamSize, m_pStreamColor);
  }
}


EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Behavior_ParticleBehavior_If);
