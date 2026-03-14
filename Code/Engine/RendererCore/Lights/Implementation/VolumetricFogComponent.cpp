#include <RendererCore/RendererCorePCH.h>

#include <Core/Messages/UpdateLocalBoundsMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Lights/VolumetricFogComponent.h>
#include <RendererCore/Pipeline/RenderDataManager.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezVolumetricFogRenderData, 1, ezRTTIDefaultAllocator<ezVolumetricFogRenderData>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_COMPONENT_TYPE(ezVolumetricFogComponent, 2, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("Extents", GetExtents, SetExtents)->AddAttributes(new ezDefaultValueAttribute(ezVec3(10.0f)), new ezClampValueAttribute(ezVec3(0.0f), ezVariant())),
    EZ_ACCESSOR_PROPERTY("Density", GetDensity, SetDensity)->AddAttributes(new ezClampValueAttribute(0.0f, ezVariant()), new ezDefaultValueAttribute(0.05f)),
    EZ_ACCESSOR_PROPERTY("Anisotropy", GetAnisotropy, SetAnisotropy)->AddAttributes(new ezClampValueAttribute(-0.99f, 0.99f), new ezDefaultValueAttribute(0.3f)),
    EZ_ACCESSOR_PROPERTY("HeightFalloff", GetHeightFalloff, SetHeightFalloff)->AddAttributes(new ezClampValueAttribute(0.0f, ezVariant()), new ezDefaultValueAttribute(0.1f)),
    EZ_ACCESSOR_PROPERTY("Albedo", GetAlbedo, SetAlbedo)->AddAttributes(new ezDefaultValueAttribute(ezColor::White)),
    EZ_ACCESSOR_PROPERTY("AmbientLight", GetAmbientLight, SetAmbientLight)->AddAttributes(new ezDefaultValueAttribute(ezColorGammaUB(ezColor(0.15f, 0.15f, 0.2f)))),
    EZ_ACCESSOR_PROPERTY("StartDistance", GetStartDistance, SetStartDistance)->AddAttributes(new ezClampValueAttribute(0.0f, ezVariant())),
    EZ_ACCESSOR_PROPERTY("NearPlane", GetNearPlane, SetNearPlane)->AddAttributes(new ezClampValueAttribute(0.1f, ezVariant()), new ezDefaultValueAttribute(0.5f)),
    EZ_ACCESSOR_PROPERTY("FarPlane", GetFarPlane, SetFarPlane)->AddAttributes(new ezClampValueAttribute(1.0f, ezVariant()), new ezDefaultValueAttribute(500.0f)),
    EZ_ACCESSOR_PROPERTY("TemporalBlendWeight", GetTemporalBlendWeight, SetTemporalBlendWeight)->AddAttributes(new ezClampValueAttribute(0.01f, 1.0f), new ezDefaultValueAttribute(0.05f)),
    EZ_ACCESSOR_PROPERTY("FalloffExponent", GetFalloffExponent, SetFalloffExponent)->AddAttributes(new ezClampValueAttribute(0.1f, 10.0f), new ezDefaultValueAttribute(2.0f)),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezMsgExtractRenderData, OnMsgExtractRenderData),
  }
  EZ_END_MESSAGEHANDLERS;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Effects"),
    new ezBoxVisualizerAttribute("Extents"),
    new ezBoxManipulatorAttribute("Extents", 1.0f, true),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_COMPONENT_TYPE
// clang-format on

ezVolumetricFogComponent::ezVolumetricFogComponent() = default;
ezVolumetricFogComponent::~ezVolumetricFogComponent() = default;

void ezVolumetricFogComponent::Deinitialize()
{
  ezRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());
  SUPER::Deinitialize();
}

void ezVolumetricFogComponent::OnActivated()
{
  SUPER::OnActivated();
}

void ezVolumetricFogComponent::OnDeactivated()
{
  SUPER::OnDeactivated();
}

ezResult ezVolumetricFogComponent::GetLocalBounds(ezBoundingBoxSphere& out_bounds, bool& out_bAlwaysVisible, ezMsgUpdateLocalBounds& ref_msg)
{
  ezVec3 vHalfExtents = m_vExtents * 0.5f;
  out_bounds = ezBoundingBoxSphere::MakeFromBox(ezBoundingBox::MakeFromMinMax(-vHalfExtents, vHalfExtents));
  return EZ_SUCCESS;
}

void ezVolumetricFogComponent::SetExtents(const ezVec3& vExtents)
{
  m_vExtents = vExtents.CompMax(ezVec3::MakeZero());
  TriggerLocalBoundsUpdate();
}

const ezVec3& ezVolumetricFogComponent::GetExtents() const { return m_vExtents; }

void ezVolumetricFogComponent::SetDensity(float fDensity)
{
  m_fDensity = ezMath::Max(fDensity, 0.0f);
  InvalidateCachedRenderData();
}

float ezVolumetricFogComponent::GetDensity() const { return m_fDensity; }

void ezVolumetricFogComponent::SetAnisotropy(float fAnisotropy)
{
  m_fAnisotropy = ezMath::Clamp(fAnisotropy, -0.99f, 0.99f);
  InvalidateCachedRenderData();
}

float ezVolumetricFogComponent::GetAnisotropy() const { return m_fAnisotropy; }

void ezVolumetricFogComponent::SetHeightFalloff(float fHeightFalloff)
{
  m_fHeightFalloff = ezMath::Max(fHeightFalloff, 0.0f);
  InvalidateCachedRenderData();
}

float ezVolumetricFogComponent::GetHeightFalloff() const { return m_fHeightFalloff; }

void ezVolumetricFogComponent::SetAlbedo(ezColor color)
{
  m_Albedo = color;
  InvalidateCachedRenderData();
}

ezColor ezVolumetricFogComponent::GetAlbedo() const { return m_Albedo; }

void ezVolumetricFogComponent::SetAmbientLight(ezColor color)
{
  m_AmbientLight = color;
  InvalidateCachedRenderData();
}

ezColor ezVolumetricFogComponent::GetAmbientLight() const { return m_AmbientLight; }

void ezVolumetricFogComponent::SetStartDistance(float fDistance)
{
  m_fStartDistance = ezMath::Max(fDistance, 0.0f);
  InvalidateCachedRenderData();
}

float ezVolumetricFogComponent::GetStartDistance() const { return m_fStartDistance; }

void ezVolumetricFogComponent::SetNearPlane(float fNear)
{
  m_fNearPlane = ezMath::Max(fNear, 0.1f);
  InvalidateCachedRenderData();
}

float ezVolumetricFogComponent::GetNearPlane() const { return m_fNearPlane; }

void ezVolumetricFogComponent::SetFarPlane(float fFar)
{
  m_fFarPlane = ezMath::Max(fFar, 1.0f);
  InvalidateCachedRenderData();
}

float ezVolumetricFogComponent::GetFarPlane() const { return m_fFarPlane; }

void ezVolumetricFogComponent::SetTemporalBlendWeight(float fWeight)
{
  m_fTemporalBlendWeight = ezMath::Clamp(fWeight, 0.01f, 1.0f);
  InvalidateCachedRenderData();
}

float ezVolumetricFogComponent::GetTemporalBlendWeight() const { return m_fTemporalBlendWeight; }

void ezVolumetricFogComponent::SetFalloffExponent(float fExponent)
{
  m_fFalloffExponent = ezMath::Clamp(fExponent, 0.1f, 10.0f);
  InvalidateCachedRenderData();
}

float ezVolumetricFogComponent::GetFalloffExponent() const { return m_fFalloffExponent; }

void ezVolumetricFogComponent::OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const
{
  if (msg.m_OverrideCategory != ezInvalidRenderDataCategory)
    return;

  if (!IsActiveAndInitialized())
    return;

  auto pRenderData = msg.m_pRenderDataManager->CreateRenderDataForThisFrame<ezVolumetricFogRenderData>(GetOwner());

  pRenderData->m_fDensity = m_fDensity;
  pRenderData->m_fAnisotropy = m_fAnisotropy;
  pRenderData->m_fHeightFalloff = m_fHeightFalloff;
  pRenderData->m_fStartDistance = m_fStartDistance;
  pRenderData->m_fNearPlane = m_fNearPlane;
  pRenderData->m_fFarPlane = m_fFarPlane;
  pRenderData->m_Albedo = m_Albedo;
  pRenderData->m_AmbientLight = m_AmbientLight;
  pRenderData->m_fTemporalBlendWeight = m_fTemporalBlendWeight;
  pRenderData->m_GlobalTransform = GetOwner()->GetGlobalTransform();
  pRenderData->m_vHalfExtents = m_vExtents * 0.5f;

  msg.AddRenderData(pRenderData, ezDefaultRenderDataCategories::Light, ezRenderData::Caching::IfStatic);
}

void ezVolumetricFogComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  ezStreamWriter& s = inout_stream.GetStream();

  s << m_vExtents;
  s << m_fDensity;
  s << m_fAnisotropy;
  s << m_fHeightFalloff;
  s << m_Albedo;
  s << m_AmbientLight;
  s << m_fStartDistance;
  s << m_fNearPlane;
  s << m_fFarPlane;
  s << m_fTemporalBlendWeight;
  s << m_fFalloffExponent;
}

void ezVolumetricFogComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const ezUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  ezStreamReader& s = inout_stream.GetStream();

  if (uiVersion >= 2)
  {
    s >> m_vExtents;
  }

  s >> m_fDensity;
  s >> m_fAnisotropy;
  s >> m_fHeightFalloff;
  s >> m_Albedo;
  s >> m_AmbientLight;
  s >> m_fStartDistance;
  s >> m_fNearPlane;
  s >> m_fFarPlane;
  s >> m_fTemporalBlendWeight;

  if (uiVersion >= 2)
  {
    s >> m_fFalloffExponent;
  }
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_VolumetricFogComponent);
