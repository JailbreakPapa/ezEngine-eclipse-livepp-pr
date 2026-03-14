#pragma once

#include <Core/Utils/Blackboard.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Strings/HashedString.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

/// Condition operator for combining multiple transition conditions.
struct EZ_RENDERERCORE_DLL ezAnimGraphTransitionLogicOperator
{
  using StorageType = ezUInt8;

  enum Enum
  {
    And,
    Or,

    Default = And,
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_RENDERERCORE_DLL, ezAnimGraphTransitionLogicOperator);

/// How a single transition condition is evaluated.
struct EZ_RENDERERCORE_DLL ezAnimGraphTransitionConditionType
{
  using StorageType = ezUInt8;

  enum Enum
  {
    BlackboardBool,            ///< Check a blackboard bool entry (true = condition met)
    BlackboardNumberComparison, ///< Compare a blackboard number against a reference value
    TimeInState,               ///< Elapsed time in the current state exceeds a threshold
    TransitionEvent,           ///< A named transition event was fired this frame

    Default = BlackboardBool,
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_RENDERERCORE_DLL, ezAnimGraphTransitionConditionType);

/// A single condition that must be satisfied for a state machine transition to fire.
struct EZ_RENDERERCORE_DLL ezAnimGraphTransitionCondition
{
  ezEnum<ezAnimGraphTransitionConditionType> m_Type;
  ezHashedString m_sParameter;
  ezEnum<ezComparisonOperator> m_Comparison;
  double m_fReferenceValue = 0.0;
  ezTime m_Timeout;

  ezResult Serialize(ezStreamWriter& inout_stream) const;
  ezResult Deserialize(ezStreamReader& inout_stream);
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_RENDERERCORE_DLL, ezAnimGraphTransitionCondition);

/// A transition between states in an animation graph state machine.
///
/// Transitions define which state to move to and under what conditions. Multiple conditions
/// can be combined with AND/OR logic. Transitions with m_uiFromStateIndex set to ezInvalidIndex
/// act as "any state" transitions that can fire from any active state.
struct EZ_RENDERERCORE_DLL ezAnimGraphStateTransition
{
  ezUInt32 m_uiFromStateIndex = ezInvalidIndex; ///< ezInvalidIndex means "any state"
  ezUInt32 m_uiToStateIndex = 0;
  ezTime m_BlendDuration = ezTime::MakeFromMilliseconds(200);
  ezEnum<ezAnimGraphTransitionLogicOperator> m_ConditionOperator;
  ezHybridArray<ezAnimGraphTransitionCondition, 2> m_Conditions;

  ezResult Serialize(ezStreamWriter& inout_stream) const;
  ezResult Deserialize(ezStreamReader& inout_stream);
};

/// A state within an animation graph state machine.
///
/// Each state owns a sub-graph (blend tree) that produces a local pose when the state is active.
/// Only the current state (and possibly the transition target during blends) has its sub-graph
/// evaluated each frame.
struct EZ_RENDERERCORE_DLL ezAnimGraphSmState
{
  ezHashedString m_sName;
  bool m_bIsInitialState = false;
  ezAnimGraph m_SubGraph;

  ezResult Serialize(ezStreamWriter& inout_stream) const;
  ezResult Deserialize(ezStreamReader& inout_stream);
};

/// Animation graph node that runs an internal state machine with blend tree sub-graphs.
///
/// This node is the core building block for hierarchical animation graphs. It contains a set of
/// states, each with its own blend tree (ezAnimGraph). Transitions between states are driven by
/// blackboard conditions, time thresholds, or named events. During transitions, poses from the
/// outgoing and incoming states are cross-faded.
///
/// In the parent graph, this node exposes a single local pose output pin. Internally, only the
/// active state's sub-graph is evaluated each frame (plus the transition target during blends).
///
/// State machines can be nested: a state's sub-graph can itself contain another
/// ezStateMachineAnimNode.
class EZ_RENDERERCORE_DLL ezStateMachineAnimNode : public ezAnimGraphNode
{
  EZ_ADD_DYNAMIC_REFLECTION(ezStateMachineAnimNode, ezAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // ezAnimGraphNode

protected:
  virtual ezResult SerializeNode(ezStreamWriter& stream) const override;
  virtual ezResult DeserializeNode(ezStreamReader& stream) override;

  virtual void Step(ezAnimController& ref_controller, ezAnimGraphInstance& ref_graph, ezTime tDiff, const ezSkeletonResource* pSkeleton, ezGameObject* pTarget) const override;
  virtual bool GetInstanceDataDesc(ezInstanceDataDesc& out_desc) const override;

  //////////////////////////////////////////////////////////////////////////
  // ezStateMachineAnimNode

public:
  ezStateMachineAnimNode();
  ~ezStateMachineAnimNode();

  /// Returns the number of states.
  ezUInt32 GetNumStates() const;

  /// Returns the state at the given index. The pointer is valid for the lifetime of this node.
  ezAnimGraphSmState& GetState(ezUInt32 uiIndex);
  const ezAnimGraphSmState& GetState(ezUInt32 uiIndex) const;

  /// Adds a new state and returns a reference to it.
  ezAnimGraphSmState& AddState();

  ezDynamicArray<ezAnimGraphStateTransition> m_Transitions;

private:
  ezAnimGraphLocalPoseOutputPin m_OutPose; // [ property ]

  struct StateStorage;
  ezUniquePtr<StateStorage> m_pStates;

  struct InstanceData
  {
    ezUInt32 m_uiCurrentStateIndex = 0;
    ezUInt32 m_uiNextStateIndex = ezInvalidIndex;
    ezTime m_TimeInCurrentState;
    ezTime m_TransitionElapsed;
    ezTime m_TransitionDuration;
    ezDeque<ezUniquePtr<ezAnimGraphInstance>> m_SubGraphInstances;
    bool m_bInitialized = false;
  };

  void InitializeInstanceData(ezAnimController& ref_controller, InstanceData* pInstance, const ezSkeletonResource* pSkeleton) const;
  bool EvaluateCondition(const ezAnimGraphTransitionCondition& cond, ezAnimController& ref_controller, const InstanceData* pInstance) const;
  bool EvaluateTransitionConditions(const ezAnimGraphStateTransition& transition, ezAnimController& ref_controller, const InstanceData* pInstance) const;
  ezUInt32 FindInitialStateIndex() const;
};
