#include <ParticlePlugin/ParticlePluginPCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_Switch.h>
#include <ParticlePlugin/Effect/ParticleEffectInstance.h>
#include <ParticlePlugin/Finalizer/ParticleFinalizer_ApplyVelocity.h>
#include <ParticlePlugin/System/ParticleSystemInstance.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehaviorFactory_Switch, 2, ezRTTIDefaultAllocator<ezParticleBehaviorFactory_Switch>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ENUM_MEMBER_PROPERTY("InputAttribute", ezParticleAttribute, m_InputAttribute)->AddAttributes(new ezDefaultValueAttribute((ezInt32)ezParticleAttribute::LifeFraction)),
    EZ_ENUM_MEMBER_PROPERTY("OutputAttribute", ezParticleAttribute, m_OutputAttribute),
    EZ_ARRAY_MEMBER_PROPERTY("Thresholds", m_Thresholds),
    EZ_ARRAY_MEMBER_PROPERTY("Values", m_Values),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehavior_Switch, 1, ezRTTIDefaultAllocator<ezParticleBehavior_Switch>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezParticleBehaviorFactory_Switch::ezParticleBehaviorFactory_Switch()
{
  // Default: 3 thresholds, 4 values (matches the old hardcoded layout)
  m_Thresholds.SetCount(3);
  m_Thresholds[0] = 0.25f;
  m_Thresholds[1] = 0.5f;
  m_Thresholds[2] = 0.75f;

  m_Values.SetCount(4);
  m_Values[0] = 0.0f;
  m_Values[1] = 1.0f;
  m_Values[2] = 2.0f;
  m_Values[3] = 3.0f;
}

const ezRTTI* ezParticleBehaviorFactory_Switch::GetBehaviorType() const
{
  return ezGetStaticRTTI<ezParticleBehavior_Switch>();
}

void ezParticleBehaviorFactory_Switch::CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const
{
  ezParticleBehavior_Switch* pBehavior = static_cast<ezParticleBehavior_Switch*>(pObject);

  pBehavior->m_InputAttribute = m_InputAttribute;
  pBehavior->m_OutputAttribute = m_OutputAttribute;
  pBehavior->m_Thresholds = m_Thresholds;
  pBehavior->m_Values = m_Values;
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
  const ezUInt8 uiVersion = 2;
  inout_stream << uiVersion;

  inout_stream << m_InputAttribute;
  inout_stream << m_OutputAttribute;

  inout_stream << (ezUInt8)m_Thresholds.GetCount();
  for (float f : m_Thresholds)
    inout_stream << f;

  inout_stream << (ezUInt8)m_Values.GetCount();
  for (float f : m_Values)
    inout_stream << f;
}

void ezParticleBehaviorFactory_Switch::Load(ezStreamReader& inout_stream)
{
  ezUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  EZ_ASSERT_DEV(uiVersion <= 2, "Invalid version {0}", uiVersion);

  inout_stream >> m_InputAttribute;
  inout_stream >> m_OutputAttribute;

  if (uiVersion == 1)
  {
    // Old format: hardcoded 3 thresholds + 4 values
    float fThreshold1, fThreshold2, fThreshold3;
    float fValue0, fValue1, fValue2, fValue3;
    inout_stream >> fThreshold1 >> fThreshold2 >> fThreshold3;
    inout_stream >> fValue0 >> fValue1 >> fValue2 >> fValue3;

    m_Thresholds.SetCount(3);
    m_Thresholds[0] = fThreshold1;
    m_Thresholds[1] = fThreshold2;
    m_Thresholds[2] = fThreshold3;

    m_Values.SetCount(4);
    m_Values[0] = fValue0;
    m_Values[1] = fValue1;
    m_Values[2] = fValue2;
    m_Values[3] = fValue3;
  }
  else
  {
    ezUInt8 uiCount = 0;

    inout_stream >> uiCount;
    m_Thresholds.SetCount(uiCount);
    for (ezUInt8 i = 0; i < uiCount; ++i)
      inout_stream >> m_Thresholds[i];

    inout_stream >> uiCount;
    m_Values.SetCount(uiCount);
    for (ezUInt8 i = 0; i < uiCount; ++i)
      inout_stream >> m_Values[i];
  }
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

  const ezUInt32 uiNumThresholds = m_Thresholds.GetCount();

  if (m_Values.IsEmpty())
    return;

  for (ezUInt32 i = 0; i < (ezUInt32)uiNumElements; ++i)
  {
    const float fInput = ezReadParticleAttribute(m_InputAttribute, i,
      m_pStreamPosition, m_pStreamVelocity, m_pStreamSize, m_pStreamColor, m_pStreamLifeTime);

    // Find which bucket the input falls into
    ezUInt32 uiBucket = uiNumThresholds; // default: last bucket
    for (ezUInt32 t = 0; t < uiNumThresholds; ++t)
    {
      if (fInput < m_Thresholds[t])
      {
        uiBucket = t;
        break;
      }
    }

    // Clamp to valid value index
    const float fOutput = m_Values[ezMath::Min(uiBucket, m_Values.GetCount() - 1)];

    ezWriteParticleAttribute(m_OutputAttribute, i,
      fOutput, m_pStreamPosition, m_pStreamVelocity, m_pStreamSize, m_pStreamColor);
  }
}


EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Behavior_ParticleBehavior_Switch);
