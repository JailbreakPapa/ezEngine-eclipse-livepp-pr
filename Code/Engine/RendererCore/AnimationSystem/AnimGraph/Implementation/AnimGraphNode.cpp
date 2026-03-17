#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>
#include <RendererCore/AnimationSystem/AnimationClipResource.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAnimGraphNode, 1, ezRTTINoAllocator)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("CustomTitle", GetCustomNodeTitle, SetCustomNodeTitle),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezAnimGraphNode::ezAnimGraphNode() = default;
ezAnimGraphNode::~ezAnimGraphNode() = default;

ezResult ezAnimGraphNode::SerializeNode(ezStreamWriter& stream) const
{
  stream.WriteVersion(2);

  stream << m_uiDebugIndex;

  return EZ_SUCCESS;
}

ezResult ezAnimGraphNode::DeserializeNode(ezStreamReader& stream)
{
  const auto uiVersion = stream.ReadVersion(2);

  if (uiVersion >= 2)
  {
    stream >> m_uiDebugIndex;
  }

  return EZ_SUCCESS;
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_Implementation_AnimGraphNode);
