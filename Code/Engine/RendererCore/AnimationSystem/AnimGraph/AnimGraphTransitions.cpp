#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphTransitions.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAnimGraphTransition_AnimationFinished, 1, ezRTTIDefaultAllocator<ezAnimGraphTransition_AnimationFinished>)
{
  EZ_BEGIN_ATTRIBUTES
  {
    new ezTitleAttribute("Animation Finished"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

EZ_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_Implementation_AnimGraphTransitions);
