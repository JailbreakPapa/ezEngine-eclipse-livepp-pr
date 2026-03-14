#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/VarianceTypes.h>
#include <ParticlePlugin/Declarations.h>
#include <ToolsFoundation/VisualGraph/VisualGraphObjectManager.h>

struct ezParticleGraphPinCategory
{
  using StorageType = ezUInt8;

  enum Enum
  {
    System,
    Emitter,
    Initializer,
    Behavior,
    Renderer,
    EventReaction,

    Default = System
  };
};

/// Pin type for the particle effect visual graph.
///
/// Carries a category that determines which pin types can connect to each other.
class ezParticleGraphPin : public ezVisualGraphPin
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleGraphPin, ezVisualGraphPin);

public:
  using ezVisualGraphPin::ezVisualGraphPin;

  ezEnum<ezParticleGraphPinCategory> m_Category;
};

/// Root node of a particle effect graph. One per document.
///
/// Holds effect-level settings. Systems and event reactions connect to this node's input pins.
class ezParticleEffectNode : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleEffectNode, ezReflectedClass);

public:
  ezEnum<ezEffectInvisibleUpdateRate> m_InvisibleUpdateRate;
  bool m_bAlwaysShared = false;
  bool m_bSimulateInLocalSpace = false;
  float m_fApplyInstanceVelocity = 0.0f;
  ezTime m_PreSimulateDuration;
  ezVec3U32 m_vNumWindSamples = ezVec3U32(1);
  ezMap<ezString, float> m_FloatParameters;
  ezMap<ezString, ezColor> m_ColorParameters;
};

/// Represents a particle system/layer within the effect graph.
///
/// Each system node has input pins for its emitter, initializers, behaviors, and renderers,
/// and an output pin that connects to the effect node.
class ezParticleSystemNode : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleSystemNode, ezReflectedClass);

public:
  ezString m_sName;
  bool m_bVisible = true;
  ezVarianceTypeTime m_LifeTime;
  ezString m_sOnDeathEvent;
  ezString m_sLifeScaleParameter;
};
