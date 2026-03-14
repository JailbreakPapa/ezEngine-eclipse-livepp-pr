#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphAsset.h>
#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphQt.h>
#include <Foundation/Math/ColorScheme.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/Nodes/Output/PoseResultAnimNode.h>
#include <RendererCore/AnimationSystem/AnimGraph/Nodes/Pose/SampleFrameAnimNode.h>
#include <RendererCore/AnimationSystem/AnimGraph/Nodes/StateMachine/StateMachineAnimNode.h>
#include <SharedPluginAssets/AnimationGraphAsset/AnimGraphStateMachineTypes.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>
#include <ToolsFoundation/Serialization/ToolsSerializationUtils.h>
#include <ToolsFoundation/VisualGraph/VisualGraphCommandAccessor.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAnimationGraphAssetDocument, 5, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAnimationGraphNodePin, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAnimationGraphAssetProperties, 1, ezRTTIDefaultAllocator<ezAnimationGraphAssetProperties>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ARRAY_MEMBER_PROPERTY("IncludeGraphs", m_IncludeGraphs)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Keyframe_Graph")),
    EZ_ARRAY_MEMBER_PROPERTY("AnimationClipMapping", m_AnimationClipMapping),
  }
    EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezSetAnimGraphInitialStateCommand, 1, ezRTTIDefaultAllocator<ezSetAnimGraphInitialStateCommand>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("NewInitialStateObject", m_NewInitialStateObject),
    EZ_MEMBER_PROPERTY("StateMachineScope", m_StateMachineScope),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_SUBSYSTEM_DECLARATION(EditorPluginAssets, AnimationGraph)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Core"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    ezQtVisualGraphScene::GetNodeFactory().RegisterCreator(ezGetStaticRTTI<ezAnimGraphNode>(), [](const ezRTTI* pRtti)->ezQtVisualGraphNode* { return new ezQtAnimationGraphNode(); });
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    ezQtVisualGraphScene::GetNodeFactory().UnregisterCreator(ezGetStaticRTTI<ezAnimGraphNode>());
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

//////////////////////////////////////////////////////////////////////////
// ezSetAnimGraphInitialStateCommand

constexpr const char* s_szIsInitialState = "IsInitialState";

ezSetAnimGraphInitialStateCommand::ezSetAnimGraphInitialStateCommand() = default;

ezStatus ezSetAnimGraphInitialStateCommand::DoInternal(bool bRedo)
{
  ezDocument* pDocument = GetDocument();
  auto* pManager = static_cast<ezAnimationGraphNodeManager*>(pDocument->GetObjectManager());

  if (!bRedo)
  {
    if (m_NewInitialStateObject.IsValid())
      m_pNewInitialStateObject = pManager->GetObject(m_NewInitialStateObject);

    if (auto pOldInitialState = pManager->GetInitialState(m_StateMachineScope))
      m_pOldInitialStateObject = pManager->GetObject(pOldInitialState->GetGuid());
  }

  if (m_pNewInitialStateObject)
    EZ_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->SetValue(m_pNewInitialStateObject, s_szIsInitialState, ezVariant(true)));

  if (m_pOldInitialStateObject)
    EZ_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->SetValue(m_pOldInitialStateObject, s_szIsInitialState, ezVariant(false)));

  return ezStatus(EZ_SUCCESS);
}

ezStatus ezSetAnimGraphInitialStateCommand::UndoInternal(bool bFireEvents)
{
  ezDocument* pDocument = GetDocument();

  if (m_pNewInitialStateObject)
    EZ_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->SetValue(m_pNewInitialStateObject, s_szIsInitialState, ezVariant(false)));

  if (m_pOldInitialStateObject)
    EZ_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->SetValue(m_pOldInitialStateObject, s_szIsInitialState, ezVariant(true)));

  return ezStatus(EZ_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
// ezAnimationGraphNodeManager — Type queries

bool ezAnimationGraphNodeManager::IsStateMachineNode(const ezDocumentObject* pObject) const
{
  return pObject->GetTypeAccessor().GetType()->IsDerivedFrom<ezStateMachineAnimNode>();
}

bool ezAnimationGraphNodeManager::IsStateNode(const ezDocumentObject* pObject) const
{
  return pObject->GetTypeAccessor().GetType()->IsDerivedFrom<ezAnimGraphStateNode>();
}

bool ezAnimationGraphNodeManager::IsAnyStateNode(const ezDocumentObject* pObject) const
{
  return pObject->GetTypeAccessor().GetType()->IsDerivedFrom<ezAnimGraphAnyStateNode>();
}

bool ezAnimationGraphNodeManager::IsTransitionConnection(const ezDocumentObject* pObject) const
{
  return pObject->GetTypeAccessor().GetType()->IsDerivedFrom<ezAnimGraphTransitionConnection>();
}

//////////////////////////////////////////////////////////////////////////
// ezAnimationGraphNodeManager — Core overrides

bool ezAnimationGraphNodeManager::InternalIsNode(const ezDocumentObject* pObject) const
{
  auto pType = pObject->GetTypeAccessor().GetType();
  return pType->IsDerivedFrom<ezAnimGraphNode>() ||
         pType->IsDerivedFrom<ezAnimGraphStateNodeBase>();
}

void ezAnimationGraphNodeManager::InternalCreatePins(const ezDocumentObject* pObject, NodeInternal& ref_node)
{
  if (IsStateNode(pObject))
  {
    CreateStatePins(pObject, ref_node, false);
    return;
  }

  if (IsAnyStateNode(pObject))
  {
    CreateStatePins(pObject, ref_node, true);
    return;
  }

  if (IsStateMachineNode(pObject))
  {
    CreateStateMachineNodePins(pObject, ref_node);
    return;
  }

  auto pType = pObject->GetTypeAccessor().GetType();
  if (pType->IsDerivedFrom<ezAnimGraphNode>())
  {
    CreateBlendTreePins(pObject, ref_node);
  }
}

void ezAnimationGraphNodeManager::GetCreateableTypes(ezHybridArray<const ezRTTI*, 32>& ref_types) const
{
  ref_types.Clear();

  if (IsCurrentScopeStateMachine())
  {
    // In state machine scope: can only create states and any-state
    ref_types.PushBack(ezGetStaticRTTI<ezAnimGraphStateNode>());
    ref_types.PushBack(ezGetStaticRTTI<ezAnimGraphAnyStateNode>());
  }
  else
  {
    // In blend tree scope (root or inside a state): can create all AnimGraphNode subtypes
    ezSet<const ezRTTI*> typeSet;
    ezReflectionUtils::GatherTypesDerivedFromClass(ezGetStaticRTTI<ezAnimGraphNode>(), typeSet);

    for (auto pType : typeSet)
    {
      if (pType->GetTypeFlags().IsAnySet(ezTypeFlags::Abstract))
        continue;

      ref_types.PushBack(pType);
    }
  }
}

ezStatus ezAnimationGraphNodeManager::InternalCanConnect(const ezVisualGraphPin& source, const ezVisualGraphPin& target, CanConnectResult& out_result) const
{
  // In state machine scope, connections are transitions (N-to-N, no type checking)
  if (IsCurrentScopeStateMachine())
  {
    out_result = CanConnectResult::ConnectNtoN;
    return ezStatus(EZ_SUCCESS);
  }

  // Blend tree scope: standard typed connection validation
  const ezAnimationGraphNodePin& sourcePin = ezStaticCast<const ezAnimationGraphNodePin&>(source);
  const ezAnimationGraphNodePin& targetPin = ezStaticCast<const ezAnimationGraphNodePin&>(target);

  out_result = CanConnectResult::ConnectNever;

  if (sourcePin.m_DataType != targetPin.m_DataType)
    return ezStatus("Can't connect pins of different data types");

  if (sourcePin.GetType() == targetPin.GetType())
    return ezStatus("Can only connect input pins with output pins.");

  switch (sourcePin.m_DataType)
  {
    case ezAnimGraphPin::Trigger:
      out_result = CanConnectResult::ConnectNtoN;
      break;

    case ezAnimGraphPin::Number:
      out_result = CanConnectResult::ConnectNto1;
      break;

    case ezAnimGraphPin::Bool:
      out_result = CanConnectResult::ConnectNto1;
      break;

    case ezAnimGraphPin::BoneWeights:
      out_result = CanConnectResult::ConnectNto1;
      break;

    case ezAnimGraphPin::LocalPose:
      if (targetPin.m_bMultiInputPin)
        out_result = CanConnectResult::ConnectNtoN;
      else
        out_result = CanConnectResult::ConnectNto1;
      break;

    case ezAnimGraphPin::ModelPose:
      out_result = CanConnectResult::ConnectNto1;
      break;

      // EXTEND THIS if a new type is introduced
      EZ_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  if (out_result != CanConnectResult::ConnectNever && WouldConnectionCreateCircle(source, target))
  {
    out_result = CanConnectResult::ConnectNever;
    return ezStatus("Connecting these pins would create a circle in the graph.");
  }

  return ezStatus(EZ_SUCCESS);
}

const ezRTTI* ezAnimationGraphNodeManager::GetConnectionType() const
{
  if (IsCurrentScopeStateMachine())
    return ezGetStaticRTTI<ezAnimGraphTransitionConnection>();

  return ezVisualGraphObjectManager::GetConnectionType();
}

bool ezAnimationGraphNodeManager::InternalIsDynamicPinProperty(const ezDocumentObject* pObject, const ezAbstractProperty* pProp) const
{
  return pProp->GetAttributeByType<ezDynamicPinAttribute>() != nullptr;
}

//////////////////////////////////////////////////////////////////////////
// ezAnimationGraphNodeManager — Pin creation helpers

void ezAnimationGraphNodeManager::CreateBlendTreePins(const ezDocumentObject* pObject, NodeInternal& ref_node)
{
  auto pType = pObject->GetTypeAccessor().GetType();

  ezHybridArray<const ezAbstractProperty*, 32> properties;
  pType->GetAllProperties(properties);

  const ezColor triggerPinColor = ezColorScheme::DarkUI(ezColorScheme::Yellow);
  const ezColor numberPinColor = ezColorScheme::DarkUI(ezColorScheme::Lime);
  const ezColor boolPinColor = ezColorScheme::LightUI(ezColorScheme::Lime);
  const ezColor weightPinColor = ezColorScheme::DarkUI(ezColorScheme::Teal);
  const ezColor localPosePinColor = ezColorScheme::DarkUI(ezColorScheme::Blue);
  const ezColor modelPosePinColor = ezColorScheme::DarkUI(ezColorScheme::Grape);
  // EXTEND THIS if a new type is introduced

  ezHybridArray<ezString, 16> pinNames;

  for (auto pProp : properties)
  {
    if (!pProp->GetSpecificType()->IsDerivedFrom<ezAnimGraphPin>())
      continue;

    pinNames.Clear();

    if (pProp->GetCategory() == ezPropertyCategory::Array)
    {
      if (const ezDynamicPinAttribute* pDynPin = pProp->GetAttributeByType<ezDynamicPinAttribute>())
      {
        GetDynamicPinNames(pObject, pDynPin->GetProperty(), pProp->GetPropertyName(), pinNames);
      }
    }
    else if (pProp->GetCategory() == ezPropertyCategory::Member)
    {
      pinNames.PushBack(pProp->GetPropertyName());
    }

    for (ezUInt32 i = 0; i < pinNames.GetCount(); ++i)
    {
      const auto& pinName = pinNames[i];

      if (pProp->GetSpecificType()->IsDerivedFrom<ezAnimGraphTriggerInputPin>())
      {
        auto pPin = EZ_DEFAULT_NEW(ezAnimationGraphNodePin, ezVisualGraphPin::Type::Input, pinName, triggerPinColor, pObject);
        pPin->m_DataType = ezAnimGraphPin::Trigger;
        ref_node.m_Inputs.PushBack(pPin);
      }
      else if (pProp->GetSpecificType()->IsDerivedFrom<ezAnimGraphTriggerOutputPin>())
      {
        auto pPin = EZ_DEFAULT_NEW(ezAnimationGraphNodePin, ezVisualGraphPin::Type::Output, pinName, triggerPinColor, pObject);
        pPin->m_DataType = ezAnimGraphPin::Trigger;
        ref_node.m_Outputs.PushBack(pPin);
      }
      else if (pProp->GetSpecificType()->IsDerivedFrom<ezAnimGraphNumberInputPin>())
      {
        auto pPin = EZ_DEFAULT_NEW(ezAnimationGraphNodePin, ezVisualGraphPin::Type::Input, pinName, numberPinColor, pObject);
        pPin->m_DataType = ezAnimGraphPin::Number;
        ref_node.m_Inputs.PushBack(pPin);
      }
      else if (pProp->GetSpecificType()->IsDerivedFrom<ezAnimGraphNumberOutputPin>())
      {
        auto pPin = EZ_DEFAULT_NEW(ezAnimationGraphNodePin, ezVisualGraphPin::Type::Output, pinName, numberPinColor, pObject);
        pPin->m_DataType = ezAnimGraphPin::Number;
        ref_node.m_Outputs.PushBack(pPin);
      }
      else if (pProp->GetSpecificType()->IsDerivedFrom<ezAnimGraphBoolInputPin>())
      {
        auto pPin = EZ_DEFAULT_NEW(ezAnimationGraphNodePin, ezVisualGraphPin::Type::Input, pinName, boolPinColor, pObject);
        pPin->m_DataType = ezAnimGraphPin::Bool;
        ref_node.m_Inputs.PushBack(pPin);
      }
      else if (pProp->GetSpecificType()->IsDerivedFrom<ezAnimGraphBoolOutputPin>())
      {
        auto pPin = EZ_DEFAULT_NEW(ezAnimationGraphNodePin, ezVisualGraphPin::Type::Output, pinName, boolPinColor, pObject);
        pPin->m_DataType = ezAnimGraphPin::Bool;
        ref_node.m_Outputs.PushBack(pPin);
      }
      else if (pProp->GetSpecificType()->IsDerivedFrom<ezAnimGraphBoneWeightsInputPin>())
      {
        auto pPin = EZ_DEFAULT_NEW(ezAnimationGraphNodePin, ezVisualGraphPin::Type::Input, pinName, weightPinColor, pObject);
        pPin->m_DataType = ezAnimGraphPin::BoneWeights;
        ref_node.m_Inputs.PushBack(pPin);
      }
      else if (pProp->GetSpecificType()->IsDerivedFrom<ezAnimGraphBoneWeightsOutputPin>())
      {
        auto pPin = EZ_DEFAULT_NEW(ezAnimationGraphNodePin, ezVisualGraphPin::Type::Output, pinName, weightPinColor, pObject);
        pPin->m_DataType = ezAnimGraphPin::BoneWeights;
        ref_node.m_Outputs.PushBack(pPin);
      }
      else if (pProp->GetSpecificType()->IsDerivedFrom<ezAnimGraphLocalPoseInputPin>())
      {
        auto pPin = EZ_DEFAULT_NEW(ezAnimationGraphNodePin, ezVisualGraphPin::Type::Input, pinName, localPosePinColor, pObject);
        pPin->m_DataType = ezAnimGraphPin::LocalPose;
        ref_node.m_Inputs.PushBack(pPin);
      }
      else if (pProp->GetSpecificType()->IsDerivedFrom<ezAnimGraphLocalPoseOutputPin>())
      {
        auto pPin = EZ_DEFAULT_NEW(ezAnimationGraphNodePin, ezVisualGraphPin::Type::Output, pinName, localPosePinColor, pObject);
        pPin->m_DataType = ezAnimGraphPin::LocalPose;
        ref_node.m_Outputs.PushBack(pPin);
      }
      else
      {
        // EXTEND THIS if a new type is introduced
        EZ_ASSERT_NOT_IMPLEMENTED;
      }
    }
  }
}

void ezAnimationGraphNodeManager::CreateStatePins(const ezDocumentObject* pObject, NodeInternal& ref_node, bool bIsAnyState)
{
  const ezColor stateColor = ezColor::Grey;

  // Any-state node has no input pin (nothing transitions *to* "any state")
  if (!bIsAnyState)
  {
    auto pInPin = EZ_DEFAULT_NEW(ezAnimationGraphNodePin, ezVisualGraphPin::Type::Input, "Enter", stateColor, pObject);
    pInPin->m_DataType = ezAnimGraphPin::Invalid;
    ref_node.m_Inputs.PushBack(pInPin);
  }

  auto pOutPin = EZ_DEFAULT_NEW(ezAnimationGraphNodePin, ezVisualGraphPin::Type::Output, "Exit", stateColor, pObject);
  pOutPin->m_DataType = ezAnimGraphPin::Invalid;
  ref_node.m_Outputs.PushBack(pOutPin);
}

void ezAnimationGraphNodeManager::CreateStateMachineNodePins(const ezDocumentObject* pObject, NodeInternal& ref_node)
{
  // In the parent scope, a state machine node exposes a single LocalPose output
  const ezColor localPosePinColor = ezColorScheme::DarkUI(ezColorScheme::Blue);

  auto pOutPin = EZ_DEFAULT_NEW(ezAnimationGraphNodePin, ezVisualGraphPin::Type::Output, "OutPose", localPosePinColor, pObject);
  pOutPin->m_DataType = ezAnimGraphPin::LocalPose;
  ref_node.m_Outputs.PushBack(pOutPin);
}

//////////////////////////////////////////////////////////////////////////
// ezAnimationGraphNodeManager — Scope tracking

void ezAnimationGraphNodeManager::SetCurrentViewScope(const ezUuid& scopeGuid)
{
  m_CurrentViewScope = scopeGuid;
}

bool ezAnimationGraphNodeManager::IsCurrentScopeStateMachine() const
{
  if (!m_CurrentViewScope.IsValid())
    return false;

  // Check if the scope UUID refers to a state machine node
  for (auto pChild : GetRootObject()->GetChildren())
  {
    if (pChild->GetGuid() == m_CurrentViewScope)
    {
      return IsStateMachineNode(pChild);
    }
  }

  return false;
}

void ezAnimationGraphNodeManager::GetBreadcrumbPath(ezDynamicArray<ezAnimGraphBreadcrumbEntry>& out_path) const
{
  out_path.Clear();
  out_path.PushBack({ezUuid(), "Root"});

  if (!m_CurrentViewScope.IsValid())
    return;

  // Walk up the scope chain to build the breadcrumb path.
  // Scope nesting: root -> SM node -> state -> (nested SM) -> ...
  // We find the current scope's object and trace its scope parents.

  ezDynamicArray<ezAnimGraphBreadcrumbEntry> chain;
  ezUuid current = m_CurrentViewScope;

  while (current.IsValid())
  {
    for (auto pChild : GetRootObject()->GetChildren())
    {
      if (pChild->GetGuid() == current)
      {
        ezString name;
        if (IsStateMachineNode(pChild))
        {
          name = pChild->GetTypeAccessor().GetValue("CustomNodeTitle").ConvertTo<ezString>();
          if (name.IsEmpty())
            name = "State Machine";
        }
        else if (IsStateNode(pChild))
        {
          name = pChild->GetTypeAccessor().GetValue("Name").ConvertTo<ezString>();
          if (name.IsEmpty())
            name = "State";
        }
        else
        {
          name = "Unknown";
        }

        chain.PushBack({current, name});
        break;
      }
    }

    // Move to the parent scope
    ezUuid parentScope;
    if (m_NodeToScope.TryGetValue(current, parentScope))
      current = parentScope;
    else
      current = ezUuid();
  }

  // Reverse to get root-to-leaf order
  for (ezInt32 i = (ezInt32)chain.GetCount() - 1; i >= 0; --i)
  {
    out_path.PushBack(chain[i]);
  }
}

void ezAnimationGraphNodeManager::GetNodesInScope(const ezUuid& scopeGuid, ezDynamicArray<const ezDocumentObject*>& out_nodes) const
{
  out_nodes.Clear();

  for (auto pChild : GetRootObject()->GetChildren())
  {
    if (!IsNode(pChild))
      continue;

    ezUuid nodeScope = GetNodeScope(pChild);

    if (nodeScope == scopeGuid)
    {
      out_nodes.PushBack(pChild);
    }
  }
}

ezUuid ezAnimationGraphNodeManager::GetNodeScope(const ezDocumentObject* pObject) const
{
  ezUuid scope;
  m_NodeToScope.TryGetValue(pObject->GetGuid(), scope);
  return scope;
}

void ezAnimationGraphNodeManager::SetNodeScope(const ezDocumentObject* pObject, const ezUuid& scopeGuid)
{
  if (scopeGuid.IsValid())
    m_NodeToScope[pObject->GetGuid()] = scopeGuid;
  else
    m_NodeToScope.Remove(pObject->GetGuid());
}

const ezDocumentObject* ezAnimationGraphNodeManager::GetInitialState(const ezUuid& stateMachineGuid) const
{
  for (auto pChild : GetRootObject()->GetChildren())
  {
    if (!IsStateNode(pChild))
      continue;

    if (GetNodeScope(pChild) != stateMachineGuid)
      continue;

    if (pChild->GetTypeAccessor().GetValue("IsInitialState").ConvertTo<bool>())
      return pChild;
  }

  return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// ezAnimationGraphAssetDocument

ezAnimationGraphAssetDocument::ezAnimationGraphAssetDocument(ezStringView sDocumentPath)
  : ezSimpleAssetDocument<ezAnimationGraphAssetProperties>(EZ_DEFAULT_NEW(ezAnimationGraphNodeManager), sDocumentPath, ezAssetDocEngineConnection::None)
{
  m_pObjectAccessor = EZ_DEFAULT_NEW(ezVisualGraphCommandAccessor, GetCommandHistory());
}

ezTransformStatus ezAnimationGraphAssetDocument::InternalTransformAsset(ezStreamWriter& stream, ezStringView sOutputTag, const ezPlatformProfile* pAssetProfile, const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags)
{
  const auto* pNodeManager = static_cast<const ezAnimationGraphNodeManager*>(GetObjectManager());

  auto pProp = GetProperties();

  {
    stream.WriteVersion(2);
    stream.WriteArray(pProp->m_IncludeGraphs).AssertSuccess();

    const ezUInt32 uiNum = pProp->m_AnimationClipMapping.GetCount();
    stream << uiNum;

    for (ezUInt32 i = 0; i < uiNum; ++i)
    {
      stream << pProp->m_AnimationClipMapping[i].m_sClipName;
      stream << pProp->m_AnimationClipMapping[i].m_hClip;
    }
  }

  // Find all root-scope nodes (nodes with no scope parent)
  ezDynamicArray<const ezDocumentObject*> allNodes;
  pNodeManager->GetNodesInScope(ezUuid(), allNodes);

  // Filter to only AnimGraphNode-derived objects (not states/connections at root level)
  ezDynamicArray<const ezDocumentObject*> rootBlendTreeNodes;
  for (const auto* pNode : allNodes)
  {
    if (pNode->GetType()->IsDerivedFrom<ezAnimGraphNode>())
    {
      rootBlendTreeNodes.PushBack(pNode);
    }
  }

  ezAnimGraph animGraph;

  ezMap<const ezDocumentObject*, ezAnimGraphNode*> docNodeToRuntimeNode;

  // create all nodes in the ezAnimGraph
  {
    for (const ezDocumentObject* pNode : rootBlendTreeNodes)
    {
      ezAnimGraphNode* pNewNode = animGraph.AddNode(pNode->GetType()->GetAllocator()->Allocate<ezAnimGraphNode>());

      // copy all the non-hidden properties
      ezToolsSerializationUtils::CopyProperties(pNode, GetObjectManager(), pNewNode, pNewNode->GetDynamicRTTI(), [](const ezAbstractProperty* p)
        { return p->GetAttributeByType<ezHiddenAttribute>() == nullptr; });

      docNodeToRuntimeNode[pNode] = pNewNode;

      // If this is a state machine node, build its internal state/transition data
      if (pNodeManager->IsStateMachineNode(pNode))
      {
        auto* pSmNode = static_cast<ezStateMachineAnimNode*>(pNewNode);

        // Gather states scoped to this SM node
        ezDynamicArray<const ezDocumentObject*> stateNodes;
        pNodeManager->GetNodesInScope(pNode->GetGuid(), stateNodes);

        for (const auto* pStateObj : stateNodes)
        {
          if (!pNodeManager->IsStateNode(pStateObj))
            continue;

          auto& state = pSmNode->AddState();
          state.m_sName.Assign(pStateObj->GetTypeAccessor().GetValue("Name").ConvertTo<ezString>());
          state.m_bIsInitialState = pStateObj->GetTypeAccessor().GetValue("IsInitialState").ConvertTo<bool>();

          // Build the sub-graph for this state's blend tree
          ezDynamicArray<const ezDocumentObject*> blendTreeNodes;
          pNodeManager->GetNodesInScope(pStateObj->GetGuid(), blendTreeNodes);

          ezMap<const ezDocumentObject*, ezAnimGraphNode*> subDocToRuntime;

          for (const auto* pBtNode : blendTreeNodes)
          {
            if (!pBtNode->GetType()->IsDerivedFrom<ezAnimGraphNode>())
              continue;

            ezAnimGraphNode* pSubNode = state.m_SubGraph.AddNode(pBtNode->GetType()->GetAllocator()->Allocate<ezAnimGraphNode>());

            ezToolsSerializationUtils::CopyProperties(pBtNode, GetObjectManager(), pSubNode, pSubNode->GetDynamicRTTI(), [](const ezAbstractProperty* p)
              { return p->GetAttributeByType<ezHiddenAttribute>() == nullptr; });

            subDocToRuntime[pBtNode] = pSubNode;
          }

          // Wire sub-graph connections
          for (const auto* pBtNode : blendTreeNodes)
          {
            if (!pBtNode->GetType()->IsDerivedFrom<ezAnimGraphNode>())
              continue;

            const auto outputPins = pNodeManager->GetOutputPins(pBtNode);
            for (auto& pPin : outputPins)
            {
              for (const ezVisualGraphConnection* pCon : pNodeManager->GetConnections(*pPin))
              {
                const auto* pSrcDoc = pCon->GetSourcePin().GetParent();
                const auto* pDstDoc = pCon->GetTargetPin().GetParent();

                if (!subDocToRuntime.Contains(pSrcDoc) || !subDocToRuntime.Contains(pDstDoc))
                  continue;

                state.m_SubGraph.AddConnection(
                  subDocToRuntime[pSrcDoc], pCon->GetSourcePin().GetName(),
                  subDocToRuntime[pDstDoc], pCon->GetTargetPin().GetName());
              }
            }
          }
        }

        // TODO: Build transitions from transition connections scoped to this SM node
      }
    }
  }

  // add all root-level node connections to the ezAnimGraph
  {
    for (const ezDocumentObject* pNode : rootBlendTreeNodes)
    {
      const auto outputPins = pNodeManager->GetOutputPins(pNode);

      for (auto& pPin : outputPins)
      {
        for (const ezVisualGraphConnection* pCon : pNodeManager->GetConnections(*pPin))
        {
          const ezAnimGraphNode* pSrcNode = docNodeToRuntimeNode[pCon->GetSourcePin().GetParent()];
          ezAnimGraphNode* pDstNode = docNodeToRuntimeNode[pCon->GetTargetPin().GetParent()];

          if (pSrcNode && pDstNode)
          {
            animGraph.AddConnection(pSrcNode, pCon->GetSourcePin().GetName(), pDstNode, pCon->GetTargetPin().GetName());
          }
        }
      }
    }
  }

  EZ_SUCCEED_OR_RETURN(animGraph.Serialize(stream));

  return ezTransformStatus(EZ_SUCCESS);
}

void ezAnimationGraphAssetDocument::InternalGetMetaDataHash(const ezDocumentObject* pObject, ezUInt64& inout_uiHash) const
{
  // without this, changing connections only (no property value) may not result in a different asset document hash and therefore no transform

  const ezVisualGraphObjectManager* pManager = static_cast<const ezVisualGraphObjectManager*>(GetObjectManager());
  pManager->GetMetaDataHash(pObject, inout_uiHash);
}

void ezAnimationGraphAssetDocument::AttachMetaDataBeforeSaving(ezAbstractObjectGraph& graph) const
{
  SUPER::AttachMetaDataBeforeSaving(graph);
  const ezVisualGraphObjectManager* pManager = static_cast<const ezVisualGraphObjectManager*>(GetObjectManager());
  pManager->AttachMetaDataBeforeSaving(graph);
}

void ezAnimationGraphAssetDocument::RestoreMetaDataAfterLoading(const ezAbstractObjectGraph& graph, bool bUndoable)
{
  SUPER::RestoreMetaDataAfterLoading(graph, bUndoable);
  ezVisualGraphObjectManager* pManager = static_cast<ezVisualGraphObjectManager*>(GetObjectManager());
  pManager->RestoreMetaDataAfterLoading(graph, bUndoable);
}



void ezAnimationGraphAssetDocument::GetSupportedMimeTypesForPasting(ezHybridArray<ezString, 4>& out_MimeTypes) const
{
  out_MimeTypes.PushBack("application/ezEditor.AnimationGraphGraph");
}

bool ezAnimationGraphAssetDocument::CopySelectedObjects(ezAbstractObjectGraph& out_objectGraph, ezStringBuilder& out_MimeType) const
{
  out_MimeType = "application/ezEditor.AnimationGraphGraph";

  const ezVisualGraphObjectManager* pManager = static_cast<const ezVisualGraphObjectManager*>(GetObjectManager());
  return pManager->CopySelectedObjects(out_objectGraph);
}

bool ezAnimationGraphAssetDocument::Paste(const ezArrayPtr<PasteInfo>& info, const ezAbstractObjectGraph& objectGraph, bool bAllowPickedPosition, ezStringView sMimeType)
{
  ezVisualGraphObjectManager* pManager = static_cast<ezVisualGraphObjectManager*>(GetObjectManager());
  return pManager->PasteObjects(info, objectGraph, ezQtVisualGraphScene::GetLastMouseInteractionPos(), bAllowPickedPosition);
}

ezAnimationGraphNodePin::ezAnimationGraphNodePin(Type type, const char* szName, const ezColorGammaUB& color, const ezDocumentObject* pObject)
  : ezVisualGraphPin(type, szName, color, pObject)
{
}

ezAnimationGraphNodePin::~ezAnimationGraphNodePin() = default;
