#include <ParticlePlugin/ParticlePluginPCH.h>

#include <Foundation/DataProcessing/Stream/ProcessingStreamIterator.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/SimdMath/SimdConversion.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_AttractToPosition.h>
#include <ParticlePlugin/Effect/ParticleEffectInstance.h>
#include <ParticlePlugin/Finalizer/ParticleFinalizer_ApplyVelocity.h>
#include <ParticlePlugin/System/ParticleSystemInstance.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_ENUM(ezParticleAttractorTarget, 1)
  EZ_ENUM_CONSTANT(ezParticleAttractorTarget::EffectOrigin),
  EZ_ENUM_CONSTANT(ezParticleAttractorTarget::CustomPosition),
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehaviorFactory_AttractToPosition, 1, ezRTTIDefaultAllocator<ezParticleBehaviorFactory_AttractToPosition>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ENUM_MEMBER_PROPERTY("Target", ezParticleAttractorTarget, m_Target),
    EZ_MEMBER_PROPERTY("CustomPosition", m_vCustomPosition),
    EZ_MEMBER_PROPERTY("Force", m_fForce)->AddAttributes(new ezDefaultValueAttribute(5.0f)),
    EZ_MEMBER_PROPERTY("MaxDistance", m_fMaxDistance)->AddAttributes(new ezDefaultValueAttribute(10.0f), new ezClampValueAttribute(0.0f, {})),
    EZ_MEMBER_PROPERTY("MinDistance", m_fMinDistance)->AddAttributes(new ezDefaultValueAttribute(0.1f), new ezClampValueAttribute(0.001f, {})),
    EZ_MEMBER_PROPERTY("AffectVelocity", m_bAffectVelocity)->AddAttributes(new ezDefaultValueAttribute(true)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehavior_AttractToPosition, 1, ezRTTIDefaultAllocator<ezParticleBehavior_AttractToPosition>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezParticleBehaviorFactory_AttractToPosition::ezParticleBehaviorFactory_AttractToPosition() = default;

const ezRTTI* ezParticleBehaviorFactory_AttractToPosition::GetBehaviorType() const
{
  return ezGetStaticRTTI<ezParticleBehavior_AttractToPosition>();
}

void ezParticleBehaviorFactory_AttractToPosition::CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const
{
  ezParticleBehavior_AttractToPosition* pBehavior = static_cast<ezParticleBehavior_AttractToPosition*>(pObject);

  pBehavior->m_Target = m_Target;
  pBehavior->m_vCustomPosition = m_vCustomPosition;
  pBehavior->m_fForce = m_fForce;
  pBehavior->m_fMaxDistance = m_fMaxDistance;
  pBehavior->m_fMinDistance = m_fMinDistance;
  pBehavior->m_bAffectVelocity = m_bAffectVelocity;
}

void ezParticleBehaviorFactory_AttractToPosition::QueryFinalizerDependencies(ezSet<const ezRTTI*>& inout_finalizerDeps) const
{
  if (m_bAffectVelocity)
  {
    inout_finalizerDeps.Insert(ezGetStaticRTTI<ezParticleFinalizerFactory_ApplyVelocity>());
  }
}

void ezParticleBehaviorFactory_AttractToPosition::Save(ezStreamWriter& inout_stream) const
{
  const ezUInt8 uiVersion = 1;
  inout_stream << uiVersion;

  inout_stream << m_Target;
  inout_stream << m_vCustomPosition;
  inout_stream << m_fForce;
  inout_stream << m_fMaxDistance;
  inout_stream << m_fMinDistance;
  inout_stream << m_bAffectVelocity;
}

void ezParticleBehaviorFactory_AttractToPosition::Load(ezStreamReader& inout_stream)
{
  ezUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  EZ_ASSERT_DEV(uiVersion <= 1, "Invalid version {0}", uiVersion);

  inout_stream >> m_Target;
  inout_stream >> m_vCustomPosition;
  inout_stream >> m_fForce;
  inout_stream >> m_fMaxDistance;
  inout_stream >> m_fMinDistance;
  inout_stream >> m_bAffectVelocity;
}

void ezParticleBehavior_AttractToPosition::CreateRequiredStreams()
{
  CreateStream("Position", ezProcessingStream::DataType::Float4, &m_pStreamPosition, false);
  CreateStream("Velocity", ezProcessingStream::DataType::Half4, &m_pStreamVelocity, false);
}

void ezParticleBehavior_AttractToPosition::Process(ezUInt64 uiNumElements)
{
  EZ_PROFILE_SCOPE("PFX: AttractToPosition");

  const float tDiff = (float)m_TimeDiff.GetSeconds();
  if (tDiff <= 0.0f)
    return;

  // Determine attractor world position
  ezVec3 vTarget;
  if (m_Target == ezParticleAttractorTarget::EffectOrigin)
  {
    vTarget = GetOwnerEffect()->GetTransform().m_vPosition;
  }
  else
  {
    vTarget = m_vCustomPosition;
  }

  const float fMaxDistSqr = m_fMaxDistance * m_fMaxDistance;
  const float fMinDistSqr = m_fMinDistance * m_fMinDistance;

  ezProcessingStreamIterator<ezSimdVec4f> itPosition(m_pStreamPosition, uiNumElements, 0);
  ezProcessingStreamIterator<ezFloat16Vec4> itVelocity(m_pStreamVelocity, uiNumElements, 0);

  if (m_bAffectVelocity)
  {
    while (!itPosition.HasReachedEnd())
    {
      const ezSimdVec4f simPos = itPosition.Current();
      const ezVec3 pos(simPos.GetComponent<0>(), simPos.GetComponent<1>(), simPos.GetComponent<2>());

      const ezVec3 toTarget = vTarget - pos;
      const float distSqr = toTarget.GetLengthSquared();

      if (distSqr < fMaxDistSqr && distSqr > fMinDistSqr)
      {
        const float dist = ezMath::Sqrt(distSqr);
        const ezVec3 dirToTarget = toTarget / dist;

        // Linear falloff: full force at minDist, zero at maxDist
        const float falloff = 1.0f - (dist - m_fMinDistance) / (m_fMaxDistance - m_fMinDistance);
        const float acceleration = m_fForce * falloff * tDiff;

        // Decompose velocity: (dirX, dirY, dirZ, speed)
        const ezVec4 vel = itVelocity.Current();
        const ezVec3 dir(vel.x, vel.y, vel.z);
        const float speed = vel.w;

        const ezVec3 newVel = dir * speed + dirToTarget * acceleration;
        const float newSpeed = newVel.GetLength();
        const ezVec3 newDir = newSpeed > 0.0f ? newVel / newSpeed : ezVec3(0, 0, 1);

        itVelocity.Current() = ezVec4(newDir.x, newDir.y, newDir.z, newSpeed);
      }

      itPosition.Advance();
      itVelocity.Advance();
    }
  }
  else
  {
    // Direct position mode
    while (!itPosition.HasReachedEnd())
    {
      const ezSimdVec4f simPos = itPosition.Current();
      const ezVec3 pos(simPos.GetComponent<0>(), simPos.GetComponent<1>(), simPos.GetComponent<2>());

      const ezVec3 toTarget = vTarget - pos;
      const float distSqr = toTarget.GetLengthSquared();

      if (distSqr < fMaxDistSqr && distSqr > fMinDistSqr)
      {
        const float dist = ezMath::Sqrt(distSqr);
        const ezVec3 dirToTarget = toTarget / dist;

        const float falloff = 1.0f - (dist - m_fMinDistance) / (m_fMaxDistance - m_fMinDistance);
        const float moveAmount = m_fForce * falloff * tDiff;

        const ezVec3 newPos = pos + dirToTarget * ezMath::Min(moveAmount, dist - m_fMinDistance);

        itPosition.Current() = ezSimdConversion::ToVec3(newPos);
      }

      itPosition.Advance();
      itVelocity.Advance();
    }
  }
}


EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Behavior_ParticleBehavior_AttractToPosition);
