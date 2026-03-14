#include <SharedPluginAssets/SharedPluginAssetsPCH.h>

#include <SharedPluginAssets/AnimationGraphAsset/AnimGraphStateMachineTypes.h>

// clang-format off

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAnimGraphStateNodeBase, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAnimGraphStateNode, 1, ezRTTIDefaultAllocator<ezAnimGraphStateNode>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Name", m_sName)->AddAttributes(new ezDefaultValueAttribute(ezStringView("State"))),
    EZ_MEMBER_PROPERTY("IsInitialState", m_bIsInitialState)->AddAttributes(new ezHiddenAttribute()),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAnimGraphAnyStateNode, 1, ezRTTIDefaultAllocator<ezAnimGraphAnyStateNode>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAnimGraphTransitionConnection, 1, ezRTTIDefaultAllocator<ezAnimGraphTransitionConnection>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("BlendDuration", m_BlendDuration)->AddAttributes(
      new ezDefaultValueAttribute(ezTime::MakeFromMilliseconds(200)),
      new ezClampValueAttribute(ezTime::MakeZero(), ezTime::MakeFromSeconds(10))),
    EZ_MEMBER_PROPERTY("TransitionCondition", m_pTransitionCondition)->AddFlags(ezPropertyFlags::PointerOwner),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

// clang-format on
