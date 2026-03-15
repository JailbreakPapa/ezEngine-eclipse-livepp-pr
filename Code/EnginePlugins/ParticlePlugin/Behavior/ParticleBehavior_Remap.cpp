#include <ParticlePlugin/ParticlePluginPCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_Remap.h>
#include <ParticlePlugin/Effect/ParticleEffectInstance.h>
#include <ParticlePlugin/Finalizer/ParticleFinalizer_ApplyVelocity.h>
#include <ParticlePlugin/System/ParticleSystemInstance.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehaviorFactory_Remap, 1, ezRTTIDefaultAllocator<ezParticleBehaviorFactory_Remap>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ENUM_MEMBER_PROPERTY("InputAttribute", ezParticleAttribute, m_InputAttribute)->AddAttributes(new ezDefaultValueAttribute((ezInt32)ezParticleAttribute::LifeFraction)),
    EZ_ENUM_MEMBER_PROPERTY("OutputAttribute", ezParticleAttribute, m_OutputAttribute),
    EZ_MEMBER_PROPERTY("InputMin", m_fInputMin)->AddAttributes(new ezDefaultValueAttribute(0.0f)),
    EZ_MEMBER_PROPERTY("InputMax", m_fInputMax)->AddAttributes(new ezDefaultValueAttribute(1.0f)),
    EZ_MEMBER_PROPERTY("OutputMin", m_fOutputMin)->AddAttributes(new ezDefaultValueAttribute(0.0f)),
    EZ_MEMBER_PROPERTY("OutputMax", m_fOutputMax)->AddAttributes(new ezDefaultValueAttribute(1.0f)),
    EZ_MEMBER_PROPERTY("ClampOutput", m_bClampOutput)->AddAttributes(new ezDefaultValueAttribute(true)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehavior_Remap, 1, ezRTTIDefaultAllocator<ezParticleBehavior_Remap>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezParticleBehaviorFactory_Remap::ezParticleBehaviorFactory_Remap() = default;

const ezRTTI* ezParticleBehaviorFactory_Remap::GetBehaviorType() const
{
  return ezGetStaticRTTI<ezParticleBehavior_Remap>();
}

void ezParticleBehaviorFactory_Remap::CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const
{
  ezParticleBehavior_Remap* pBehavior = static_cast<ezParticleBehavior_Remap*>(pObject);

  pBehavior->m_InputAttribute = m_InputAttribute;
  pBehavior->m_OutputAttribute = m_OutputAttribute;
  pBehavior->m_fInputMin = m_fInputMin;
  pBehavior->m_fInputMax = m_fInputMax;
  pBehavior->m_fOutputMin = m_fOutputMin;
  pBehavior->m_fOutputMax = m_fOutputMax;
  pBehavior->m_bClampOutput = m_bClampOutput;
}

void ezParticleBehaviorFactory_Remap::QueryFinalizerDependencies(ezSet<const ezRTTI*>& inout_finalizerDeps) const
{
  if (m_OutputAttribute == ezParticleAttribute::Speed)
  {
    inout_finalizerDeps.Insert(ezGetStaticRTTI<ezParticleFinalizerFactory_ApplyVelocity>());
  }
}

void ezParticleBehaviorFactory_Remap::Save(ezStreamWriter& inout_stream) const
{
  const ezUInt8 uiVersion = 1;
  inout_stream << uiVersion;

  inout_stream << m_InputAttribute;
  inout_stream << m_OutputAttribute;
  inout_stream << m_fInputMin;
  inout_stream << m_fInputMax;
  inout_stream << m_fOutputMin;
  inout_stream << m_fOutputMax;
  inout_stream << m_bClampOutput;
}

void ezParticleBehaviorFactory_Remap::Load(ezStreamReader& inout_stream)
{
  ezUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  EZ_ASSERT_DEV(uiVersion <= 1, "Invalid version {0}", uiVersion);

  inout_stream >> m_InputAttribute;
  inout_stream >> m_OutputAttribute;
  inout_stream >> m_fInputMin;
  inout_stream >> m_fInputMax;
  inout_stream >> m_fOutputMin;
  inout_stream >> m_fOutputMax;
  inout_stream >> m_bClampOutput;
}

void ezParticleBehavior_Remap::CreateRequiredStreams()
{
  CreateStream("Position", ezProcessingStream::DataType::Float4, &m_pStreamPosition, false);
  CreateStream("Velocity", ezProcessingStream::DataType::Half4, &m_pStreamVelocity, false);
}

void ezParticleBehavior_Remap::QueryOptionalStreams()
{
  m_pStreamSize = GetOwnerSystem()->QueryStream("Size", ezProcessingStream::DataType::Half);
  m_pStreamColor = GetOwnerSystem()->QueryStream("Color", ezProcessingStream::DataType::Half4);
  m_pStreamLifeTime = GetOwnerSystem()->QueryStream("LifeTime", ezProcessingStream::DataType::Float2);
}

void ezParticleBehavior_Remap::Process(ezUInt64 uiNumElements)
{
  EZ_PROFILE_SCOPE("PFX: Remap");

  const float fInputRange = m_fInputMax - m_fInputMin;
  if (ezMath::IsZero(fInputRange, 0.0001f))
    return;

  const float fInvInputRange = 1.0f / fInputRange;
  const float fOutputRange = m_fOutputMax - m_fOutputMin;

  for (ezUInt32 i = 0; i < (ezUInt32)uiNumElements; ++i)
  {
    float fInput = ezReadParticleAttribute(m_InputAttribute, i,
      m_pStreamPosition, m_pStreamVelocity, m_pStreamSize, m_pStreamColor, m_pStreamLifeTime);

    // Normalize input to [0..1]
    float fNormalized = (fInput - m_fInputMin) * fInvInputRange;

    if (m_bClampOutput)
      fNormalized = ezMath::Clamp(fNormalized, 0.0f, 1.0f);

    // Map to output range
    const float fOutput = m_fOutputMin + fNormalized * fOutputRange;

    ezWriteParticleAttribute(m_OutputAttribute, i,
      fOutput, m_pStreamPosition, m_pStreamVelocity, m_pStreamSize, m_pStreamColor);
  }
}


EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Behavior_ParticleBehavior_Remap);
