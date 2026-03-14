#include <EditorPluginParticle/EditorPluginParticlePCH.h>

#include <EditorPluginParticle/ParticleEffectAsset/ParticleEffectNodes.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleGraphPin, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleEffectNode, 1, ezRTTIDefaultAllocator<ezParticleEffectNode>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ENUM_MEMBER_PROPERTY("WhenInvisible", ezEffectInvisibleUpdateRate, m_InvisibleUpdateRate),
    EZ_MEMBER_PROPERTY("AlwaysShared", m_bAlwaysShared),
    EZ_MEMBER_PROPERTY("SimulateInLocalSpace", m_bSimulateInLocalSpace),
    EZ_MEMBER_PROPERTY("ApplyOwnerVelocity", m_fApplyInstanceVelocity)->AddAttributes(new ezClampValueAttribute(0.0f, 1.0f)),
    EZ_MEMBER_PROPERTY("PreSimulateDuration", m_PreSimulateDuration),
    EZ_MEMBER_PROPERTY("NumWindSamples", m_vNumWindSamples)->AddAttributes(new ezDefaultValueAttribute(ezVec3U32(1)), new ezClampValueAttribute(ezVec3U32(1), ezVec3U32(8))),
    EZ_MAP_MEMBER_PROPERTY("FloatParameters", m_FloatParameters),
    EZ_MAP_MEMBER_PROPERTY("ColorParameters", m_ColorParameters)->AddAttributes(new ezExposeColorAlphaAttribute),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Effect"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleSystemNode, 1, ezRTTIDefaultAllocator<ezParticleSystemNode>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Name", m_sName),
    EZ_MEMBER_PROPERTY("Visible", m_bVisible)->AddAttributes(new ezDefaultValueAttribute(true)),
    EZ_MEMBER_PROPERTY("LifeTime", m_LifeTime)->AddAttributes(new ezDefaultValueAttribute(ezVarianceTypeTime(ezTime::MakeFromSeconds(2))), new ezClampValueAttribute(ezTime::MakeFromSeconds(0.0), ezVariant())),
    EZ_MEMBER_PROPERTY("LifeScaleParam", m_sLifeScaleParameter),
    EZ_MEMBER_PROPERTY("OnDeathEvent", m_sOnDeathEvent)->AddAttributes(new ezDynamicStringEnumAttribute("ParticleEventNamesEnum")),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("System"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on
