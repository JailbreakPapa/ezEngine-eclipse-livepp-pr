#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <RendererCore/RendererCoreDLL.h>

/// Marker type indicating a transition should fire when the state's animation finishes.
///
/// Used within AnimGraph state machines as a transition condition type. The runtime
/// StateMachineAnimNode checks for this condition type during transition evaluation.
/// This is a lightweight reflected marker class — it carries no runtime logic itself,
/// as the actual condition evaluation is handled by the StateMachineAnimNode's inline
/// condition system.
class EZ_RENDERERCORE_DLL ezAnimGraphTransition_AnimationFinished : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAnimGraphTransition_AnimationFinished, ezReflectedClass);
};
