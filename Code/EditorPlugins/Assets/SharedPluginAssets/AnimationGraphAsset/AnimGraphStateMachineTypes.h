#pragma once

#include <SharedPluginAssets/SharedPluginAssetsDLL.h>

#include <GameEngine/StateMachine/StateMachine.h>
#include <ToolsFoundation/VisualGraph/VisualGraphObjectManager.h>

/// Base class for state and any-state nodes within an AnimGraph state machine.
class EZ_SHAREDPLUGINASSETS_DLL ezAnimGraphStateNodeBase : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAnimGraphStateNodeBase, ezReflectedClass);
};

/// A state within an AnimGraph state machine.
///
/// Each state has a name and references a sub-graph (blend tree) that produces the animation pose
/// when this state is active. The sub-graph is stored as scoped child nodes in the document model.
class EZ_SHAREDPLUGINASSETS_DLL ezAnimGraphStateNode : public ezAnimGraphStateNodeBase
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAnimGraphStateNode, ezAnimGraphStateNodeBase);

public:
  ezString m_sName;
  bool m_bIsInitialState = false;
};

/// "Any state" pseudo-node within an AnimGraph state machine.
///
/// Transitions from this node fire regardless of the currently active state,
/// enabling global transitions without duplicating connections from every state.
class EZ_SHAREDPLUGINASSETS_DLL ezAnimGraphAnyStateNode : public ezAnimGraphStateNodeBase
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAnimGraphAnyStateNode, ezAnimGraphStateNodeBase);
};

/// Connection between states in an AnimGraph state machine.
///
/// Carries transition configuration: blend duration and an optional transition condition.
/// Users can switch the condition type in the property panel (blackboard conditions, timeout, etc.).
class EZ_SHAREDPLUGINASSETS_DLL ezAnimGraphTransitionConnection : public ezDocumentObject_ConnectionBase
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAnimGraphTransitionConnection, ezDocumentObject_ConnectionBase);

public:
  ezTime m_BlendDuration = ezTime::MakeFromMilliseconds(200);
  ezStateMachineTransition* m_pTransitionCondition = nullptr;
};
