#include <RendererCore/RendererCorePCH.h>

#include <Core/Messages/UpdateLocalBoundsMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Lights/VolumetricFogComponent.h>
#include <RendererCore/Pipeline/RenderDataManager.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezVolumetricFogRenderData, 1, ezRTTIDefaultAllocator<ezVolumetricFogRenderData>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_COMPONENT_TYPE(ezVolumetricFogComponent, 1, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("Density", GetDensity, SetDensity)->AddAttributes(new ezClampValueAttribute(0.0f, ezVariant()), new ezDefaultValueAttribute(0.5f)),
    EZ_ACCESSOR_PROPERTY("Anisotropy", GetAnisotropy, SetAnisotropy)->AddAttributes(new ezClampValueAttribute(-0.99f, 0.99f), new ezDefaultValueAttribute(0.3f)),
    EZ_ACCESSOR_PROPERTY("HeightFalloff", GetHeightFalloff, SetHeightFalloff)->AddAttributes(new ezClampValueAttribute(0.0f, ezVariant()), new ezDefaultValueAttribute(0.1f)),
    EZ_ACCESSOR_PROPERTY("Albedo", GetAlbedo, SetAlbedo)->AddAttributes(new ezDefaultValueAttribute(ezColor::White)),
    EZ_ACCESSOR_PROPERTY("AmbientLight", GetAmbientLight, SetAmbientLight)->AddAttributes(new ezDefaultValueAttribute(ezColorGammaUB(ezColor(0.05f, 0.05f, 0.08f)))),
    EZ_ACCESSOR_PROPERTY("StartDistance", GetStartDistance, SetStartDistance)->AddAttributes(new ezClampValueAttribute(0.0f, ezVariant())),
    EZ_ACCESSOR_PROPERTY("NearPlane", GetNearPlane, SetNearPlane)->AddAttributes(new ezClampValueAttribute(0.1f, ezVariant()), new ezDefaultValueAttribute(0.5f)),
    EZ_ACCESSOR_PROPERTY("FarPlane", GetFarPlane, SetFarPlane)->AddAttributes(new ezClampValueAttribute(1.0f, ezVariant()), new ezDefaultValueAttribute(500.0f)),
    EZ_ACCESSOR_PROPERTY("TemporalBlendWeight", GetTemporalBlendWeight, SetTemporalBlendWeight)->AddAttributes(new ezClampValueAttribute(0.01f, 1.0f), new ezDefaultValueAttribute(0.05f)),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezMsgUpdateLocalBounds, OnUpdateLocalBounds),
    EZ_MESSAGE_HANDLER(ezMsgExtractRenderData, OnMsgExtractRenderData),
  }
  EZ_END_MESSAGEHANDLERS;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Effects"),
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
  GetOwner()->UpdateLocalBounds();
}

void ezVolumetricFogComponent::OnDeactivated()
{
  GetOwner()->UpdateLocalBounds();
}

void ezVolumetricFogComponent::SetDensity(float fDensity)
{
  m_fDensity = ezMath::Max(fDensity, 0.0f);
  if (IsActiveAndInitialized())
    ezRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());
}

float ezVolumetricFogComponent::GetDensity() const { return m_fDensity; }

void ezVolumetricFogComponent::SetAnisotropy(float fAnisotropy)
{
  m_fAnisotropy = ezMath::Clamp(fAnisotropy, -0.99f, 0.99f);
  if (IsActiveAndInitialized())
    ezRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());
}

float ezVolumetricFogComponent::GetAnisotropy() const { return m_fAnisotropy; }

void ezVolumetricFogComponent::SetHeightFalloff(float fHeightFalloff)
{
  m_fHeightFalloff = ezMath::Max(fHeightFalloff, 0.0f);
  if (IsActiveAndInitialized())
    ezRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());
}

float ezVolumetricFogComponent::GetHeightFalloff() const { return m_fHeightFalloff; }

void ezVolumetricFogComponent::SetAlbedo(ezColor color)
{
  m_Albedo = color;
  if (IsActiveAndInitialized())
    ezRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());
}

ezColor ezVolumetricFogComponent::GetAlbedo() const { return m_Albedo; }

void ezVolumetricFogComponent::SetAmbientLight(ezColor color)
{
  m_AmbientLight = color;
  if (IsActiveAndInitialized())
    ezRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());
}

ezColor ezVolumetricFogComponent::GetAmbientLight() const { return m_AmbientLight; }

void ezVolumetricFogComponent::SetStartDistance(float fDistance)
{
  m_fStartDistance = ezMath::Max(fDistance, 0.0f);
  if (IsActiveAndInitialized())
    ezRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());
}

float ezVolumetricFogComponent::GetStartDistance() const { return m_fStartDistance; }

void ezVolumetricFogComponent::SetNearPlane(float fNear)
{
  m_fNearPlane = ezMath::Max(fNear, 0.1f);
  if (IsActiveAndInitialized())
    ezRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());
}

float ezVolumetricFogComponent::GetNearPlane() const { return m_fNearPlane; }

void ezVolumetricFogComponent::SetFarPlane(float fFar)
{
  m_fFarPlane = ezMath::Max(fFar, 1.0f);
  if (IsActiveAndInitialized())
    ezRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());
}

float ezVolumetricFogComponent::GetFarPlane() const { return m_fFarPlane; }

void ezVolumetricFogComponent::SetTemporalBlendWeight(float fWeight)
{
  m_fTemporalBlendWeight = ezMath::Clamp(fWeight, 0.01f, 1.0f);
  if (IsActiveAndInitialized())
    ezRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());
}

float ezVolumetricFogComponent::GetTemporalBlendWeight() const { return m_fTemporalBlendWeight; }

void ezVolumetricFogComponent::OnUpdateLocalBounds(ezMsgUpdateLocalBounds& msg)
{
  msg.SetAlwaysVisible(GetOwner()->IsDynamic() ? ezDefaultSpatialDataCategories::RenderDynamic : ezDefaultSpatialDataCategories::RenderStatic);
}

void ezVolumetricFogComponent::OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const
{
  if (msg.m_OverrideCategory != ezInvalidRenderDataCategory)
    return;

  auto pRenderData = msg.m_pRenderDataManager->CreateRenderDataForThisFrame<ezVolumetricFogRenderData>(GetOwner());

  pRenderData->m_fDensity = m_fDensity / 100.0f;
  pRenderData->m_fAnisotropy = m_fAnisotropy;
  pRenderData->m_fBaseHeight = GetOwner()->GetGlobalTransform().m_vPosition.z;
  pRenderData->m_fHeightFalloff = m_fHeightFalloff;
  pRenderData->m_fStartDistance = m_fStartDistance;
  pRenderData->m_fNearPlane = m_fNearPlane;
  pRenderData->m_fFarPlane = m_fFarPlane;
  pRenderData->m_Albedo = m_Albedo;
  pRenderData->m_AmbientLight = m_AmbientLight;
  pRenderData->m_fTemporalBlendWeight = m_fTemporalBlendWeight;

  msg.AddRenderData(pRenderData, ezDefaultRenderDataCategories::Light, ezRenderData::Caching::IfStatic);
}

void ezVolumetricFogComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  ezStreamWriter& s = inout_stream.GetStream();

  s << m_fDensity;
  s << m_fAnisotropy;
  s << m_fHeightFalloff;
  s << m_Albedo;
  s << m_AmbientLight;
  s << m_fStartDistance;
  s << m_fNearPlane;
  s << m_fFarPlane;
  s << m_fTemporalBlendWeight;
}

void ezVolumetricFogComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  // const ezUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  ezStreamReader& s = inout_stream.GetStream();

  s >> m_fDensity;
  s >> m_fAnisotropy;
  s >> m_fHeightFalloff;
  s >> m_Albedo;
  s >> m_AmbientLight;
  s >> m_fStartDistance;
  s >> m_fNearPlane;
  s >> m_fFarPlane;
  s >> m_fTemporalBlendWeight;
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_VolumetricFogComponent);
