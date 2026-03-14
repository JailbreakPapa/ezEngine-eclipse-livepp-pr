#include <EditorPluginParticle/EditorPluginParticlePCH.h>

#include <EditorPluginParticle/ParticleEffectAsset/ParticleEffectNodeManager.h>
#include <EditorPluginParticle/ParticleEffectAsset/ParticleEffectNodes.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <ParticlePlugin/Behavior/ParticleBehavior.h>
#include <ParticlePlugin/Emitter/ParticleEmitter.h>
#include <ParticlePlugin/Events/ParticleEventReaction.h>
#include <ParticlePlugin/Initializer/ParticleInitializer.h>
#include <ParticlePlugin/Type/ParticleType.h>

static ezParticleGraphPinCategory::Enum GetPinCategory(const ezRTTI* pType)
{
  if (pType->IsDerivedFrom<ezParticleEffectNode>())
    return ezParticleGraphPinCategory::System; // effect node's inputs are system-category
  if (pType->IsDerivedFrom<ezParticleSystemNode>())
    return ezParticleGraphPinCategory::System;
  if (pType->IsDerivedFrom<ezParticleEmitterFactory>())
    return ezParticleGraphPinCategory::Emitter;
  if (pType->IsDerivedFrom<ezParticleInitializerFactory>())
    return ezParticleGraphPinCategory::Initializer;
  if (pType->IsDerivedFrom<ezParticleBehaviorFactory>())
    return ezParticleGraphPinCategory::Behavior;
  if (pType->IsDerivedFrom<ezParticleTypeFactory>())
    return ezParticleGraphPinCategory::Renderer;
  if (pType->IsDerivedFrom<ezParticleEventReactionFactory>())
    return ezParticleGraphPinCategory::EventReaction;

  return ezParticleGraphPinCategory::System;
}

bool ezParticleEffectNodeManager::InternalIsNode(const ezDocumentObject* pObject) const
{
  auto pType = pObject->GetTypeAccessor().GetType();

  return pType->IsDerivedFrom<ezParticleEffectNode>() ||
         pType->IsDerivedFrom<ezParticleSystemNode>() ||
         pType->IsDerivedFrom<ezParticleEmitterFactory>() ||
         pType->IsDerivedFrom<ezParticleInitializerFactory>() ||
         pType->IsDerivedFrom<ezParticleBehaviorFactory>() ||
         pType->IsDerivedFrom<ezParticleTypeFactory>() ||
         pType->IsDerivedFrom<ezParticleEventReactionFactory>();
}

void ezParticleEffectNodeManager::InternalCreatePins(const ezDocumentObject* pObject, NodeInternal& ref_node)
{
  auto pType = pObject->GetTypeAccessor().GetType();

  if (pType->IsDerivedFrom<ezParticleEffectNode>())
  {
    // Effect node has input pins for systems and event reactions
    {
      auto pPin = EZ_DEFAULT_NEW(ezParticleGraphPin, ezVisualGraphPin::Type::Input, "Systems", ezColorScheme::DarkUI(ezColorScheme::Blue), pObject);
      pPin->m_Category = ezParticleGraphPinCategory::System;
      ref_node.m_Inputs.PushBack(pPin);
    }
    {
      auto pPin = EZ_DEFAULT_NEW(ezParticleGraphPin, ezVisualGraphPin::Type::Input, "EventReactions", ezColorScheme::DarkUI(ezColorScheme::Red), pObject);
      pPin->m_Category = ezParticleGraphPinCategory::EventReaction;
      ref_node.m_Inputs.PushBack(pPin);
    }
  }
  else if (pType->IsDerivedFrom<ezParticleSystemNode>())
  {
    // System node: output connects to effect, inputs for each module category
    {
      auto pPin = EZ_DEFAULT_NEW(ezParticleGraphPin, ezVisualGraphPin::Type::Output, "System", ezColorScheme::DarkUI(ezColorScheme::Blue), pObject);
      pPin->m_Category = ezParticleGraphPinCategory::System;
      ref_node.m_Outputs.PushBack(pPin);
    }
    {
      auto pPin = EZ_DEFAULT_NEW(ezParticleGraphPin, ezVisualGraphPin::Type::Input, "Emitter", ezColorScheme::DarkUI(ezColorScheme::Yellow), pObject);
      pPin->m_Category = ezParticleGraphPinCategory::Emitter;
      ref_node.m_Inputs.PushBack(pPin);
    }
    {
      auto pPin = EZ_DEFAULT_NEW(ezParticleGraphPin, ezVisualGraphPin::Type::Input, "Initializers", ezColorScheme::DarkUI(ezColorScheme::Green), pObject);
      pPin->m_Category = ezParticleGraphPinCategory::Initializer;
      ref_node.m_Inputs.PushBack(pPin);
    }
    {
      auto pPin = EZ_DEFAULT_NEW(ezParticleGraphPin, ezVisualGraphPin::Type::Input, "Behaviors", ezColorScheme::DarkUI(ezColorScheme::Orange), pObject);
      pPin->m_Category = ezParticleGraphPinCategory::Behavior;
      ref_node.m_Inputs.PushBack(pPin);
    }
    {
      auto pPin = EZ_DEFAULT_NEW(ezParticleGraphPin, ezVisualGraphPin::Type::Input, "Renderers", ezColorScheme::DarkUI(ezColorScheme::Grape), pObject);
      pPin->m_Category = ezParticleGraphPinCategory::Renderer;
      ref_node.m_Inputs.PushBack(pPin);
    }
  }
  else if (pType->IsDerivedFrom<ezParticleEmitterFactory>())
  {
    auto pPin = EZ_DEFAULT_NEW(ezParticleGraphPin, ezVisualGraphPin::Type::Output, "Emitter", ezColorScheme::DarkUI(ezColorScheme::Yellow), pObject);
    pPin->m_Category = ezParticleGraphPinCategory::Emitter;
    ref_node.m_Outputs.PushBack(pPin);
  }
  else if (pType->IsDerivedFrom<ezParticleInitializerFactory>())
  {
    auto pPin = EZ_DEFAULT_NEW(ezParticleGraphPin, ezVisualGraphPin::Type::Output, "Initializer", ezColorScheme::DarkUI(ezColorScheme::Green), pObject);
    pPin->m_Category = ezParticleGraphPinCategory::Initializer;
    ref_node.m_Outputs.PushBack(pPin);
  }
  else if (pType->IsDerivedFrom<ezParticleBehaviorFactory>())
  {
    auto pPin = EZ_DEFAULT_NEW(ezParticleGraphPin, ezVisualGraphPin::Type::Output, "Behavior", ezColorScheme::DarkUI(ezColorScheme::Orange), pObject);
    pPin->m_Category = ezParticleGraphPinCategory::Behavior;
    ref_node.m_Outputs.PushBack(pPin);
  }
  else if (pType->IsDerivedFrom<ezParticleTypeFactory>())
  {
    auto pPin = EZ_DEFAULT_NEW(ezParticleGraphPin, ezVisualGraphPin::Type::Output, "Renderer", ezColorScheme::DarkUI(ezColorScheme::Grape), pObject);
    pPin->m_Category = ezParticleGraphPinCategory::Renderer;
    ref_node.m_Outputs.PushBack(pPin);
  }
  else if (pType->IsDerivedFrom<ezParticleEventReactionFactory>())
  {
    auto pPin = EZ_DEFAULT_NEW(ezParticleGraphPin, ezVisualGraphPin::Type::Output, "EventReaction", ezColorScheme::DarkUI(ezColorScheme::Red), pObject);
    pPin->m_Category = ezParticleGraphPinCategory::EventReaction;
    ref_node.m_Outputs.PushBack(pPin);
  }
}

void ezParticleEffectNodeManager::GetCreateableTypes(ezHybridArray<const ezRTTI*, 32>& ref_types) const
{
  ref_types.Clear();

  // Add system node
  ref_types.PushBack(ezGetStaticRTTI<ezParticleSystemNode>());

  // Gather all concrete factory types
  ezSet<const ezRTTI*> typeSet;

  ezReflectionUtils::GatherTypesDerivedFromClass(ezGetStaticRTTI<ezParticleEmitterFactory>(), typeSet);
  ezReflectionUtils::GatherTypesDerivedFromClass(ezGetStaticRTTI<ezParticleInitializerFactory>(), typeSet);
  ezReflectionUtils::GatherTypesDerivedFromClass(ezGetStaticRTTI<ezParticleBehaviorFactory>(), typeSet);
  ezReflectionUtils::GatherTypesDerivedFromClass(ezGetStaticRTTI<ezParticleTypeFactory>(), typeSet);
  ezReflectionUtils::GatherTypesDerivedFromClass(ezGetStaticRTTI<ezParticleEventReactionFactory>(), typeSet);

  for (auto pType : typeSet)
  {
    if (pType->GetTypeFlags().IsAnySet(ezTypeFlags::Abstract))
      continue;

    ref_types.PushBack(pType);
  }

  // Note: ezParticleEffectNode is NOT in this list — it is auto-created by the document.
}

ezStatus ezParticleEffectNodeManager::InternalCanConnect(const ezVisualGraphPin& source, const ezVisualGraphPin& target, CanConnectResult& out_result) const
{
  const auto& srcPin = static_cast<const ezParticleGraphPin&>(source);
  const auto& tgtPin = static_cast<const ezParticleGraphPin&>(target);

  // Only allow connections between matching categories
  if (srcPin.m_Category != tgtPin.m_Category)
  {
    return ezStatus("Pin categories do not match.");
  }

  // Emitter is 1:1 (only one emitter per system)
  if (srcPin.m_Category == ezParticleGraphPinCategory::Emitter)
  {
    out_result = CanConnectResult::Connect1to1;
  }
  else
  {
    out_result = CanConnectResult::ConnectNto1;
  }

  return ezStatus(EZ_SUCCESS);
}
