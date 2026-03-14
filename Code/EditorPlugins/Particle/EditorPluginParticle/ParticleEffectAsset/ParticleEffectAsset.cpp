#include <EditorPluginParticle/EditorPluginParticlePCH.h>

#include <EditorFramework/GUI/ExposedParameters.h>
#include <EditorPluginParticle/ParticleEffectAsset/ParticleEffectAsset.h>
#include <EditorPluginParticle/ParticleEffectAsset/ParticleEffectNodeManager.h>
#include <EditorPluginParticle/ParticleEffectAsset/ParticleEffectNodes.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <GuiFoundation/PropertyGrid/PropertyMetaState.h>
#include <GuiFoundation/PropertyGrid/VisualizerManager.h>
#include <GuiFoundation/VisualGraph/Scene.moc.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_ColorGradient.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_Move.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_Opacity.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_SizeCurve.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_Velocity.h>
#include <ParticlePlugin/Emitter/ParticleEmitter_Continuous.h>
#include <ParticlePlugin/Events/ParticleEventReaction.h>
#include <ParticlePlugin/Initializer/ParticleInitializer_CylinderPosition.h>
#include <ParticlePlugin/Initializer/ParticleInitializer_RandomColor.h>
#include <ParticlePlugin/Initializer/ParticleInitializer_SpherePosition.h>
#include <ParticlePlugin/System/ParticleSystemDescriptor.h>
#include <ParticlePlugin/Type/Quad/ParticleTypeQuad.h>
#include <ParticlePlugin/Type/Trail/ParticleTypeTrail.h>
#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/Command/VisualGraphCommands.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleEffectAssetDocument, 8, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezParticleEffectAssetDocument::ezParticleEffectAssetDocument(ezStringView sDocumentPath)
  : ezAssetDocument(sDocumentPath, EZ_DEFAULT_NEW(ezParticleEffectNodeManager), ezAssetDocEngineConnection::Simple)
  , m_LightSettings(false)
{
  ezVisualizerManager::GetSingleton()->SetVisualizersActive(this, m_bRenderVisualizers);
}

ezParticleEffectAssetDocument::~ezParticleEffectAssetDocument() = default;

void ezParticleEffectAssetDocument::PropertyMetaStateEventHandler(ezPropertyMetaStateEvent& e)
{
  if (e.m_pObject->GetTypeAccessor().GetType() == ezGetStaticRTTI<ezParticleEffectNode>())
  {
    auto& props = *e.m_pPropertyStates;

    bool bShared = e.m_pObject->GetTypeAccessor().GetValue("AlwaysShared").ConvertTo<bool>();

    props["SimulateInLocalSpace"].m_Visibility = bShared ? ezPropertyUiState::Disabled : ezPropertyUiState::Default;
    props["ApplyOwnerVelocity"].m_Visibility = bShared ? ezPropertyUiState::Disabled : ezPropertyUiState::Default;
  }
  else if (e.m_pObject->GetTypeAccessor().GetType() == ezGetStaticRTTI<ezParticleTypeQuadFactory>())
  {
    auto& props = *e.m_pPropertyStates;

    bool useMaterial = e.m_pObject->GetTypeAccessor().GetValue("UseCustomMaterial").ConvertTo<bool>();
    ezInt64 orientation = e.m_pObject->GetTypeAccessor().GetValue("Orientation").ConvertTo<ezInt64>();
    ezInt64 renderMode = e.m_pObject->GetTypeAccessor().GetValue("RenderMode").ConvertTo<ezInt64>();
    ezInt64 lightingMode = e.m_pObject->GetTypeAccessor().GetValue("LightingMode").ConvertTo<ezInt64>();
    ezInt64 textureAtlas = e.m_pObject->GetTypeAccessor().GetValue("TextureAtlas").ConvertTo<ezInt64>();

    props["Deviation"].m_Visibility = ezPropertyUiState::Invisible;
    props["DistortionTexture"].m_Visibility = ezPropertyUiState::Invisible;
    props["DistortionStrength"].m_Visibility = ezPropertyUiState::Invisible;
    props["ParticleStretch"].m_Visibility =
      (orientation == ezQuadParticleOrientation::FixedAxis_EmitterDir || orientation == ezQuadParticleOrientation::FixedAxis_ParticleDir)
        ? ezPropertyUiState::Default
        : ezPropertyUiState::Invisible;
    props["NumSpritesX"].m_Visibility = (textureAtlas == (int)ezParticleTextureAtlasType::None) ? ezPropertyUiState::Invisible : ezPropertyUiState::Default;
    props["NumSpritesY"].m_Visibility = (textureAtlas == (int)ezParticleTextureAtlasType::None) ? ezPropertyUiState::Invisible : ezPropertyUiState::Default;
    props["NormalCurvature"].m_Visibility = ezPropertyUiState::Invisible;
    props["LightDirectionality"].m_Visibility = ezPropertyUiState::Invisible;
    props["Texture"].m_Visibility = useMaterial ? ezPropertyUiState::Invisible : ezPropertyUiState::Default;
    props["CustomMaterial"].m_Visibility = useMaterial ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;

    if (orientation == ezQuadParticleOrientation::Fixed_EmitterDir || orientation == ezQuadParticleOrientation::Fixed_WorldUp)
    {
      props["Deviation"].m_Visibility = ezPropertyUiState::Default;
    }

    if (lightingMode == ezParticleLightingMode::VertexLit)
    {
      props["NormalCurvature"].m_Visibility = ezPropertyUiState::Default;
      props["LightDirectionality"].m_Visibility = ezPropertyUiState::Default;
    }
  }
  else if (e.m_pObject->GetTypeAccessor().GetType() == ezGetStaticRTTI<ezParticleTypeTrailFactory>())
  {
    auto& props = *e.m_pPropertyStates;

    bool useMaterial = e.m_pObject->GetTypeAccessor().GetValue("UseCustomMaterial").ConvertTo<bool>();
    ezInt64 renderMode = e.m_pObject->GetTypeAccessor().GetValue("RenderMode").ConvertTo<ezInt64>();
    ezInt64 lightingMode = e.m_pObject->GetTypeAccessor().GetValue("LightingMode").ConvertTo<ezInt64>();
    ezInt64 textureAtlas = e.m_pObject->GetTypeAccessor().GetValue("TextureAtlas").ConvertTo<ezInt64>();

    props["DistortionTexture"].m_Visibility = ezPropertyUiState::Invisible;
    props["DistortionStrength"].m_Visibility = ezPropertyUiState::Invisible;
    props["NumSpritesX"].m_Visibility =
      (textureAtlas == (int)ezParticleTextureAtlasType::None) ? ezPropertyUiState::Invisible : ezPropertyUiState::Default;
    props["NumSpritesY"].m_Visibility =
      (textureAtlas == (int)ezParticleTextureAtlasType::None) ? ezPropertyUiState::Invisible : ezPropertyUiState::Default;
    props["NormalCurvature"].m_Visibility = ezPropertyUiState::Invisible;
    props["LightDirectionality"].m_Visibility = ezPropertyUiState::Invisible;
    props["Texture"].m_Visibility = useMaterial ? ezPropertyUiState::Invisible : ezPropertyUiState::Default;
    props["CustomMaterial"].m_Visibility = useMaterial ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;

    if (lightingMode == ezParticleLightingMode::VertexLit)
    {
      props["NormalCurvature"].m_Visibility = ezPropertyUiState::Default;
      props["LightDirectionality"].m_Visibility = ezPropertyUiState::Default;
    }
  }
  else if (e.m_pObject->GetTypeAccessor().GetType() == ezGetStaticRTTI<ezParticleBehaviorFactory_ColorGradient>())
  {
    auto& props = *e.m_pPropertyStates;

    ezInt64 gradientSource = e.m_pObject->GetTypeAccessor().GetValue("GradientSource").ConvertTo<ezInt64>();
    ezInt64 mode = e.m_pObject->GetTypeAccessor().GetValue("ColorGradientMode").ConvertTo<ezInt64>();

    props["Gradient"].m_Visibility = (gradientSource == ezGradientSource::CustomGradient) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["SharedGradient"].m_Visibility = (gradientSource == ezGradientSource::SharedGradient) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["GradientMaxSpeed"].m_Visibility = (mode == ezParticleColorGradientMode::Speed) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
  }
  else if (e.m_pObject->GetTypeAccessor().GetType() == ezGetStaticRTTI<ezParticleBehaviorFactory_Opacity>())
  {
    auto& props = *e.m_pPropertyStates;

    ezInt64 curveSource = e.m_pObject->GetTypeAccessor().GetValue("ChangeOpacityWith").ConvertTo<ezInt64>();

    props["OpacityCurve"].m_Visibility = (curveSource == ezCurveSource::CustomCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["SharedOpacityCurve"].m_Visibility = (curveSource == ezCurveSource::SharedCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
  }
  else if (e.m_pObject->GetTypeAccessor().GetType() == ezGetStaticRTTI<ezParticleBehaviorFactory_SizeCurve>())
  {
    auto& props = *e.m_pPropertyStates;

    ezInt64 curveSource = e.m_pObject->GetTypeAccessor().GetValue("ChangeSizeWith").ConvertTo<ezInt64>();

    props["SizeCurve"].m_Visibility = (curveSource == ezCurveSource::CustomCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["SharedSizeCurve"].m_Visibility = (curveSource == ezCurveSource::SharedCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
  }
  else if (e.m_pObject->GetTypeAccessor().GetType() == ezGetStaticRTTI<ezParticleBehaviorFactory_Velocity>())
  {
    auto& props = *e.m_pPropertyStates;

    ezInt64 changeSpeedWith = e.m_pObject->GetTypeAccessor().GetValue("ChangeSpeedWith").ConvertTo<ezInt64>();

    props["Friction"].m_Visibility = (changeSpeedWith == ezVelocityChangeMode::Friction) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["SpeedCurve"].m_Visibility = (changeSpeedWith == ezVelocityChangeMode::CustomCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["SharedSpeedCurve"].m_Visibility = (changeSpeedWith == ezVelocityChangeMode::SharedCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["SpeedCurveOffset"].m_Visibility = (changeSpeedWith == ezVelocityChangeMode::CustomCurve || changeSpeedWith == ezVelocityChangeMode::SharedCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["SpeedCurveScale"].m_Visibility = (changeSpeedWith == ezVelocityChangeMode::CustomCurve || changeSpeedWith == ezVelocityChangeMode::SharedCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
  }
  else if (e.m_pObject->GetTypeAccessor().GetType() == ezGetStaticRTTI<ezParticleBehaviorFactory_Move>())
  {
    auto& props = *e.m_pPropertyStates;

    ezInt64 moveX_Mode = e.m_pObject->GetTypeAccessor().GetValue("MoveX_Mode").ConvertTo<ezInt64>();
    ezInt64 moveY_Mode = e.m_pObject->GetTypeAccessor().GetValue("MoveY_Mode").ConvertTo<ezInt64>();
    ezInt64 moveZ_Mode = e.m_pObject->GetTypeAccessor().GetValue("MoveZ_Mode").ConvertTo<ezInt64>();

    props["MoveX_Speed"].m_Visibility = (moveX_Mode == ezMovementMode::Constant) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["MoveX_Curve"].m_Visibility = (moveX_Mode == ezMovementMode::CustomCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["MoveX_SharedCurve"].m_Visibility = (moveX_Mode == ezMovementMode::SharedCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["MoveX_CurveOffset"].m_Visibility = (moveX_Mode == ezMovementMode::CustomCurve || moveX_Mode == ezMovementMode::SharedCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["MoveX_CurveScale"].m_Visibility = (moveX_Mode == ezMovementMode::CustomCurve || moveX_Mode == ezMovementMode::SharedCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;

    props["MoveY_Speed"].m_Visibility = (moveY_Mode == ezMovementMode::Constant) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["MoveY_Curve"].m_Visibility = (moveY_Mode == ezMovementMode::CustomCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["MoveY_SharedCurve"].m_Visibility = (moveY_Mode == ezMovementMode::SharedCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["MoveY_CurveOffset"].m_Visibility = (moveY_Mode == ezMovementMode::CustomCurve || moveY_Mode == ezMovementMode::SharedCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["MoveY_CurveScale"].m_Visibility = (moveY_Mode == ezMovementMode::CustomCurve || moveY_Mode == ezMovementMode::SharedCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;

    props["MoveZ_Speed"].m_Visibility = (moveZ_Mode == ezMovementMode::Constant) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["MoveZ_Curve"].m_Visibility = (moveZ_Mode == ezMovementMode::CustomCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["MoveZ_SharedCurve"].m_Visibility = (moveZ_Mode == ezMovementMode::SharedCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["MoveZ_CurveOffset"].m_Visibility = (moveZ_Mode == ezMovementMode::CustomCurve || moveZ_Mode == ezMovementMode::SharedCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["MoveZ_CurveScale"].m_Visibility = (moveZ_Mode == ezMovementMode::CustomCurve || moveZ_Mode == ezMovementMode::SharedCurve) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
  }
  else if (e.m_pObject->GetTypeAccessor().GetType() == ezGetStaticRTTI<ezParticleInitializerFactory_CylinderPosition>())
  {
    auto& props = *e.m_pPropertyStates;

    bool bSetVelocity = e.m_pObject->GetTypeAccessor().GetValue("SetVelocity").ConvertTo<bool>();

    props["Speed"].m_Visibility = bSetVelocity ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
  }
  else if (e.m_pObject->GetTypeAccessor().GetType() == ezGetStaticRTTI<ezParticleInitializerFactory_SpherePosition>())
  {
    auto& props = *e.m_pPropertyStates;

    bool bSetVelocity = e.m_pObject->GetTypeAccessor().GetValue("SetVelocity").ConvertTo<bool>();

    props["Speed"].m_Visibility = bSetVelocity ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
  }
  else if (e.m_pObject->GetTypeAccessor().GetType() == ezGetStaticRTTI<ezParticleInitializerFactory_RandomColor>())
  {
    auto& props = *e.m_pPropertyStates;

    ezInt64 gradientSource = e.m_pObject->GetTypeAccessor().GetValue("GradientSource").ConvertTo<ezInt64>();

    props["Gradient"].m_Visibility = (gradientSource == ezGradientSource::CustomGradient) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
    props["SharedGradient"].m_Visibility = (gradientSource == ezGradientSource::SharedGradient) ? ezPropertyUiState::Default : ezPropertyUiState::Invisible;
  }
}

//////////////////////////////////////////////////////////////////////////

const ezDocumentObject* ezParticleEffectAssetDocument::FindEffectNode() const
{
  const auto* pManager = static_cast<const ezVisualGraphObjectManager*>(GetObjectManager());
  const ezDocumentObject* pRoot = GetObjectManager()->GetRootObject();

  for (const ezDocumentObject* pChild : pRoot->GetChildren())
  {
    if (pChild->GetTypeAccessor().GetType()->IsDerivedFrom<ezParticleEffectNode>())
      return pChild;
  }

  return nullptr;
}

void ezParticleEffectAssetDocument::CollectConnectedNodes(const ezDocumentObject* pNode, ezStringView sPinName, ezDynamicArray<const ezDocumentObject*>& out_nodes) const
{
  out_nodes.Clear();

  const auto* pManager = static_cast<const ezVisualGraphObjectManager*>(GetObjectManager());

  const ezVisualGraphPin* pPin = pManager->GetInputPinByName(pNode, sPinName);
  if (pPin == nullptr)
    return;

  auto connections = pManager->GetConnections(*pPin);
  for (const ezVisualGraphConnection* pConn : connections)
  {
    const ezVisualGraphPin& otherPin = (&pConn->GetSourcePin() == pPin) ? pConn->GetTargetPin() : pConn->GetSourcePin();
    out_nodes.PushBack(otherPin.GetParent());
  }
}

static void MirrorMemberProperties(const ezDocumentObject* pDocObj, ezReflectedClass* pNativeObj)
{
  const ezRTTI* pType = pDocObj->GetTypeAccessor().GetType();

  ezHybridArray<const ezAbstractProperty*, 32> properties;
  pType->GetAllProperties(properties);

  for (auto pProp : properties)
  {
    if (pProp->GetCategory() != ezPropertyCategory::Member)
      continue;

    if (pProp->GetFlags().IsSet(ezPropertyFlags::ReadOnly))
      continue;

    ezVariant value = pDocObj->GetTypeAccessor().GetValue(pProp->GetPropertyName());
    if (value.IsValid())
    {
      ezReflectionUtils::SetMemberPropertyValue(static_cast<const ezAbstractMemberProperty*>(pProp), pNativeObj, value);
    }
  }
}

void ezParticleEffectAssetDocument::BuildDescriptorFromGraph(ezParticleEffectDescriptor& out_desc) const
{
  out_desc.ClearSystems();
  out_desc.ClearEventReactions();

  const ezDocumentObject* pEffectNode = FindEffectNode();
  if (pEffectNode == nullptr)
    return;

  // Copy effect-level properties from the effect node
  {
    const auto& acc = pEffectNode->GetTypeAccessor();
    out_desc.m_InvisibleUpdateRate = (ezEffectInvisibleUpdateRate::Enum)acc.GetValue("WhenInvisible").ConvertTo<ezInt64>();
    out_desc.m_bAlwaysShared = acc.GetValue("AlwaysShared").ConvertTo<bool>();
    out_desc.m_bSimulateInLocalSpace = acc.GetValue("SimulateInLocalSpace").ConvertTo<bool>();
    out_desc.m_fApplyInstanceVelocity = acc.GetValue("ApplyOwnerVelocity").ConvertTo<float>();
    out_desc.m_PreSimulateDuration = acc.GetValue("PreSimulateDuration").ConvertTo<ezTime>();
    out_desc.m_vNumWindSamples = acc.GetValue("NumWindSamples").Get<ezVec3U32>();

    // Copy map properties
    out_desc.m_FloatParameters.Clear();
    out_desc.m_ColorParameters.Clear();

    const ezRTTI* pEffectNodeType = ezGetStaticRTTI<ezParticleEffectNode>();
    ezHybridArray<const ezAbstractProperty*, 32> allProps;
    pEffectNodeType->GetAllProperties(allProps);

    for (auto pProp : allProps)
    {
      if (pProp->GetCategory() == ezPropertyCategory::Map)
      {
        auto* pMapProp = static_cast<const ezAbstractMapProperty*>(pProp);

        if (ezStringUtils::IsEqual(pProp->GetPropertyName(), "FloatParameters"))
        {
          ezDynamicArray<ezVariant> keys;
          acc.GetKeys(pProp->GetPropertyName(), keys);
          for (auto& key : keys)
          {
            ezString sKey = key.ConvertTo<ezString>();
            ezVariant val = acc.GetValue(pProp->GetPropertyName(), key);
            out_desc.m_FloatParameters[sKey] = val.ConvertTo<float>();
          }
        }
        else if (ezStringUtils::IsEqual(pProp->GetPropertyName(), "ColorParameters"))
        {
          ezDynamicArray<ezVariant> keys;
          acc.GetKeys(pProp->GetPropertyName(), keys);
          for (auto& key : keys)
          {
            ezString sKey = key.ConvertTo<ezString>();
            ezVariant val = acc.GetValue(pProp->GetPropertyName(), key);
            out_desc.m_ColorParameters[sKey] = val.Get<ezColor>();
          }
        }
      }
    }
  }

  // Collect system nodes connected to the effect node
  ezDynamicArray<const ezDocumentObject*> systemNodes;
  CollectConnectedNodes(pEffectNode, "Systems", systemNodes);

  for (const ezDocumentObject* pSystemNode : systemNodes)
  {
    ezParticleSystemDescriptor* pSysDesc = EZ_DEFAULT_NEW(ezParticleSystemDescriptor);
    out_desc.AddParticleSystem(pSysDesc);

    // Copy system-level properties
    const auto& sysAcc = pSystemNode->GetTypeAccessor();
    pSysDesc->m_bVisible = sysAcc.GetValue("Visible").ConvertTo<bool>();
    pSysDesc->m_LifeTime = sysAcc.GetValue("LifeTime").Get<ezVarianceTypeTime>();
    pSysDesc->m_sOnDeathEvent = sysAcc.GetValue("OnDeathEvent").ConvertTo<ezString>();
    pSysDesc->m_sLifeScaleParameter = sysAcc.GetValue("LifeScaleParam").ConvertTo<ezString>();

    // Emitter (at most 1)
    {
      ezDynamicArray<const ezDocumentObject*> emitterNodes;
      CollectConnectedNodes(pSystemNode, "Emitter", emitterNodes);

      for (const ezDocumentObject* pEmitterNode : emitterNodes)
      {
        const ezRTTI* pRtti = pEmitterNode->GetTypeAccessor().GetType();
        ezParticleEmitterFactory* pFactory = static_cast<ezParticleEmitterFactory*>(pRtti->GetAllocator()->Allocate<void>().m_pInstance);
        MirrorMemberProperties(pEmitterNode, pFactory);

        // m_EmitterFactories is private, so insert via RTTI array property
        const ezAbstractArrayProperty* pArrayProp = static_cast<const ezAbstractArrayProperty*>(
          ezGetStaticRTTI<ezParticleSystemDescriptor>()->FindPropertyByName("Emitters"));
        pArrayProp->Insert(pSysDesc, pArrayProp->GetCount(pSysDesc), &pFactory);
      }
    }

    // Initializers
    {
      ezDynamicArray<const ezDocumentObject*> initNodes;
      CollectConnectedNodes(pSystemNode, "Initializers", initNodes);

      for (const ezDocumentObject* pInitNode : initNodes)
      {
        const ezRTTI* pRtti = pInitNode->GetTypeAccessor().GetType();
        ezParticleInitializerFactory* pFactory = static_cast<ezParticleInitializerFactory*>(pRtti->GetAllocator()->Allocate<void>().m_pInstance);
        MirrorMemberProperties(pInitNode, pFactory);
        pSysDesc->AddInitializerFactory(pFactory);
      }
    }

    // Behaviors
    {
      ezDynamicArray<const ezDocumentObject*> behaviorNodes;
      CollectConnectedNodes(pSystemNode, "Behaviors", behaviorNodes);

      for (const ezDocumentObject* pBehaviorNode : behaviorNodes)
      {
        const ezRTTI* pRtti = pBehaviorNode->GetTypeAccessor().GetType();
        ezParticleBehaviorFactory* pFactory = static_cast<ezParticleBehaviorFactory*>(pRtti->GetAllocator()->Allocate<void>().m_pInstance);
        MirrorMemberProperties(pBehaviorNode, pFactory);
        pSysDesc->AddBehaviorFactory(pFactory);
      }
    }

    // Renderers/Types
    {
      ezDynamicArray<const ezDocumentObject*> typeNodes;
      CollectConnectedNodes(pSystemNode, "Renderers", typeNodes);

      for (const ezDocumentObject* pTypeNode : typeNodes)
      {
        const ezRTTI* pRtti = pTypeNode->GetTypeAccessor().GetType();
        ezParticleTypeFactory* pFactory = static_cast<ezParticleTypeFactory*>(pRtti->GetAllocator()->Allocate<void>().m_pInstance);
        MirrorMemberProperties(pTypeNode, pFactory);
        pSysDesc->AddTypeFactory(pFactory);
      }
    }
  }

  // Event reactions connected to the effect node
  {
    ezDynamicArray<const ezDocumentObject*> reactionNodes;
    CollectConnectedNodes(pEffectNode, "EventReactions", reactionNodes);

    for (const ezDocumentObject* pReactionNode : reactionNodes)
    {
      const ezRTTI* pRtti = pReactionNode->GetTypeAccessor().GetType();
      ezParticleEventReactionFactory* pFactory = static_cast<ezParticleEventReactionFactory*>(pRtti->GetAllocator()->Allocate<void>().m_pInstance);
      MirrorMemberProperties(pReactionNode, pFactory);
      out_desc.AddEventReaction(pFactory);
    }
  }
}

void ezParticleEffectAssetDocument::WriteResource(ezStreamWriter& inout_stream) const
{
  ezParticleEffectDescriptor desc;
  BuildDescriptorFromGraph(desc);
  desc.Save(inout_stream);
}

void ezParticleEffectAssetDocument::InitializeAfterLoading(bool bFirstTimeCreation)
{
  SUPER::InitializeAfterLoading(bFirstTimeCreation);

  AddSyncObject(&m_LightSettings);

  bool bNeedDefaultGraph = bFirstTimeCreation;

  if (!bFirstTimeCreation && FindEffectNode() == nullptr)
  {
    // Old-format document loaded (from ezSimpleAssetDocument era).
    // Remove legacy objects and create a fresh default graph.
    auto* pRoot = GetObjectManager()->GetRootObject();
    ezHybridArray<const ezDocumentObject*, 16> oldChildren;
    for (auto* pChild : pRoot->GetChildren())
      oldChildren.PushBack(pChild);

    if (!oldChildren.IsEmpty())
    {
      GetCommandHistory()->StartTransaction("Remove Legacy Objects");
      for (auto* pChild : oldChildren)
      {
        ezRemoveObjectCommand cmd;
        cmd.m_Object = pChild->GetGuid();
        GetCommandHistory()->AddCommand(cmd).IgnoreResult();
      }
      GetCommandHistory()->FinishTransaction();
    }

    bNeedDefaultGraph = true;
  }

  if (bNeedDefaultGraph)
  {
    auto* pHistory = GetCommandHistory();
    auto* pManager = static_cast<const ezVisualGraphObjectManager*>(GetObjectManager());
    const ezRTTI* pConnectionType = pManager->GetConnectionType();

    pHistory->StartTransaction("Init Particle Effect");

    // Create the effect node
    ezUuid effectGuid = ezUuid::MakeUuid();
    {
      ezAddObjectCommand cmd;
      cmd.m_pType = ezGetStaticRTTI<ezParticleEffectNode>();
      cmd.m_NewObjectGuid = effectGuid;
      cmd.m_Index = -1;
      pHistory->AddCommand(cmd).IgnoreResult();
    }
    {
      ezMoveNodeCommand cmd;
      cmd.m_Object = effectGuid;
      cmd.m_NewPos = ezVec2(400, 0);
      pHistory->AddCommand(cmd).IgnoreResult();
    }

    // Create a default system node
    ezUuid systemGuid = ezUuid::MakeUuid();
    {
      ezAddObjectCommand cmd;
      cmd.m_pType = ezGetStaticRTTI<ezParticleSystemNode>();
      cmd.m_NewObjectGuid = systemGuid;
      cmd.m_Index = -1;
      pHistory->AddCommand(cmd).IgnoreResult();
    }
    {
      ezSetObjectPropertyCommand cmd;
      cmd.m_Object = systemGuid;
      cmd.m_sProperty = "Name";
      cmd.m_NewValue = "Default";
      pHistory->AddCommand(cmd).IgnoreResult();
    }
    {
      ezMoveNodeCommand cmd;
      cmd.m_Object = systemGuid;
      cmd.m_NewPos = ezVec2(0, 0);
      pHistory->AddCommand(cmd).IgnoreResult();
    }

    // Connect system -> effect
    {
      const ezVisualGraphPin* pSourcePin = pManager->GetOutputPinByName(GetObjectManager()->GetObject(systemGuid), "System");
      const ezVisualGraphPin* pTargetPin = pManager->GetInputPinByName(GetObjectManager()->GetObject(effectGuid), "Systems");
      if (pSourcePin && pTargetPin)
        ezNodeCommands::AddAndConnectCommand(pHistory, pConnectionType, *pSourcePin, *pTargetPin).IgnoreResult();
    }

    // Create default emitter (Continuous)
    ezUuid emitterGuid = ezUuid::MakeUuid();
    {
      ezAddObjectCommand cmd;
      cmd.m_pType = ezGetStaticRTTI<ezParticleEmitterFactory_Continuous>();
      cmd.m_NewObjectGuid = emitterGuid;
      cmd.m_Index = -1;
      pHistory->AddCommand(cmd).IgnoreResult();
    }
    {
      ezMoveNodeCommand cmd;
      cmd.m_Object = emitterGuid;
      cmd.m_NewPos = ezVec2(-400, -100);
      pHistory->AddCommand(cmd).IgnoreResult();
    }
    {
      const ezVisualGraphPin* pSourcePin = pManager->GetOutputPinByName(GetObjectManager()->GetObject(emitterGuid), "Emitter");
      const ezVisualGraphPin* pTargetPin = pManager->GetInputPinByName(GetObjectManager()->GetObject(systemGuid), "Emitter");
      if (pSourcePin && pTargetPin)
        ezNodeCommands::AddAndConnectCommand(pHistory, pConnectionType, *pSourcePin, *pTargetPin).IgnoreResult();
    }

    // Create default Quad renderer
    ezUuid quadGuid = ezUuid::MakeUuid();
    {
      ezAddObjectCommand cmd;
      cmd.m_pType = ezGetStaticRTTI<ezParticleTypeQuadFactory>();
      cmd.m_NewObjectGuid = quadGuid;
      cmd.m_Index = -1;
      pHistory->AddCommand(cmd).IgnoreResult();
    }
    {
      ezMoveNodeCommand cmd;
      cmd.m_Object = quadGuid;
      cmd.m_NewPos = ezVec2(-400, 200);
      pHistory->AddCommand(cmd).IgnoreResult();
    }
    {
      const ezVisualGraphPin* pSourcePin = pManager->GetOutputPinByName(GetObjectManager()->GetObject(quadGuid), "Renderer");
      const ezVisualGraphPin* pTargetPin = pManager->GetInputPinByName(GetObjectManager()->GetObject(systemGuid), "Renderers");
      if (pSourcePin && pTargetPin)
        ezNodeCommands::AddAndConnectCommand(pHistory, pConnectionType, *pSourcePin, *pTargetPin).IgnoreResult();
    }

    pHistory->FinishTransaction();
  }
}

void ezParticleEffectAssetDocument::TriggerRestartEffect()
{
  ezParticleEffectAssetEvent e;
  e.m_pDocument = this;
  e.m_Type = ezParticleEffectAssetEvent::RestartEffect;
  m_Events.Broadcast(e);
}

void ezParticleEffectAssetDocument::SetAutoRestart(bool bEnable)
{
  if (m_bAutoRestart == bEnable)
    return;

  m_bAutoRestart = bEnable;

  ezParticleEffectAssetEvent e;
  e.m_pDocument = this;
  e.m_Type = ezParticleEffectAssetEvent::AutoRestartChanged;
  m_Events.Broadcast(e);
}

void ezParticleEffectAssetDocument::SetSimulationPaused(bool bPaused)
{
  if (m_bSimulationPaused == bPaused)
    return;

  m_bSimulationPaused = bPaused;

  ezParticleEffectAssetEvent e;
  e.m_pDocument = this;
  e.m_Type = ezParticleEffectAssetEvent::SimulationSpeedChanged;
  m_Events.Broadcast(e);
}

void ezParticleEffectAssetDocument::SetSimulationSpeed(float fSpeed)
{
  if (m_fSimulationSpeed == fSpeed)
    return;

  m_fSimulationSpeed = fSpeed;

  ezParticleEffectAssetEvent e;
  e.m_pDocument = this;
  e.m_Type = ezParticleEffectAssetEvent::SimulationSpeedChanged;
  m_Events.Broadcast(e);
}

void ezParticleEffectAssetDocument::SetRenderVisualizers(bool b)
{
  if (m_bRenderVisualizers == b)
    return;

  m_bRenderVisualizers = b;

  ezVisualizerManager::GetSingleton()->SetVisualizersActive(this, m_bRenderVisualizers);

  ezParticleEffectAssetEvent e;
  e.m_pDocument = this;
  e.m_Type = ezParticleEffectAssetEvent::RenderVisualizersChanged;
  m_Events.Broadcast(e);
}

ezResult ezParticleEffectAssetDocument::ComputeObjectTransformation(const ezDocumentObject* pObject, ezTransform& out_result) const
{
  out_result.SetIdentity();
  return EZ_SUCCESS;
}

void ezParticleEffectAssetDocument::UpdateAssetDocumentInfo(ezAssetDocumentInfo* pInfo) const
{
  SUPER::UpdateAssetDocumentInfo(pInfo);

  // Build a temporary descriptor to extract dependency info
  ezParticleEffectDescriptor desc;
  BuildDescriptorFromGraph(desc);

  for (const auto& system : desc.GetParticleSystems())
  {
    for (const auto& type : system->GetTypeFactories())
    {
      if (auto* pType = ezDynamicCast<ezParticleTypeQuadFactory*>(type))
      {
        if (pType->m_bUseCustomMaterial)
          pInfo->m_TransformDependencies.Remove(pType->m_sTexture);
        else
          pInfo->m_TransformDependencies.Remove(pType->m_sCustomMaterial);
      }

      if (auto* pType = ezDynamicCast<ezParticleTypeTrailFactory*>(type))
      {
        if (pType->m_bUseCustomMaterial)
          pInfo->m_TransformDependencies.Remove(pType->m_sTexture);
        else
          pInfo->m_TransformDependencies.Remove(pType->m_sCustomMaterial);
      }
    }
  }

  if (!desc.m_bAlwaysShared)
  {
    ezExposedParameters* pExposedParams = EZ_DEFAULT_NEW(ezExposedParameters);
    for (auto it = desc.m_FloatParameters.GetIterator(); it.IsValid(); ++it)
    {
      ezExposedParameter* param = EZ_DEFAULT_NEW(ezExposedParameter);
      pExposedParams->m_Parameters.PushBack(param);
      param->m_sName = it.Key();
      param->m_DefaultValue = it.Value();
    }
    for (auto it = desc.m_ColorParameters.GetIterator(); it.IsValid(); ++it)
    {
      ezExposedParameter* param = EZ_DEFAULT_NEW(ezExposedParameter);
      pExposedParams->m_Parameters.PushBack(param);
      param->m_sName = it.Key();
      param->m_DefaultValue = it.Value();
    }

    pInfo->m_MetaInfo.PushBack(pExposedParams);
  }
}

ezTransformStatus ezParticleEffectAssetDocument::InternalTransformAsset(ezStreamWriter& stream, ezStringView sOutputTag,
  const ezPlatformProfile* pAssetProfile, const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags)
{
  WriteResource(stream);
  return ezStatus(EZ_SUCCESS);
}

ezTransformStatus ezParticleEffectAssetDocument::InternalCreateThumbnail(const ThumbnailInfo& ThumbnailInfo)
{
  ezStatus status = ezAssetDocument::RemoteCreateThumbnail(ThumbnailInfo);
  return status;
}

//////////////////////////////////////////////////////////////////////////
// Visual graph metadata

void ezParticleEffectAssetDocument::InternalGetMetaDataHash(const ezDocumentObject* pObject, ezUInt64& inout_uiHash) const
{
  const ezVisualGraphObjectManager* pManager = static_cast<const ezVisualGraphObjectManager*>(GetObjectManager());
  pManager->GetMetaDataHash(pObject, inout_uiHash);
}

void ezParticleEffectAssetDocument::AttachMetaDataBeforeSaving(ezAbstractObjectGraph& graph) const
{
  SUPER::AttachMetaDataBeforeSaving(graph);
  const ezVisualGraphObjectManager* pManager = static_cast<const ezVisualGraphObjectManager*>(GetObjectManager());
  pManager->AttachMetaDataBeforeSaving(graph);
}

void ezParticleEffectAssetDocument::RestoreMetaDataAfterLoading(const ezAbstractObjectGraph& graph, bool bUndoable)
{
  SUPER::RestoreMetaDataAfterLoading(graph, bUndoable);
  ezVisualGraphObjectManager* pManager = static_cast<ezVisualGraphObjectManager*>(GetObjectManager());
  pManager->RestoreMetaDataAfterLoading(graph, bUndoable);
}

//////////////////////////////////////////////////////////////////////////
// Copy/Paste

void ezParticleEffectAssetDocument::GetSupportedMimeTypesForPasting(ezHybridArray<ezString, 4>& out_MimeTypes) const
{
  out_MimeTypes.PushBack("application/ezEditor.ParticleEffectGraph");
}

bool ezParticleEffectAssetDocument::CopySelectedObjects(ezAbstractObjectGraph& out_objectGraph, ezStringBuilder& out_MimeType) const
{
  out_MimeType = "application/ezEditor.ParticleEffectGraph";
  const ezVisualGraphObjectManager* pManager = static_cast<const ezVisualGraphObjectManager*>(GetObjectManager());
  return pManager->CopySelectedObjects(out_objectGraph);
}

bool ezParticleEffectAssetDocument::Paste(const ezArrayPtr<PasteInfo>& info, const ezAbstractObjectGraph& objectGraph, bool bAllowPickedPosition, ezStringView sMimeType)
{
  ezVisualGraphObjectManager* pManager = static_cast<ezVisualGraphObjectManager*>(GetObjectManager());
  return pManager->PasteObjects(info, objectGraph, ezQtVisualGraphScene::GetLastMouseInteractionPos(), bAllowPickedPosition);
}
