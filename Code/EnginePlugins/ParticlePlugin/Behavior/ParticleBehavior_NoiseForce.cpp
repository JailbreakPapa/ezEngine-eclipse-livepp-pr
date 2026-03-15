#include <ParticlePlugin/ParticlePluginPCH.h>

#include <Foundation/DataProcessing/Stream/ProcessingStreamIterator.h>
#include <Foundation/Profiling/Profiling.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_NoiseForce.h>
#include <ParticlePlugin/Effect/ParticleEffectInstance.h>
#include <ParticlePlugin/Finalizer/ParticleFinalizer_ApplyVelocity.h>
#include <ParticlePlugin/System/ParticleSystemInstance.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehaviorFactory_NoiseForce, 1, ezRTTIDefaultAllocator<ezParticleBehaviorFactory_NoiseForce>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Strength", m_fStrength)->AddAttributes(new ezDefaultValueAttribute(2.0f)),
    EZ_MEMBER_PROPERTY("Frequency", m_fFrequency)->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.01f, {})),
    EZ_MEMBER_PROPERTY("ScrollSpeed", m_vScrollSpeed),
    EZ_MEMBER_PROPERTY("Octaves", m_uiOctaves)->AddAttributes(new ezDefaultValueAttribute(1), new ezClampValueAttribute(1, 4)),
    EZ_MEMBER_PROPERTY("AffectVelocity", m_bAffectVelocity)->AddAttributes(new ezDefaultValueAttribute(true)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehavior_NoiseForce, 1, ezRTTIDefaultAllocator<ezParticleBehavior_NoiseForce>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezParticleBehaviorFactory_NoiseForce::ezParticleBehaviorFactory_NoiseForce() = default;

const ezRTTI* ezParticleBehaviorFactory_NoiseForce::GetBehaviorType() const
{
  return ezGetStaticRTTI<ezParticleBehavior_NoiseForce>();
}

void ezParticleBehaviorFactory_NoiseForce::CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const
{
  ezParticleBehavior_NoiseForce* pBehavior = static_cast<ezParticleBehavior_NoiseForce*>(pObject);

  pBehavior->m_fStrength = m_fStrength;
  pBehavior->m_fFrequency = m_fFrequency;
  pBehavior->m_vScrollSpeed = m_vScrollSpeed;
  pBehavior->m_uiOctaves = m_uiOctaves;
  pBehavior->m_bAffectVelocity = m_bAffectVelocity;
}

void ezParticleBehaviorFactory_NoiseForce::QueryFinalizerDependencies(ezSet<const ezRTTI*>& inout_finalizerDeps) const
{
  if (m_bAffectVelocity)
  {
    inout_finalizerDeps.Insert(ezGetStaticRTTI<ezParticleFinalizerFactory_ApplyVelocity>());
  }
}

void ezParticleBehaviorFactory_NoiseForce::Save(ezStreamWriter& inout_stream) const
{
  const ezUInt8 uiVersion = 1;
  inout_stream << uiVersion;

  inout_stream << m_fStrength;
  inout_stream << m_fFrequency;
  inout_stream << m_vScrollSpeed;
  inout_stream << m_uiOctaves;
  inout_stream << m_bAffectVelocity;
}

void ezParticleBehaviorFactory_NoiseForce::Load(ezStreamReader& inout_stream)
{
  ezUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  EZ_ASSERT_DEV(uiVersion <= 1, "Invalid version {0}", uiVersion);

  inout_stream >> m_fStrength;
  inout_stream >> m_fFrequency;
  inout_stream >> m_vScrollSpeed;
  inout_stream >> m_uiOctaves;
  inout_stream >> m_bAffectVelocity;
}

void ezParticleBehavior_NoiseForce::CreateRequiredStreams()
{
  CreateStream("Position", ezProcessingStream::DataType::Float4, &m_pStreamPosition, false);
  CreateStream("Velocity", ezProcessingStream::DataType::Half4, &m_pStreamVelocity, false);
}

void ezParticleBehavior_NoiseForce::Process(ezUInt64 uiNumElements)
{
  EZ_PROFILE_SCOPE("PFX: NoiseForce");

  const float tDiff = (float)m_TimeDiff.GetSeconds();
  if (tDiff <= 0.0f)
    return;

  m_fTotalTime += tDiff;

  const float freq = m_fFrequency;
  const ezVec3 scroll = m_vScrollSpeed * m_fTotalTime;

  // Small offsets for curl noise approximation: sample noise at 3 different offsets
  // to get decorrelated X, Y, Z force components
  const float kOffset1 = 31.416f;
  const float kOffset2 = 67.123f;

  ezProcessingStreamIterator<ezSimdVec4f> itPosition(m_pStreamPosition, uiNumElements, 0);
  ezProcessingStreamIterator<ezFloat16Vec4> itVelocity(m_pStreamVelocity, uiNumElements, 0);

  if (m_bAffectVelocity)
  {
    while (!itPosition.HasReachedEnd())
    {
      const ezSimdVec4f simPos = itPosition.Current();
      const float px = simPos.GetComponent<0>();
      const float py = simPos.GetComponent<1>();
      const float pz = simPos.GetComponent<2>();

      // Noise sampling coordinates with scroll and frequency
      const float sx = (px + scroll.x) * freq;
      const float sy = (py + scroll.y) * freq;
      const float sz = (pz + scroll.z) * freq;

      // Sample noise at 3 offset positions for curl-noise approximation
      // Each component uses a different spatial offset to get decorrelated values
      const ezSimdVec4f noiseX = m_Noise.NoiseZeroToOne(
        ezSimdVec4f(sx), ezSimdVec4f(sy), ezSimdVec4f(sz), m_uiOctaves);
      const ezSimdVec4f noiseY = m_Noise.NoiseZeroToOne(
        ezSimdVec4f(sx + kOffset1), ezSimdVec4f(sy + kOffset1), ezSimdVec4f(sz + kOffset1), m_uiOctaves);
      const ezSimdVec4f noiseZ = m_Noise.NoiseZeroToOne(
        ezSimdVec4f(sx + kOffset2), ezSimdVec4f(sy + kOffset2), ezSimdVec4f(sz + kOffset2), m_uiOctaves);

      // Map [0,1] -> [-1,1]
      const float forceX = (static_cast<float>(noiseX.GetComponent<0>()) * 2.0f - 1.0f) * m_fStrength * tDiff;
      const float forceY = (static_cast<float>(noiseY.GetComponent<0>()) * 2.0f - 1.0f) * m_fStrength * tDiff;
      const float forceZ = (static_cast<float>(noiseZ.GetComponent<0>()) * 2.0f - 1.0f) * m_fStrength * tDiff;

      // Decompose velocity: (dirX, dirY, dirZ, speed)
      const ezVec4 vel = itVelocity.Current();
      const ezVec3 dir(vel.x, vel.y, vel.z);
      const float speed = vel.w;

      const ezVec3 newVel = dir * speed + ezVec3(forceX, forceY, forceZ);
      const float newSpeed = newVel.GetLength();
      const ezVec3 newDir = newSpeed > 0.0f ? newVel / newSpeed : ezVec3(0, 0, 1);

      itVelocity.Current() = ezVec4(newDir.x, newDir.y, newDir.z, newSpeed);

      itPosition.Advance();
      itVelocity.Advance();
    }
  }
  else
  {
    // Direct position offset mode
    while (!itPosition.HasReachedEnd())
    {
      const ezSimdVec4f simPos = itPosition.Current();
      const float px = simPos.GetComponent<0>();
      const float py = simPos.GetComponent<1>();
      const float pz = simPos.GetComponent<2>();

      const float sx = (px + scroll.x) * freq;
      const float sy = (py + scroll.y) * freq;
      const float sz = (pz + scroll.z) * freq;

      const ezSimdVec4f noiseX = m_Noise.NoiseZeroToOne(
        ezSimdVec4f(sx), ezSimdVec4f(sy), ezSimdVec4f(sz), m_uiOctaves);
      const ezSimdVec4f noiseY = m_Noise.NoiseZeroToOne(
        ezSimdVec4f(sx + kOffset1), ezSimdVec4f(sy + kOffset1), ezSimdVec4f(sz + kOffset1), m_uiOctaves);
      const ezSimdVec4f noiseZ = m_Noise.NoiseZeroToOne(
        ezSimdVec4f(sx + kOffset2), ezSimdVec4f(sy + kOffset2), ezSimdVec4f(sz + kOffset2), m_uiOctaves);

      const float offsetX = (static_cast<float>(noiseX.GetComponent<0>()) * 2.0f - 1.0f) * m_fStrength * tDiff;
      const float offsetY = (static_cast<float>(noiseY.GetComponent<0>()) * 2.0f - 1.0f) * m_fStrength * tDiff;
      const float offsetZ = (static_cast<float>(noiseZ.GetComponent<0>()) * 2.0f - 1.0f) * m_fStrength * tDiff;

      itPosition.Current() = simPos + ezSimdVec4f(offsetX, offsetY, offsetZ, 0.0f);

      itPosition.Advance();
      itVelocity.Advance();
    }
  }
}


EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Behavior_ParticleBehavior_NoiseForce);
