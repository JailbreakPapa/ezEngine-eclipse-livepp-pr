#include <ParticlePlugin/ParticlePluginPCH.h>

#include <Foundation/DataProcessing/Stream/ProcessingStreamIterator.h>
#include <Foundation/Profiling/Profiling.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_ConditionalKill.h>
#include <ParticlePlugin/System/ParticleSystemInstance.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehaviorFactory_ConditionalKill, 2, ezRTTIDefaultAllocator<ezParticleBehaviorFactory_ConditionalKill>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ENUM_MEMBER_PROPERTY("Attribute", ezParticleAttribute, m_Attribute)->AddAttributes(new ezDefaultValueAttribute((ezInt32)ezParticleAttribute::PositionZ)),
    EZ_ENUM_MEMBER_PROPERTY("Comparison", ezComparisonOperator, m_Comparison)->AddAttributes(new ezDefaultValueAttribute((ezInt32)ezComparisonOperator::Less)),
    EZ_MEMBER_PROPERTY("Threshold", m_fThreshold),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehavior_ConditionalKill, 1, ezRTTIDefaultAllocator<ezParticleBehavior_ConditionalKill>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezParticleBehaviorFactory_ConditionalKill::ezParticleBehaviorFactory_ConditionalKill()
{
  m_Attribute = ezParticleAttribute::PositionZ;
  m_Comparison = ezComparisonOperator::Less;
}

const ezRTTI* ezParticleBehaviorFactory_ConditionalKill::GetBehaviorType() const
{
  return ezGetStaticRTTI<ezParticleBehavior_ConditionalKill>();
}

void ezParticleBehaviorFactory_ConditionalKill::CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const
{
  ezParticleBehavior_ConditionalKill* pBehavior = static_cast<ezParticleBehavior_ConditionalKill*>(pObject);

  pBehavior->m_Attribute = m_Attribute;
  pBehavior->m_Comparison = m_Comparison;
  pBehavior->m_fThreshold = m_fThreshold;
}

void ezParticleBehaviorFactory_ConditionalKill::Save(ezStreamWriter& inout_stream) const
{
  const ezUInt8 uiVersion = 2;
  inout_stream << uiVersion;

  inout_stream << m_Attribute;
  inout_stream << m_Comparison;
  inout_stream << m_fThreshold;
}

void ezParticleBehaviorFactory_ConditionalKill::Load(ezStreamReader& inout_stream)
{
  ezUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  EZ_ASSERT_DEV(uiVersion <= 2, "Invalid version {0}", uiVersion);

  if (uiVersion == 1)
  {
    // Old format used ezParticleKillAttribute and ezParticleComparisonOp
    ezUInt8 uiOldAttr = 0;
    ezUInt8 uiOldOp = 0;
    inout_stream >> uiOldAttr;
    inout_stream >> uiOldOp;
    inout_stream >> m_fThreshold;

    // Map old ezParticleKillAttribute to ezParticleAttribute
    // Old: PositionX=0 PositionY=1 PositionZ=2 Speed=3 Size=4 ColorAlpha=5
    // New: PositionX=0 PositionY=1 PositionZ=2 Speed=3 Size=4 LifeFraction=5 ColorR=6 ColorG=7 ColorB=8 ColorA=9
    switch (uiOldAttr)
    {
      case 0: m_Attribute = ezParticleAttribute::PositionX; break;
      case 1: m_Attribute = ezParticleAttribute::PositionY; break;
      case 2: m_Attribute = ezParticleAttribute::PositionZ; break;
      case 3: m_Attribute = ezParticleAttribute::Speed; break;
      case 4: m_Attribute = ezParticleAttribute::Size; break;
      case 5: m_Attribute = ezParticleAttribute::ColorA; break; // ColorAlpha -> ColorA
      default: m_Attribute = ezParticleAttribute::PositionZ; break;
    }

    // Map old ezParticleComparisonOp to ezComparisonOperator
    // Old: Less=0 LessEqual=1 Greater=2 GreaterEqual=3
    // New: Equal=0 NotEqual=1 Less=2 LessEqual=3 Greater=4 GreaterEqual=5
    switch (uiOldOp)
    {
      case 0: m_Comparison = ezComparisonOperator::Less; break;
      case 1: m_Comparison = ezComparisonOperator::LessEqual; break;
      case 2: m_Comparison = ezComparisonOperator::Greater; break;
      case 3: m_Comparison = ezComparisonOperator::GreaterEqual; break;
      default: m_Comparison = ezComparisonOperator::Less; break;
    }
  }
  else
  {
    inout_stream >> m_Attribute;
    inout_stream >> m_Comparison;
    inout_stream >> m_fThreshold;
  }
}

void ezParticleBehavior_ConditionalKill::CreateRequiredStreams()
{
  CreateStream("Position", ezProcessingStream::DataType::Float4, &m_pStreamPosition, false);
  CreateStream("Velocity", ezProcessingStream::DataType::Half4, &m_pStreamVelocity, false);
}

void ezParticleBehavior_ConditionalKill::QueryOptionalStreams()
{
  m_pStreamSize = GetOwnerSystem()->QueryStream("Size", ezProcessingStream::DataType::Half);
  m_pStreamColor = GetOwnerSystem()->QueryStream("Color", ezProcessingStream::DataType::Half4);
  m_pStreamLifeTime = GetOwnerSystem()->QueryStream("LifeTime", ezProcessingStream::DataType::Float2);
}

void ezParticleBehavior_ConditionalKill::Process(ezUInt64 uiNumElements)
{
  EZ_PROFILE_SCOPE("PFX: ConditionalKill");

  for (ezUInt32 idx = 0; idx < (ezUInt32)uiNumElements; ++idx)
  {
    const float fValue = ezReadParticleAttribute(m_Attribute, idx,
      m_pStreamPosition, m_pStreamVelocity, m_pStreamSize, m_pStreamColor, m_pStreamLifeTime);

    if (ezEvaluateParticleCondition(m_Comparison, fValue, m_fThreshold))
    {
      m_pStreamGroup->RemoveElement(idx);
    }
  }
}


EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Behavior_ParticleBehavior_ConditionalKill);
