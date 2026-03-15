#include <ParticlePlugin/ParticlePluginPCH.h>

#include <Foundation/DataProcessing/Stream/ProcessingStreamIterator.h>
#include <Foundation/Profiling/Profiling.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_ConformToSphere.h>
#include <ParticlePlugin/Effect/ParticleEffectInstance.h>
#include <ParticlePlugin/System/ParticleSystemInstance.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_ENUM(ezParticleSphereOutOfBoundsMode, 1)
  EZ_ENUM_CONSTANT(ezParticleSphereOutOfBoundsMode::Kill),
  EZ_ENUM_CONSTANT(ezParticleSphereOutOfBoundsMode::Constrain),
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehaviorFactory_ConformToSphere, 1, ezRTTIDefaultAllocator<ezParticleBehaviorFactory_ConformToSphere>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("CenterOffset", m_vCenterOffset),
    EZ_MEMBER_PROPERTY("Radius", m_fRadius)->AddAttributes(new ezDefaultValueAttribute(3.0f), new ezClampValueAttribute(0.01f, {})),
    EZ_ENUM_MEMBER_PROPERTY("OutOfBoundsMode", ezParticleSphereOutOfBoundsMode, m_OutOfBoundsMode),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezSphereVisualizerAttribute("Radius", ezColor::LightGreen, nullptr, ezVisualizerAnchor::Center, ezVec3::MakeZero(), "CenterOffset")
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehavior_ConformToSphere, 1, ezRTTIDefaultAllocator<ezParticleBehavior_ConformToSphere>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezParticleBehaviorFactory_ConformToSphere::ezParticleBehaviorFactory_ConformToSphere() = default;

const ezRTTI* ezParticleBehaviorFactory_ConformToSphere::GetBehaviorType() const
{
  return ezGetStaticRTTI<ezParticleBehavior_ConformToSphere>();
}

void ezParticleBehaviorFactory_ConformToSphere::CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const
{
  ezParticleBehavior_ConformToSphere* pBehavior = static_cast<ezParticleBehavior_ConformToSphere*>(pObject);

  pBehavior->m_vCenterOffset = m_vCenterOffset;
  pBehavior->m_fRadius = m_fRadius;
  pBehavior->m_OutOfBoundsMode = m_OutOfBoundsMode;
}

void ezParticleBehaviorFactory_ConformToSphere::Save(ezStreamWriter& inout_stream) const
{
  const ezUInt8 uiVersion = 1;
  inout_stream << uiVersion;

  inout_stream << m_vCenterOffset;
  inout_stream << m_fRadius;
  inout_stream << m_OutOfBoundsMode;
}

void ezParticleBehaviorFactory_ConformToSphere::Load(ezStreamReader& inout_stream)
{
  ezUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  EZ_ASSERT_DEV(uiVersion <= 1, "Invalid version {0}", uiVersion);

  inout_stream >> m_vCenterOffset;
  inout_stream >> m_fRadius;
  inout_stream >> m_OutOfBoundsMode;
}

void ezParticleBehavior_ConformToSphere::CreateRequiredStreams()
{
  CreateStream("Position", ezProcessingStream::DataType::Float4, &m_pStreamPosition, false);
}

void ezParticleBehavior_ConformToSphere::Process(ezUInt64 uiNumElements)
{
  EZ_PROFILE_SCOPE("PFX: ConformToSphere");

  const ezSimdTransform trans = ezSimdConversion::ToTransform(GetOwnerSystem()->GetTransform());
  const ezSimdVec4f center = trans.TransformPosition(ezSimdConversion::ToVec3(m_vCenterOffset));
  const float fRadiusSqr = m_fRadius * m_fRadius;

  ezProcessingStreamIterator<ezSimdVec4f> itPosition(m_pStreamPosition, uiNumElements, 0);

  if (m_OutOfBoundsMode == ezParticleSphereOutOfBoundsMode::Kill)
  {
    ezUInt32 idx = 0;

    while (!itPosition.HasReachedEnd())
    {
      const ezSimdVec4f pos = itPosition.Current();
      const ezSimdVec4f diff = pos - center;
      const float distSqr = diff.Dot<3>(diff);

      if (distSqr > fRadiusSqr)
      {
        m_pStreamGroup->RemoveElement(idx);
      }

      ++idx;
      itPosition.Advance();
    }
  }
  else // Constrain
  {
    while (!itPosition.HasReachedEnd())
    {
      const ezSimdVec4f pos = itPosition.Current();
      const ezSimdVec4f diff = pos - center;
      const float distSqr = diff.Dot<3>(diff);

      if (distSqr > fRadiusSqr)
      {
        // Push particle back to the sphere surface
        const float dist = ezMath::Sqrt(distSqr);
        const ezSimdVec4f normalized = diff / ezSimdFloat(dist);
        itPosition.Current() = center + normalized * ezSimdFloat(m_fRadius);
      }

      itPosition.Advance();
    }
  }
}


EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Behavior_ParticleBehavior_ConformToSphere);
