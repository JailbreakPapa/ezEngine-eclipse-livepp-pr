#include <RendererCore/RendererCorePCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Meshes/HairComponent.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezHairComponent, 1, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_RESOURCE_ACCESSOR_PROPERTY("Mesh", GetMesh, SetMesh)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Mesh_Static")),
    EZ_ACCESSOR_PROPERTY("Color", GetColor, SetColor)->AddAttributes(new ezExposeColorAlphaAttribute()),
    EZ_ARRAY_ACCESSOR_PROPERTY("Materials", Materials_GetCount, Materials_GetValue, Materials_SetValue, Materials_Insert, Materials_Remove)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Material")),
    EZ_ACCESSOR_PROPERTY("SortingDepthOffset", GetSortingDepthOffset, SetSortingDepthOffset),
    EZ_ACCESSOR_PROPERTY("WindInfluence", GetWindInfluence, SetWindInfluence)
      ->AddAttributes(new ezDefaultValueAttribute(0.5f), new ezClampValueAttribute(0.0f, 2.0f)),
    EZ_ACCESSOR_PROPERTY("GravityStrength", GetGravityStrength, SetGravityStrength)
      ->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.0f, 5.0f)),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezMsgExtractRenderData, OnMsgExtractRenderData),
  }
  EZ_END_MESSAGEHANDLERS;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Rendering"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_COMPONENT_TYPE;
// clang-format on

ezHairComponent::ezHairComponent()
{
  m_vCustomData = ezVec4(m_fWindInfluence, m_fGravityStrength, 0.0f, 1.0f);
}
ezHairComponent::~ezHairComponent() = default;

void ezHairComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  ezStreamWriter& s = inout_stream.GetStream();

  s << m_fWindInfluence;
  s << m_fGravityStrength;
}

void ezHairComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const ezUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  ezStreamReader& s = inout_stream.GetStream();

  s >> m_fWindInfluence;
  s >> m_fGravityStrength;

  m_vCustomData = ezVec4(m_fWindInfluence, m_fGravityStrength, 0.0f, 1.0f);
}

void ezHairComponent::OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const
{
  // Delegate to base class which handles mesh/material/instance data extraction.
  // Custom data (wind/gravity) is already packed into m_vCustomData by the setters.
  ezMeshComponentBase::OnMsgExtractRenderData(msg);
}

void ezHairComponent::SetWindInfluence(float fInfluence)
{
  m_fWindInfluence = ezMath::Clamp(fInfluence, 0.0f, 2.0f);
  m_vCustomData.x = m_fWindInfluence;
  InvalidateCachedRenderData();
}
float ezHairComponent::GetWindInfluence() const { return m_fWindInfluence; }

void ezHairComponent::SetGravityStrength(float fStrength)
{
  m_fGravityStrength = ezMath::Clamp(fStrength, 0.0f, 5.0f);
  m_vCustomData.y = m_fGravityStrength;
  InvalidateCachedRenderData();
}
float ezHairComponent::GetGravityStrength() const { return m_fGravityStrength; }

EZ_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_HairComponent);
