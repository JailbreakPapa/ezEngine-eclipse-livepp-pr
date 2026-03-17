#include <RendererCore/RendererCorePCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Lights/Implementation/ShadowPool.h>
#include <RendererCore/Lights/TubeLightComponent.h>
#include <RendererCore/Pipeline/RenderDataManager.h>
#include <RendererCore/Pipeline/View.h>

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
extern ezCVarBool cvar_RenderingLightingVisScreenSpaceSize;
#endif

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezTubeLightRenderData, 1, ezRTTIDefaultAllocator<ezTubeLightRenderData>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_COMPONENT_TYPE(ezTubeLightComponent, 2, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("Length", GetLength, SetLength)->AddAttributes(new ezClampValueAttribute(0.01f, ezVariant()), new ezDefaultValueAttribute(1.0f), new ezSuffixAttribute(" m")),
    EZ_ACCESSOR_PROPERTY("Radius", GetRadius, SetRadius)->AddAttributes(new ezClampValueAttribute(0.001f, ezVariant()), new ezDefaultValueAttribute(0.05f), new ezSuffixAttribute(" m")),
    EZ_ACCESSOR_PROPERTY("Range", GetRange, SetRange)->AddAttributes(new ezClampValueAttribute(0.0f, ezVariant()), new ezSuffixAttribute(" m"), new ezMinValueTextAttribute("Auto")),
    EZ_ACCESSOR_PROPERTY("ShadowFadeOutRange", GetShadowFadeOutRange, SetShadowFadeOutRange)->AddAttributes(new ezClampValueAttribute(0.0f, ezVariant()), new ezSuffixAttribute(" m"), new ezMinValueTextAttribute("Auto")),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezMsgExtractRenderData, OnMsgExtractRenderData),
  }
  EZ_END_MESSAGEHANDLERS;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezSphereManipulatorAttribute("Range"),
    new ezTubeLightVisualizerAttribute("Length", "Radius", "Range", "Intensity", "LightColor"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_COMPONENT_TYPE
// clang-format on

ezTubeLightComponent::ezTubeLightComponent() = default;
ezTubeLightComponent::~ezTubeLightComponent() = default;

ezResult ezTubeLightComponent::GetLocalBounds(ezBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, ezMsgUpdateLocalBounds& ref_msg)
{
  m_fEffectiveRange = CalculateEffectiveRange(m_fRange, m_fIntensity);

  // Bounding sphere must encompass the tube endpoints plus the attenuation range
  const float fBoundingRadius = m_fEffectiveRange + m_fLength * 0.5f;
  ref_bounds = ezBoundingSphere::MakeFromCenterAndRadius(ezVec3::MakeZero(), fBoundingRadius);
  return EZ_SUCCESS;
}

void ezTubeLightComponent::SetLength(float fLength)
{
  m_fLength = ezMath::Max(fLength, 0.01f);
  TriggerLocalBoundsUpdate();
}

float ezTubeLightComponent::GetLength() const
{
  return m_fLength;
}

void ezTubeLightComponent::SetRadius(float fRadius)
{
  m_fRadius = ezMath::Max(fRadius, 0.001f);
  InvalidateCachedRenderData();
}

float ezTubeLightComponent::GetRadius() const
{
  return m_fRadius;
}

void ezTubeLightComponent::SetRange(float fRange)
{
  m_fRange = ezMath::Max(fRange, 0.0f);
  TriggerLocalBoundsUpdate();
}

float ezTubeLightComponent::GetRange() const
{
  return m_fRange;
}

float ezTubeLightComponent::GetEffectiveRange() const
{
  return m_fEffectiveRange;
}

void ezTubeLightComponent::SetShadowFadeOutRange(float fRange)
{
  m_fShadowFadeOutRange = ezMath::Max(fRange, 0.0f);
  InvalidateCachedRenderData();
}

float ezTubeLightComponent::GetShadowFadeOutRange() const
{
  return m_fShadowFadeOutRange;
}

void ezTubeLightComponent::OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const
{
  // Don't extract light render data for selection or in shadow views.
  if (msg.m_OverrideCategory != ezInvalidRenderDataCategory || msg.m_pView->GetCameraUsageHint() == ezCameraUsageHint::Shadow)
    return;

  if (m_fIntensity <= 0.0f || m_fEffectiveRange <= 0.0f)
    return;

  const ezTransform t = GetOwner()->GetGlobalTransform();
  const float fBoundingRadius = m_fEffectiveRange + m_fLength * 0.5f;
  const ezBoundingSphere bounds = ezBoundingSphere::MakeFromCenterAndRadius(t.m_vPosition, fBoundingRadius);
  const float fScreenSpaceSize = CalculateScreenSpaceSize(bounds, *msg.m_pView->GetCullingCamera());
  float fShadowScreenSize = 0.0f;
  const float fShadowFadeOut = CalculateShadowFadeOut(bounds, m_fShadowFadeOutRange, *msg.m_pView->GetCullingCamera(), fShadowScreenSize);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  VisualizeScreenSpaceSize(msg.m_pView->GetHandle(), bounds, fScreenSpaceSize, fShadowScreenSize, fShadowFadeOut);

  if (cvar_RenderingLightingVisScreenSpaceSize)
  {
    // Draw the tube axis and endpoints
    const ezVec3 vAxis = t.m_qRotation * ezVec3::MakeAxisY();
    const float halfLen = m_fLength * 0.5f;
    const ezVec3 endA = t.m_vPosition - vAxis * halfLen;
    const ezVec3 endB = t.m_vPosition + vAxis * halfLen;

    ezColor c = ezColorScheme::LightUI(ezColorScheme::Yellow);

    ezDebugRendererLine lines[3];
    // Tube axis
    lines[0] = ezDebugRendererLine(endA, endB);
    // Endpoint crosses perpendicular to axis
    const ezVec3 vPerp = t.m_qRotation * ezVec3::MakeAxisX();
    lines[1] = ezDebugRendererLine(endA - vPerp * m_fRadius, endA + vPerp * m_fRadius);
    lines[2] = ezDebugRendererLine(endB - vPerp * m_fRadius, endB + vPerp * m_fRadius);

    ezDebugRenderer::DrawLines(msg.m_pView->GetHandle(), ezMakeArrayPtr(lines, 3), c);

    // Draw capsule shape at both endpoints
    ezMat4 capsuleMat;
    capsuleMat.SetIdentity();
    capsuleMat.SetTranslationVector(t.m_vPosition);
    capsuleMat.SetRotationalPart(t.m_qRotation.GetAsMat3());
    // DrawLineCapsuleZ draws along Z; our tube is along Y, so rotate Y->Z
    ezMat3 rotYtoZ = ezMat3::MakeIdentity();
    rotYtoZ.SetColumn(1, ezVec3(0, 0, 1));
    rotYtoZ.SetColumn(2, ezVec3(0, -1, 0));
    capsuleMat.SetRotationalPart(t.m_qRotation.GetAsMat3() * rotYtoZ);

    ezDebugRenderer::DrawLineCapsuleZ(msg.m_pView->GetHandle(), m_fLength, m_fRadius, c, capsuleMat);
  }
#endif

  auto pRenderData = msg.m_pRenderDataManager->CreateRenderDataForThisFrame<ezTubeLightRenderData>(GetOwner());

  pRenderData->m_LightColor = GetEffectiveColor();
  pRenderData->m_fIntensity = m_fIntensity;
  pRenderData->m_fSpecularMultiplier = m_fSpecularMultiplier;
  pRenderData->m_fLength = m_fLength;
  pRenderData->m_fRadius = m_fRadius;
  pRenderData->m_fRange = m_fEffectiveRange;
  pRenderData->m_qGlobalRotation = t.m_qRotation;
  pRenderData->m_fPenumbraSize = m_fPenumbraSize;

  if (m_bCastShadows && fShadowFadeOut > 0.0f)
  {
    pRenderData->FillShadowDataOffsetAndFadeOut(
      ezShadowPool::AddAreaLightAsPointLight(this, m_fEffectiveRange, fScreenSpaceSize, msg.m_pView), fShadowFadeOut);
  }
  else
  {
    pRenderData->m_uiShadowDataOffsetAndFadeOut = 0;
  }

  pRenderData->FillBatchIdAndSortingKey(fScreenSpaceSize);

  ezRenderData::Caching::Enum caching = m_bCastShadows ? ezRenderData::Caching::Never : ezRenderData::Caching::IfStatic;
  msg.AddRenderData(pRenderData, ezDefaultRenderDataCategories::Light, caching);
}

void ezTubeLightComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  ezStreamWriter& s = inout_stream.GetStream();

  s << m_fLength;
  s << m_fRadius;
  s << m_fRange;
  s << m_fShadowFadeOutRange;
}

void ezTubeLightComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const ezUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  ezStreamReader& s = inout_stream.GetStream();

  s >> m_fLength;
  s >> m_fRadius;
  s >> m_fRange;
  if (uiVersion >= 2)
  {
    s >> m_fShadowFadeOutRange;
  }
}

//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezTubeLightVisualizerAttribute, 1, ezRTTIDefaultAllocator<ezTubeLightVisualizerAttribute>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

ezTubeLightVisualizerAttribute::ezTubeLightVisualizerAttribute()
  : ezVisualizerAttribute(nullptr)
{
}

ezTubeLightVisualizerAttribute::ezTubeLightVisualizerAttribute(
  const char* szLengthProperty, const char* szRadiusProperty, const char* szRangeProperty, const char* szIntensityProperty, const char* szColorProperty)
  : ezVisualizerAttribute(szLengthProperty, szRadiusProperty, szRangeProperty, szIntensityProperty, szColorProperty)
{
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_TubeLightComponent);
