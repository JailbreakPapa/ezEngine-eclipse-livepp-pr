#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Math/CurveFunctions.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/Nodes/StateMachine/StateMachineAnimNode.h>
#include <RendererCore/AnimationSystem/AnimPoseGenerator.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>

// clang-format off

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezAnimGraphTransitionLogicOperator, 1)
  EZ_ENUM_CONSTANTS(ezAnimGraphTransitionLogicOperator::And, ezAnimGraphTransitionLogicOperator::Or)
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezAnimGraphTransitionConditionType, 1)
  EZ_ENUM_CONSTANTS(ezAnimGraphTransitionConditionType::BlackboardBool, ezAnimGraphTransitionConditionType::BlackboardNumberComparison, ezAnimGraphTransitionConditionType::TimeInState, ezAnimGraphTransitionConditionType::TransitionEvent)
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_STATIC_REFLECTED_TYPE(ezAnimGraphTransitionCondition, ezNoBase, 1, ezRTTIDefaultAllocator<ezAnimGraphTransitionCondition>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ENUM_MEMBER_PROPERTY("Type", ezAnimGraphTransitionConditionType, m_Type),
    EZ_MEMBER_PROPERTY("Parameter", m_sParameter),
    EZ_ENUM_MEMBER_PROPERTY("Comparison", ezComparisonOperator, m_Comparison),
    EZ_MEMBER_PROPERTY("ReferenceValue", m_fReferenceValue),
    EZ_MEMBER_PROPERTY("Timeout", m_Timeout),
  }
  EZ_END_PROPERTIES;
}
EZ_END_STATIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezStateMachineAnimNode, 1, ezRTTIDefaultAllocator<ezStateMachineAnimNode>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("OutPose", m_OutPose)->AddAttributes(new ezHiddenAttribute()),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("State Machine"),
    new ezColorAttribute(ezColorScheme::DarkUI(ezColorScheme::Teal)),
    new ezTitleAttribute("State Machine"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

// clang-format on

//////////////////////////////////////////////////////////////////////////
// StateStorage — defined in cpp to avoid non-copyable container issues in the header

struct ezStateMachineAnimNode::StateStorage
{
  ezDeque<ezAnimGraphSmState> m_States;
};

//////////////////////////////////////////////////////////////////////////
// ezAnimGraphTransitionCondition

ezResult ezAnimGraphTransitionCondition::Serialize(ezStreamWriter& inout_stream) const
{
  inout_stream << m_Type;
  inout_stream << m_sParameter;
  inout_stream << m_Comparison;
  inout_stream << m_fReferenceValue;
  inout_stream << m_Timeout;

  return EZ_SUCCESS;
}

ezResult ezAnimGraphTransitionCondition::Deserialize(ezStreamReader& inout_stream)
{
  inout_stream >> m_Type;
  inout_stream >> m_sParameter;
  inout_stream >> m_Comparison;
  inout_stream >> m_fReferenceValue;
  inout_stream >> m_Timeout;

  return EZ_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// ezAnimGraphStateTransition

ezResult ezAnimGraphStateTransition::Serialize(ezStreamWriter& inout_stream) const
{
  inout_stream << m_uiFromStateIndex;
  inout_stream << m_uiToStateIndex;
  inout_stream << m_BlendDuration;
  inout_stream << m_ConditionOperator;

  const ezUInt32 uiNumConditions = m_Conditions.GetCount();
  inout_stream << uiNumConditions;

  for (const auto& cond : m_Conditions)
  {
    EZ_SUCCEED_OR_RETURN(cond.Serialize(inout_stream));
  }

  return EZ_SUCCESS;
}

ezResult ezAnimGraphStateTransition::Deserialize(ezStreamReader& inout_stream)
{
  inout_stream >> m_uiFromStateIndex;
  inout_stream >> m_uiToStateIndex;
  inout_stream >> m_BlendDuration;
  inout_stream >> m_ConditionOperator;

  ezUInt32 uiNumConditions = 0;
  inout_stream >> uiNumConditions;
  m_Conditions.SetCount(uiNumConditions);

  for (auto& cond : m_Conditions)
  {
    EZ_SUCCEED_OR_RETURN(cond.Deserialize(inout_stream));
  }

  return EZ_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// ezAnimGraphSmState

ezResult ezAnimGraphSmState::Serialize(ezStreamWriter& inout_stream) const
{
  inout_stream << m_sName;
  inout_stream << m_bIsInitialState;
  EZ_SUCCEED_OR_RETURN(m_SubGraph.Serialize(inout_stream));

  return EZ_SUCCESS;
}

ezResult ezAnimGraphSmState::Deserialize(ezStreamReader& inout_stream)
{
  inout_stream >> m_sName;
  inout_stream >> m_bIsInitialState;
  EZ_SUCCEED_OR_RETURN(m_SubGraph.Deserialize(inout_stream));

  return EZ_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// ezStateMachineAnimNode

ezStateMachineAnimNode::ezStateMachineAnimNode()
{
  m_pStates = EZ_DEFAULT_NEW(StateStorage);
}

ezStateMachineAnimNode::~ezStateMachineAnimNode() = default;

ezUInt32 ezStateMachineAnimNode::GetNumStates() const
{
  return m_pStates->m_States.GetCount();
}

ezAnimGraphSmState& ezStateMachineAnimNode::GetState(ezUInt32 uiIndex)
{
  return m_pStates->m_States[uiIndex];
}

const ezAnimGraphSmState& ezStateMachineAnimNode::GetState(ezUInt32 uiIndex) const
{
  return m_pStates->m_States[uiIndex];
}

ezAnimGraphSmState& ezStateMachineAnimNode::AddState()
{
  m_pStates->m_States.PushBack();
  return m_pStates->m_States.PeekBack();
}

ezResult ezStateMachineAnimNode::SerializeNode(ezStreamWriter& stream) const
{
  stream.WriteVersion(1);

  EZ_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  EZ_SUCCEED_OR_RETURN(m_OutPose.Serialize(stream));

  // States
  {
    const ezUInt32 uiNumStates = GetNumStates();
    stream << uiNumStates;

    for (ezUInt32 i = 0; i < uiNumStates; ++i)
    {
      EZ_SUCCEED_OR_RETURN(GetState(i).Serialize(stream));
    }
  }

  // Transitions
  {
    const ezUInt32 uiNumTransitions = m_Transitions.GetCount();
    stream << uiNumTransitions;

    for (const auto& transition : m_Transitions)
    {
      EZ_SUCCEED_OR_RETURN(transition.Serialize(stream));
    }
  }

  return EZ_SUCCESS;
}

ezResult ezStateMachineAnimNode::DeserializeNode(ezStreamReader& stream)
{
  stream.ReadVersion(1);

  EZ_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  EZ_SUCCEED_OR_RETURN(m_OutPose.Deserialize(stream));

  // States
  {
    ezUInt32 uiNumStates = 0;
    stream >> uiNumStates;

    for (ezUInt32 i = 0; i < uiNumStates; ++i)
    {
      auto& state = AddState();
      EZ_SUCCEED_OR_RETURN(state.Deserialize(stream));
    }
  }

  // Transitions
  {
    ezUInt32 uiNumTransitions = 0;
    stream >> uiNumTransitions;
    m_Transitions.SetCount(uiNumTransitions);

    for (auto& transition : m_Transitions)
    {
      EZ_SUCCEED_OR_RETURN(transition.Deserialize(stream));
    }
  }

  return EZ_SUCCESS;
}

ezUInt32 ezStateMachineAnimNode::FindInitialStateIndex() const
{
  const ezUInt32 uiNumStates = GetNumStates();
  for (ezUInt32 i = 0; i < uiNumStates; ++i)
  {
    if (GetState(i).m_bIsInitialState)
      return i;
  }

  // Default to first state if none marked
  return uiNumStates == 0 ? ezInvalidIndex : 0;
}

void ezStateMachineAnimNode::InitializeInstanceData(ezAnimController& ref_controller, InstanceData* pInstance, const ezSkeletonResource* pSkeleton) const
{
  pInstance->m_bInitialized = true;
  pInstance->m_uiCurrentStateIndex = FindInitialStateIndex();
  pInstance->m_uiNextStateIndex = ezInvalidIndex;
  pInstance->m_TimeInCurrentState = ezTime::MakeZero();
  pInstance->m_TransitionElapsed = ezTime::MakeZero();
  pInstance->m_TransitionDuration = ezTime::MakeZero();

  pInstance->m_SubGraphInstances.Clear();

  const ezUInt32 uiNumStates = GetNumStates();
  for (ezUInt32 i = 0; i < uiNumStates; ++i)
  {
    ezAnimGraph& subGraph = const_cast<ezAnimGraph&>(GetState(i).m_SubGraph);
    subGraph.PrepareForUse();

    auto pSubInstance = EZ_DEFAULT_NEW(ezAnimGraphInstance);
    pSubInstance->Configure(GetState(i).m_SubGraph);
    pInstance->m_SubGraphInstances.PushBack(std::move(ezUniquePtr<ezAnimGraphInstance>(pSubInstance)));
  }
}

bool ezStateMachineAnimNode::EvaluateCondition(const ezAnimGraphTransitionCondition& cond, ezAnimController& ref_controller, const InstanceData* pInstance) const
{
  switch (cond.m_Type)
  {
    case ezAnimGraphTransitionConditionType::BlackboardBool:
    {
      const auto& pBB = ref_controller.GetBlackboard();
      if (!pBB)
        return false;

      const ezVariant val = pBB->GetEntryValue(cond.m_sParameter, false);
      if (val.IsA<bool>())
        return val.Get<bool>();

      return false;
    }

    case ezAnimGraphTransitionConditionType::BlackboardNumberComparison:
    {
      const auto& pBB = ref_controller.GetBlackboard();
      if (!pBB)
        return false;

      const ezVariant val = pBB->GetEntryValue(cond.m_sParameter, 0.0);
      const double fVal = val.ConvertTo<double>();
      return ezComparisonOperator::Compare(cond.m_Comparison, fVal, cond.m_fReferenceValue);
    }

    case ezAnimGraphTransitionConditionType::TimeInState:
    {
      return pInstance->m_TimeInCurrentState >= cond.m_Timeout;
    }

    case ezAnimGraphTransitionConditionType::TransitionEvent:
    {
      const auto& pBB = ref_controller.GetBlackboard();
      if (!pBB)
        return false;

      const ezVariant val = pBB->GetEntryValue(cond.m_sParameter, false);
      if (val.IsA<bool>())
        return val.Get<bool>();

      return false;
    }

    default:
      return false;
  }
}

bool ezStateMachineAnimNode::EvaluateTransitionConditions(const ezAnimGraphStateTransition& transition, ezAnimController& ref_controller, const InstanceData* pInstance) const
{
  if (transition.m_Conditions.IsEmpty())
    return false;

  if (transition.m_ConditionOperator == ezAnimGraphTransitionLogicOperator::And)
  {
    for (const auto& cond : transition.m_Conditions)
    {
      if (!EvaluateCondition(cond, ref_controller, pInstance))
        return false;
    }
    return true;
  }
  else // Or
  {
    for (const auto& cond : transition.m_Conditions)
    {
      if (EvaluateCondition(cond, ref_controller, pInstance))
        return true;
    }
    return false;
  }
}

void ezStateMachineAnimNode::Step(ezAnimController& ref_controller, ezAnimGraphInstance& ref_graph, ezTime tDiff, const ezSkeletonResource* pSkeleton, ezGameObject* pTarget) const
{
  if (!m_OutPose.IsConnected() || GetNumStates() == 0)
    return;

  InstanceData* pInstance = ref_graph.GetAnimNodeInstanceData<InstanceData>(*this);

  if (!pInstance->m_bInitialized)
  {
    InitializeInstanceData(ref_controller, pInstance, pSkeleton);

    if (pInstance->m_uiCurrentStateIndex == ezInvalidIndex)
      return;
  }

  const ezUInt32 uiCurrentState = pInstance->m_uiCurrentStateIndex;
  const bool bIsTransitioning = pInstance->m_uiNextStateIndex != ezInvalidIndex;

  // Check transitions (only when not already transitioning)
  if (!bIsTransitioning)
  {
    ezUInt32 uiTargetState = ezInvalidIndex;

    // First check transitions from the current state
    for (const auto& transition : m_Transitions)
    {
      if (transition.m_uiFromStateIndex != uiCurrentState)
        continue;

      if (EvaluateTransitionConditions(transition, ref_controller, pInstance))
      {
        uiTargetState = transition.m_uiToStateIndex;
        pInstance->m_TransitionDuration = transition.m_BlendDuration;
        break;
      }
    }

    // Then check "any state" transitions
    if (uiTargetState == ezInvalidIndex)
    {
      for (const auto& transition : m_Transitions)
      {
        if (transition.m_uiFromStateIndex != ezInvalidIndex)
          continue;

        // Don't transition to self from "any state"
        if (transition.m_uiToStateIndex == uiCurrentState)
          continue;

        if (EvaluateTransitionConditions(transition, ref_controller, pInstance))
        {
          uiTargetState = transition.m_uiToStateIndex;
          pInstance->m_TransitionDuration = transition.m_BlendDuration;
          break;
        }
      }
    }

    if (uiTargetState != ezInvalidIndex && uiTargetState < GetNumStates())
    {
      pInstance->m_uiNextStateIndex = uiTargetState;
      pInstance->m_TransitionElapsed = ezTime::MakeZero();
    }
  }

  // Update time in current state
  pInstance->m_TimeInCurrentState += tDiff;

  // Evaluate sub-graphs
  if (pInstance->m_uiNextStateIndex == ezInvalidIndex)
  {
    // Not transitioning: evaluate only the current state
    if (uiCurrentState < pInstance->m_SubGraphInstances.GetCount())
    {
      pInstance->m_SubGraphInstances[uiCurrentState]->Update(ref_controller, tDiff, pTarget, pSkeleton);
    }
  }
  else
  {
    // Transitioning: evaluate both current and next state sub-graphs
    const ezUInt32 uiNextState = pInstance->m_uiNextStateIndex;

    if (uiCurrentState < pInstance->m_SubGraphInstances.GetCount())
    {
      pInstance->m_SubGraphInstances[uiCurrentState]->Update(ref_controller, tDiff, pTarget, pSkeleton);
    }

    if (uiNextState < pInstance->m_SubGraphInstances.GetCount())
    {
      pInstance->m_SubGraphInstances[uiNextState]->Update(ref_controller, tDiff, pTarget, pSkeleton);
    }

    pInstance->m_TransitionElapsed += tDiff;

    // Check if transition is complete
    if (pInstance->m_TransitionElapsed >= pInstance->m_TransitionDuration)
    {
      pInstance->m_uiCurrentStateIndex = uiNextState;
      pInstance->m_uiNextStateIndex = ezInvalidIndex;
      pInstance->m_TimeInCurrentState = pInstance->m_TransitionElapsed;
      pInstance->m_TransitionElapsed = ezTime::MakeZero();
    }
  }
}

bool ezStateMachineAnimNode::GetInstanceDataDesc(ezInstanceDataDesc& out_desc) const
{
  out_desc.FillFromType<InstanceData>();
  return true;
}


EZ_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_Nodes_StateMachine_StateMachineAnimNode);
