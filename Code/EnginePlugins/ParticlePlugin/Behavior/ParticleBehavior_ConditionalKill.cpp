#include <ParticlePlugin/ParticlePluginPCH.h>

#include <Foundation/DataProcessing/Stream/ProcessingStreamIterator.h>
#include <Foundation/Profiling/Profiling.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_ConditionalKill.h>
#include <ParticlePlugin/System/ParticleSystemInstance.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_ENUM(ezParticleKillAttribute, 1)
  EZ_ENUM_CONSTANT(ezParticleKillAttribute::PositionX),
  EZ_ENUM_CONSTANT(ezParticleKillAttribute::PositionY),
  EZ_ENUM_CONSTANT(ezParticleKillAttribute::PositionZ),
  EZ_ENUM_CONSTANT(ezParticleKillAttribute::Speed),
  EZ_ENUM_CONSTANT(ezParticleKillAttribute::Size),
  EZ_ENUM_CONSTANT(ezParticleKillAttribute::ColorAlpha),
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezParticleComparisonOp, 1)
  EZ_ENUM_CONSTANT(ezParticleComparisonOp::Less),
  EZ_ENUM_CONSTANT(ezParticleComparisonOp::LessEqual),
  EZ_ENUM_CONSTANT(ezParticleComparisonOp::Greater),
  EZ_ENUM_CONSTANT(ezParticleComparisonOp::GreaterEqual),
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehaviorFactory_ConditionalKill, 1, ezRTTIDefaultAllocator<ezParticleBehaviorFactory_ConditionalKill>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ENUM_MEMBER_PROPERTY("Attribute", ezParticleKillAttribute, m_Attribute),
    EZ_ENUM_MEMBER_PROPERTY("Comparison", ezParticleComparisonOp, m_Comparison),
    EZ_MEMBER_PROPERTY("Threshold", m_fThreshold),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehavior_ConditionalKill, 1, ezRTTIDefaultAllocator<ezParticleBehavior_ConditionalKill>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezParticleBehaviorFactory_ConditionalKill::ezParticleBehaviorFactory_ConditionalKill() = default;

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
  const ezUInt8 uiVersion = 1;
  inout_stream << uiVersion;

  inout_stream << m_Attribute;
  inout_stream << m_Comparison;
  inout_stream << m_fThreshold;
}

void ezParticleBehaviorFactory_ConditionalKill::Load(ezStreamReader& inout_stream)
{
  ezUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  EZ_ASSERT_DEV(uiVersion <= 1, "Invalid version {0}", uiVersion);

  inout_stream >> m_Attribute;
  inout_stream >> m_Comparison;
  inout_stream >> m_fThreshold;
}

void ezParticleBehavior_ConditionalKill::CreateRequiredStreams()
{
  CreateStream("Position", ezProcessingStream::DataType::Float4, &m_pStreamPosition, false);
}

void ezParticleBehavior_ConditionalKill::QueryOptionalStreams()
{
  m_pStreamVelocity = GetOwnerSystem()->QueryStream("Velocity", ezProcessingStream::DataType::Half4);
  m_pStreamSize = GetOwnerSystem()->QueryStream("Size", ezProcessingStream::DataType::Half);
  m_pStreamColor = GetOwnerSystem()->QueryStream("Color", ezProcessingStream::DataType::Half4);
}

static float GetAttributeValue(
  ezParticleKillAttribute::Enum attr,
  const ezSimdVec4f& position,
  const ezFloat16Vec4* pVelocity,
  const ezFloat16* pSize,
  const ezFloat16Vec4* pColor)
{
  switch (attr)
  {
    case ezParticleKillAttribute::PositionX:
      return position.GetComponent<0>();
    case ezParticleKillAttribute::PositionY:
      return position.GetComponent<1>();
    case ezParticleKillAttribute::PositionZ:
      return position.GetComponent<2>();
    case ezParticleKillAttribute::Speed:
    {
      if (pVelocity)
        return static_cast<float>(pVelocity->w);
      return 0.0f;
    }
    case ezParticleKillAttribute::Size:
    {
      if (pSize)
        return static_cast<float>(*pSize);
      return 0.0f;
    }
    case ezParticleKillAttribute::ColorAlpha:
    {
      if (pColor)
        return static_cast<float>(pColor->w);
      return 1.0f;
    }
    default:
      return 0.0f;
  }
}

static bool CompareValue(ezParticleComparisonOp::Enum op, float fValue, float fThreshold)
{
  switch (op)
  {
    case ezParticleComparisonOp::Less:
      return fValue < fThreshold;
    case ezParticleComparisonOp::LessEqual:
      return fValue <= fThreshold;
    case ezParticleComparisonOp::Greater:
      return fValue > fThreshold;
    case ezParticleComparisonOp::GreaterEqual:
      return fValue >= fThreshold;
    default:
      return false;
  }
}

void ezParticleBehavior_ConditionalKill::Process(ezUInt64 uiNumElements)
{
  EZ_PROFILE_SCOPE("PFX: ConditionalKill");

  ezProcessingStreamIterator<ezSimdVec4f> itPosition(m_pStreamPosition, uiNumElements, 0);

  const ezFloat16Vec4* pVelocityData = m_pStreamVelocity ? m_pStreamVelocity->GetData<ezFloat16Vec4>() : nullptr;
  const ezFloat16* pSizeData = m_pStreamSize ? m_pStreamSize->GetData<ezFloat16>() : nullptr;
  const ezFloat16Vec4* pColorData = m_pStreamColor ? m_pStreamColor->GetData<ezFloat16Vec4>() : nullptr;

  ezUInt32 idx = 0;

  while (!itPosition.HasReachedEnd())
  {
    const ezSimdVec4f pos = itPosition.Current();

    const float fValue = GetAttributeValue(
      m_Attribute,
      pos,
      pVelocityData ? &pVelocityData[idx] : nullptr,
      pSizeData ? &pSizeData[idx] : nullptr,
      pColorData ? &pColorData[idx] : nullptr);

    if (CompareValue(m_Comparison, fValue, m_fThreshold))
    {
      m_pStreamGroup->RemoveElement(idx);
    }

    ++idx;
    itPosition.Advance();
  }
}


EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Behavior_ParticleBehavior_ConditionalKill);
