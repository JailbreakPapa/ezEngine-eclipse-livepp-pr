#include <ParticlePlugin/ParticlePluginPCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_Switch.h>
#include <ParticlePlugin/Effect/ParticleEffectInstance.h>
#include <ParticlePlugin/Finalizer/ParticleFinalizer_ApplyVelocity.h>
#include <ParticlePlugin/System/ParticleSystemInstance.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehaviorFactory_Switch, 1, ezRTTIDefaultAllocator<ezParticleBehaviorFactory_Switch>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ENUM_MEMBER_PROPERTY("InputAttribute", ezParticleAttribute, m_InputAttribute)->AddAttributes(new ezDefaultValueAttribute((ezInt32)ezParticleAttribute::LifeFraction)),
    EZ_ENUM_MEMBER_PROPERTY("OutputAttribute", ezParticleAttribute, m_OutputAttribute),
    EZ_MEMBER_PROPERTY("Threshold1", m_fThreshold1)->AddAttributes(new ezDefaultValueAttribute(0.25f)),
    EZ_MEMBER_PROPERTY("Threshold2", m_fThreshold2)->AddAttributes(new ezDefaultValueAttribute(0.5f)),
    EZ_MEMBER_PROPERTY("Threshold3", m_fThreshold3)->AddAttributes(new ezDefaultValueAttribute(0.75f)),
    EZ_MEMBER_PROPERTY("Value0", m_fValue0)->AddAttributes(new ezDefaultValueAttribute(0.0f)),
    EZ_MEMBER_PROPERTY("Value1", m_fValue1)->AddAttributes(new ezDefaultValueAttribute(1.0f)),
    EZ_MEMBER_PROPERTY("Value2", m_fValue2)->AddAttributes(new ezDefaultValueAttribute(2.0f)),
    EZ_MEMBER_PROPERTY("Value3", m_fValue3)->AddAttributes(new ezDefaultValueAttribute(3.0f)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehavior_Switch, 1, ezRTTIDefaultAllocator<ezParticleBehavior_Switch>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezParticleBehaviorFactory_Switch::ezParticleBehaviorFactory_Switch() = default;

const ezRTTI* ezParticleBehaviorFactory_Switch::GetBehaviorType() const
{
  return ezGetStaticRTTI<ezParticleBehavior_Switch>();
}

void ezParticleBehaviorFactory_Switch::CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const
{
  ezParticleBehavior_Switch* pBehavior = static_cast<ezParticleBehavior_Switch*>(pObject);

  pBehavior->m_InputAttribute = m_InputAttribute;
  pBehavior->m_OutputAttribute = m_OutputAttribute;
  pBehavior->m_fThreshold1 = m_fThreshold1;
  pBehavior->m_fThreshold2 = m_fThreshold2;
  pBehavior->m_fThreshold3 = m_fThreshold3;
  pBehavior->m_fValue0 = m_fValue0;
  pBehavior->m_fValue1 = m_fValue1;
  pBehavior->m_fValue2 = m_fValue2;
  pBehavior->m_fValue3 = m_fValue3;
}

void ezParticleBehaviorFactory_Switch::QueryFinalizerDependencies(ezSet<const ezRTTI*>& inout_finalizerDeps) const
{
  if (m_OutputAttribute == ezParticleAttribute::Speed)
  {
    inout_finalizerDeps.Insert(ezGetStaticRTTI<ezParticleFinalizerFactory_ApplyVelocity>());
  }
}

void ezParticleBehaviorFactory_Switch::Save(ezStreamWriter& inout_stream) const
{
  const ezUInt8 uiVersion = 1;
  inout_stream << uiVersion;

  inout_stream << m_InputAttribute;
  inout_stream << m_OutputAttribute;
  inout_stream << m_fThreshold1;
  inout_stream << m_fThreshold2;
  inout_stream << m_fThreshold3;
  inout_stream << m_fValue0;
  inout_stream << m_fValue1;
  inout_stream << m_fValue2;
  inout_stream << m_fValue3;
}

void ezParticleBehaviorFactory_Switch::Load(ezStreamReader& inout_stream)
{
  ezUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  EZ_ASSERT_DEV(uiVersion <= 1, "Invalid version {0}", uiVersion);

  inout_stream >> m_InputAttribute;
  inout_stream >> m_OutputAttribute;
  inout_stream >> m_fThreshold1;
  inout_stream >> m_fThreshold2;
  inout_stream >> m_fThreshold3;
  inout_stream >> m_fValue0;
  inout_stream >> m_fValue1;
  inout_stream >> m_fValue2;
  inout_stream >> m_fValue3;
}

void ezParticleBehavior_Switch::CreateRequiredStreams()
{
  CreateStream("Position", ezProcessingStream::DataType::Float4, &m_pStreamPosition, false);
  CreateStream("Velocity", ezProcessingStream::DataType::Half4, &m_pStreamVelocity, false);
}

void ezParticleBehavior_Switch::QueryOptionalStreams()
{
  m_pStreamSize = GetOwnerSystem()->QueryStream("Size", ezProcessingStream::DataType::Half);
  m_pStreamColor = GetOwnerSystem()->QueryStream("Color", ezProcessingStream::DataType::Half4);
  m_pStreamLifeTime = GetOwnerSystem()->QueryStream("LifeTime", ezProcessingStream::DataType::Float2);
}

void ezParticleBehavior_Switch::Process(ezUInt64 uiNumElements)
{
  EZ_PROFILE_SCOPE("PFX: Switch");

  for (ezUInt32 i = 0; i < (ezUInt32)uiNumElements; ++i)
  {
    const float fInput = ezReadParticleAttribute(m_InputAttribute, i,
      m_pStreamPosition, m_pStreamVelocity, m_pStreamSize, m_pStreamColor, m_pStreamLifeTime);

    float fOutput;
    if (fInput < m_fThreshold1)
      fOutput = m_fValue0;
    else if (fInput < m_fThreshold2)
      fOutput = m_fValue1;
    else if (fInput < m_fThreshold3)
      fOutput = m_fValue2;
    else
      fOutput = m_fValue3;

    ezWriteParticleAttribute(m_OutputAttribute, i,
      fOutput, m_pStreamPosition, m_pStreamVelocity, m_pStreamSize, m_pStreamColor);
  }
}


EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Behavior_ParticleBehavior_Switch);
