#include <EnginePluginAssets/EnginePluginAssetsPCH.h>

#include <Core/ResourceManager/ResourceManager.h>
#include <Core/Utils/Blackboard.h>
#include <EnginePluginAssets/AnimationGraphAsset/AnimationGraphContext.h>
#include <EnginePluginAssets/AnimationGraphAsset/AnimationGraphView.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Utilities/ConversionUtils.h>
#include <GameEngine/Animation/Skeletal/AnimatedMeshComponent.h>
#include <GameEngine/Animation/Skeletal/AnimationControllerComponent.h>
#include <GameEngine/Animation/Skeletal/SimpleAnimationComponent.h>
#include <GameEngine/Gameplay/BlackboardComponent.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphResource.h>
#include <RendererCore/AnimationSystem/AnimGraph/Nodes/StateMachine/StateMachineAnimNode.h>
#include <RendererCore/AnimationSystem/SkeletonComponent.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAnimationGraphContext, 1, ezRTTIDefaultAllocator<ezAnimationGraphContext>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_CONSTANT_PROPERTY("DocumentType", (const char*) "Animation Graph"),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezAnimationGraphContext::ezAnimationGraphContext()
  : ezEngineProcessDocumentContext(ezEngineProcessDocumentContextFlags::CreateWorld)
{
}

void ezAnimationGraphContext::HandleMessage(const ezEditorEngineDocumentMsg* pMsg0)
{
  if (auto pMsg = ezDynamicCast<const ezQuerySelectionBBoxMsgToEngine*>(pMsg0))
  {
    QuerySelectionBBox(pMsg);
    return;
  }

  if (auto pMsg = ezDynamicCast<const ezSimpleDocumentConfigMsgToEngine*>(pMsg0))
  {
    if (pMsg->m_sWhatToDo == "CommonAssetUiState")
    {
      if (pMsg->m_sPayload == "Grid")
      {
        m_bDisplayGrid = pMsg->m_PayloadValue.ConvertTo<float>() > 0;
      }
    }
    else if (pMsg->m_sWhatToDo == "PreviewMesh" && m_sAnimatedMeshToUse != pMsg->m_sPayload)
    {
      m_sAnimatedMeshToUse = pMsg->m_sPayload;

      auto pWorld = m_pWorld;
      EZ_LOCK(pWorld->GetWriteMarker());

      ezAnimatedMeshComponent* pAnimMesh;
      if (pWorld->TryGetComponent(m_hAnimMeshComponent, pAnimMesh))
      {
        pAnimMesh->DeleteComponent();
        m_hAnimMeshComponent.Invalidate();
      }

      ezSkeletonComponent* pSkeleton;
      if (pWorld->TryGetComponent(m_hSkeletonComponent, pSkeleton))
      {
        pSkeleton->DeleteComponent();
        m_hSkeletonComponent.Invalidate();
      }

      ezAnimationControllerComponent* pAnimController;
      if (pWorld->TryGetComponent(m_hAnimControllerComponent, pAnimController))
      {
        pAnimController->DeleteComponent();
        m_hAnimControllerComponent.Invalidate();
      }

      ezLocalBlackboardComponent* pBlackboard;
      if (pWorld->TryGetComponent(m_hBlackboardComponent, pBlackboard))
      {
        pBlackboard->DeleteComponent();
        m_hBlackboardComponent.Invalidate();
      }

      if (!m_sAnimatedMeshToUse.IsEmpty())
      {
        m_hAnimMeshComponent = ezAnimatedMeshComponent::CreateComponent(m_pGameObject, pAnimMesh);
        pAnimMesh->SetMeshFile(m_sAnimatedMeshToUse);

        m_hSkeletonComponent = ezSkeletonComponent::CreateComponent(m_pGameObject, pSkeleton);
        pSkeleton->m_bVisualizeBones = true;

        // Create a local blackboard for animation parameters.
        // This must be created BEFORE the animation controller so that
        // FindBlackboard() can find it during OnSimulationStarted().
        m_hBlackboardComponent = ezLocalBlackboardComponent::CreateComponent(m_pGameObject, pBlackboard);

        // Apply any cached blackboard parameters now that the component exists
        ApplyBlackboardParams();

        // Create animation controller to evaluate the current animation graph
        ezStringBuilder sAnimGraphGuid;
        ezConversionUtils::ToString(GetDocumentGuid(), sAnimGraphGuid);

        m_hAnimControllerComponent = ezAnimationControllerComponent::CreateComponent(m_pGameObject, pAnimController);

        // Set the AnimGraph resource via RTTI property since m_hAnimGraph is protected.
        // Use the document GUID as the resource identifier — the editor engine process
        // maps document GUIDs to actual resource files via the asset curator.
        const ezAbstractProperty* pProp = pAnimController->GetDynamicRTTI()->FindPropertyByName("AnimGraph");
        if (pProp && pProp->GetCategory() == ezPropertyCategory::Member)
        {
          auto* pMemberProp = static_cast<const ezAbstractMemberProperty*>(pProp);
          ezVariant v = ezString(sAnimGraphGuid);
          ezReflectionUtils::SetMemberPropertyValue(pMemberProp, pAnimController, v);
        }

        pWorld->SetWorldSimulationEnabled(true);
      }
    }
    else if (pMsg->m_sWhatToDo == "RenderBones")
    {
      EZ_LOCK(m_pWorld->GetWriteMarker());

      ezSkeletonComponent* pSkeleton;
      if (m_pWorld->TryGetComponent(m_hSkeletonComponent, pSkeleton))
      {
        pSkeleton->m_bVisualizeBones = pMsg->m_PayloadValue.ConvertTo<bool>();
      }
    }
    else if (pMsg->m_sWhatToDo == "RenderMesh")
    {
      EZ_LOCK(m_pWorld->GetWriteMarker());

      ezAnimatedMeshComponent* pAnimMesh;
      if (m_pWorld->TryGetComponent(m_hAnimMeshComponent, pAnimMesh))
      {
        pAnimMesh->SetActiveFlag(pMsg->m_PayloadValue.ConvertTo<bool>());
      }
    }
    else if (pMsg->m_sWhatToDo == "SimulationSpeed")
    {
      EZ_LOCK(m_pWorld->GetWriteMarker());

      m_fSimulationSpeed = pMsg->m_PayloadValue.ConvertTo<float>();
      m_pWorld->GetClock().SetSpeed(m_fSimulationSpeed);
    }
    else if (pMsg->m_sWhatToDo == "Restart")
    {
      // Restart by deleting and recreating the animation controller, which resets all state
      if (!m_sAnimatedMeshToUse.IsEmpty())
      {
        auto pWorld = m_pWorld;
        EZ_LOCK(pWorld->GetWriteMarker());

        ezAnimationControllerComponent* pAnimController;
        if (pWorld->TryGetComponent(m_hAnimControllerComponent, pAnimController))
        {
          pAnimController->DeleteComponent();
          m_hAnimControllerComponent.Invalidate();
        }

        ezStringBuilder sAnimGraphGuid;
        ezConversionUtils::ToString(GetDocumentGuid(), sAnimGraphGuid);

        m_hAnimControllerComponent = ezAnimationControllerComponent::CreateComponent(m_pGameObject, pAnimController);

        const ezAbstractProperty* pProp = pAnimController->GetDynamicRTTI()->FindPropertyByName("AnimGraph");
        if (pProp && pProp->GetCategory() == ezPropertyCategory::Member)
        {
          auto* pMemberProp = static_cast<const ezAbstractMemberProperty*>(pProp);
          ezVariant v = ezString(sAnimGraphGuid);
          ezReflectionUtils::SetMemberPropertyValue(pMemberProp, pAnimController, v);
        }
      }
    }
    else if (pMsg->m_sWhatToDo == "PlayClip")
    {
      auto pWorld = m_pWorld;
      EZ_LOCK(pWorld->GetWriteMarker());

      // Remove any existing simple anim component from a previous PlayClip
      ezSimpleAnimationComponent* pSimpleAnim;
      if (pWorld->TryGetComponent(m_hSimpleAnimComponent, pSimpleAnim))
      {
        pSimpleAnim->DeleteComponent();
        m_hSimpleAnimComponent.Invalidate();
      }

      if (pMsg->m_sPayload.IsEmpty())
      {
        // Empty payload means "stop isolated clip, re-enable controller"
        m_bPlayingIsolatedClip = false;

        ezAnimationControllerComponent* pAnimCtrl;
        if (pWorld->TryGetComponent(m_hAnimControllerComponent, pAnimCtrl))
        {
          pAnimCtrl->SetActiveFlag(true);
        }
      }
      else
      {
        // Disable the animation controller while playing a clip in isolation
        ezAnimationControllerComponent* pAnimCtrl;
        if (pWorld->TryGetComponent(m_hAnimControllerComponent, pAnimCtrl))
        {
          pAnimCtrl->SetActiveFlag(false);
        }

        m_bPlayingIsolatedClip = true;

        m_hSimpleAnimComponent = ezSimpleAnimationComponent::CreateComponent(m_pGameObject, pSimpleAnim);
        pSimpleAnim->SetAnimationClipFile(pMsg->m_sPayload);
      }
    }
    else if (pMsg->m_sWhatToDo == "RegisterBlackboardParams")
    {
      // Cache the payload and apply immediately if blackboard exists.
      // If the blackboard doesn't exist yet (preview mesh not set), the cached
      // payload will be applied when the blackboard is created.
      m_sBlackboardParamsPayload = pMsg->m_sPayload;
      ApplyBlackboardParams();
    }
    else if (pMsg->m_sWhatToDo == "SetBlackboardValue")
    {
      // Format: "name|type|value"
      ezStringBuilder sPayload = pMsg->m_sPayload;
      ezHybridArray<ezStringView, 3> parts;
      sPayload.Split(false, parts, "|");

      if (parts.GetCount() >= 3)
      {
        ezString sName(parts[0]);
        ezString sType(parts[1]);
        ezString sValue(parts[2]);

        EZ_LOCK(m_pWorld->GetWriteMarker());

        ezLocalBlackboardComponent* pBlackboard;
        if (m_pWorld->TryGetComponent(m_hBlackboardComponent, pBlackboard))
        {
          ezBlackboard* pBB = pBlackboard->GetBoard().Borrow();
          if (pBB)
          {
            if (sType == "Bool")
            {
              bool bValue = (sValue == "true" || sValue == "1");
              pBB->SetEntryValue(sName, bValue);
            }
            else // Number
            {
              double fValue = 0.0;
              ezConversionUtils::StringToFloat(sValue, fValue).IgnoreResult();
              pBB->SetEntryValue(sName, fValue);
            }
          }
        }
      }
    }

    return;
  }

  if (auto pMsg = ezDynamicCast<const ezViewRedrawMsgToEngine*>(pMsg0))
  {
    SendDebugState(pMsg0);
  }

  ezEngineProcessDocumentContext::HandleMessage(pMsg0);
}

void ezAnimationGraphContext::OnInitialize()
{
  auto pWorld = m_pWorld;
  EZ_LOCK(pWorld->GetWriteMarker());

  ezGameObjectDesc obj;
  obj.m_bDynamic = true;
  obj.m_sName.Assign("AnimGraphPreview");
  pWorld->CreateObject(obj, m_pGameObject);
}

ezEngineProcessViewContext* ezAnimationGraphContext::CreateViewContext()
{
  return EZ_DEFAULT_NEW(ezAnimationGraphViewContext, this);
}

void ezAnimationGraphContext::DestroyViewContext(ezEngineProcessViewContext* pContext)
{
  EZ_DEFAULT_DELETE(pContext);
}

bool ezAnimationGraphContext::UpdateThumbnailViewContext(ezEngineProcessViewContext* pThumbnailViewContext)
{
  ezBoundingBoxSphere bounds = GetWorldBounds(m_pWorld);

  if (!m_hAnimControllerComponent.IsInvalidated())
  {
    EZ_LOCK(m_pWorld->GetWriteMarker());

    // Do a one-shot world update to get the animation pose applied
    m_pWorld->SetWorldSimulationEnabled(true);
    m_pWorld->Update();
    m_pWorld->SetWorldSimulationEnabled(false);

    bounds = GetWorldBounds(m_pWorld);
  }

  ezAnimationGraphViewContext* pViewContext = static_cast<ezAnimationGraphViewContext*>(pThumbnailViewContext);
  return pViewContext->UpdateThumbnailCamera(bounds);
}

void ezAnimationGraphContext::QuerySelectionBBox(const ezEditorEngineDocumentMsg* pMsg)
{
  if (m_pGameObject == nullptr)
    return;

  ezBoundingBoxSphere bounds = ezBoundingBoxSphere::MakeInvalid();

  {
    EZ_LOCK(m_pWorld->GetWriteMarker());

    m_pGameObject->UpdateLocalBounds();
    m_pGameObject->UpdateGlobalTransformAndBounds();
    const auto& b = m_pGameObject->GetGlobalBounds();

    if (b.IsValid())
      bounds.ExpandToInclude(b);
  }

  const ezQuerySelectionBBoxMsgToEngine* msg = static_cast<const ezQuerySelectionBBoxMsgToEngine*>(pMsg);

  ezQuerySelectionBBoxResultMsgToEditor res;
  res.m_uiViewID = msg->m_uiViewID;
  res.m_iPurpose = msg->m_iPurpose;
  res.m_vCenter = bounds.m_vCenter;
  res.m_vHalfExtents = bounds.m_vBoxHalfExtents;
  res.m_DocumentGuid = pMsg->m_DocumentGuid;

  SendProcessMessage(&res);
}

void ezAnimationGraphContext::SendDebugState(const ezEditorEngineDocumentMsg* pMsg)
{
  EZ_LOCK(m_pWorld->GetWriteMarker());

  ezAnimationControllerComponent* pAnimCtrl;
  if (!m_pWorld->TryGetComponent(m_hAnimControllerComponent, pAnimCtrl))
    return;

  const ezAnimController& controller = pAnimCtrl->GetAnimController();
  const auto* pActiveNodes = controller.GetActiveNodes();

  if (pActiveNodes == nullptr)
    return;

  // Get the animation graph resource to look up debug indices
  const auto& hGraphRes = controller.GetGraphResourceHandle();
  if (!hGraphRes.IsValid())
    return;

  ezResourceLock<ezAnimGraphResource> pGraphRes(hGraphRes, ezResourceAcquireMode::AllowLoadingFallback_NeverFail);
  if (pGraphRes.GetAcquireResult() != ezResourceAcquireResult::Final)
    return;

  const ezAnimGraph& graph = pGraphRes->GetAnimationGraph();
  auto nodes = graph.GetNodes();

  // Build comma-separated list of active debug indices
  ezStringBuilder sActiveNodes;
  for (ezUInt32 i = 0; i < nodes.GetCount() && i < pActiveNodes->GetCount(); ++i)
  {
    if ((*pActiveNodes)[i])
    {
      if (!sActiveNodes.IsEmpty())
        sActiveNodes.Append(",");
      sActiveNodes.AppendFormat("{}", nodes[i]->GetDebugIndex());
    }
  }

  {
    ezSimpleDocumentConfigMsgToEditor msg;
    msg.m_DocumentGuid = pMsg->m_DocumentGuid;
    msg.m_sWhatToDo = "ActiveNodes";
    msg.m_sPayload = sActiveNodes;
    SendProcessMessage(&msg);
  }

  // Find state machine nodes and report their state
  ezStringBuilder sStateName;
  double fTimeInState = 0.0;

  const ezAnimGraphInstance* pGraphInst = controller.GetGraphInstance(0);
  if (pGraphInst)
  {
    for (ezUInt32 i = 0; i < nodes.GetCount(); ++i)
    {
      if (auto* pSmNode = ezDynamicCast<const ezStateMachineAnimNode*>(nodes[i].Borrow()))
      {
        ezStringView name = pSmNode->GetCurrentStateName(*pGraphInst);
        if (!name.IsEmpty())
        {
          sStateName = name;
          fTimeInState = pSmNode->GetTimeInCurrentState(*pGraphInst).GetSeconds();
        }
        break; // Only report the first state machine
      }
    }
  }

  {
    ezSimpleDocumentConfigMsgToEditor msg;
    msg.m_DocumentGuid = pMsg->m_DocumentGuid;
    msg.m_sWhatToDo = "StateMachineState";
    msg.m_sPayload = sStateName;
    msg.m_PayloadValue = fTimeInState;
    SendProcessMessage(&msg);
  }
}

void ezAnimationGraphContext::ApplyBlackboardParams()
{
  if (m_sBlackboardParamsPayload.IsEmpty())
    return;

  EZ_LOCK(m_pWorld->GetWriteMarker());

  ezLocalBlackboardComponent* pBlackboard;
  if (!m_pWorld->TryGetComponent(m_hBlackboardComponent, pBlackboard))
    return;

  ezBlackboard* pBB = pBlackboard->GetBoard().Borrow();
  if (!pBB)
    return;

  // Format: "name1|type1;name2|type2;..."
  ezStringBuilder sPayload = m_sBlackboardParamsPayload;
  ezHybridArray<ezStringView, 16> entries;
  sPayload.Split(false, entries, ";");

  for (const ezStringView& entry : entries)
  {
    ezStringBuilder sEntry(entry);
    ezHybridArray<ezStringView, 2> parts;
    sEntry.Split(false, parts, "|");

    if (parts.GetCount() >= 2)
    {
      ezString sName(parts[0]);
      ezString sType(parts[1]);

      if (sType == "Bool")
        pBB->SetEntryValue(sName, false);
      else
        pBB->SetEntryValue(sName, 0.0);
    }
  }
}
