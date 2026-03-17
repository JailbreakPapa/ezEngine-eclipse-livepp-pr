#include <RendererCore/RendererCorePCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Lights/Implementation/ShadowPool.h>
#include <RendererCore/Lights/RectLightComponent.h>
#include <RendererCore/Pipeline/RenderDataManager.h>
#include <RendererCore/Pipeline/View.h>

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
extern ezCVarBool cvar_RenderingLightingVisScreenSpaceSize;
#endif

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRectLightRenderData, 1, ezRTTIDefaultAllocator<ezRectLightRenderData>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_COMPONENT_TYPE(ezRectLightComponent, 2, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("Width", GetWidth, SetWidth)->AddAttributes(new ezClampValueAttribute(0.01f, ezVariant()), new ezDefaultValueAttribute(1.0f), new ezSuffixAttribute(" m")),
    EZ_ACCESSOR_PROPERTY("Height", GetHeight, SetHeight)->AddAttributes(new ezClampValueAttribute(0.01f, ezVariant()), new ezDefaultValueAttribute(1.0f), new ezSuffixAttribute(" m")),
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
    new ezRectLightVisualizerAttribute("Width", "Height", "Range", "Intensity", "LightColor"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_COMPONENT_TYPE
// clang-format on

ezRectLightComponent::ezRectLightComponent() = default;
ezRectLightComponent::~ezRectLightComponent() = default;

ezResult ezRectLightComponent::GetLocalBounds(ezBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, ezMsgUpdateLocalBounds& ref_msg)
{
  m_fEffectiveRange = CalculateEffectiveRange(m_fRange, m_fIntensity);

  ref_bounds = ezBoundingSphere::MakeFromCenterAndRadius(ezVec3::MakeZero(), m_fEffectiveRange);
  return EZ_SUCCESS;
}

void ezRectLightComponent::SetWidth(float fWidth)
{
  m_fWidth = ezMath::Max(fWidth, 0.01f);
  InvalidateCachedRenderData();
}

float ezRectLightComponent::GetWidth() const
{
  return m_fWidth;
}

void ezRectLightComponent::SetHeight(float fHeight)
{
  m_fHeight = ezMath::Max(fHeight, 0.01f);
  InvalidateCachedRenderData();
}

float ezRectLightComponent::GetHeight() const
{
  return m_fHeight;
}

void ezRectLightComponent::SetRange(float fRange)
{
  m_fRange = ezMath::Max(fRange, 0.0f);
  TriggerLocalBoundsUpdate();
}

float ezRectLightComponent::GetRange() const
{
  return m_fRange;
}

float ezRectLightComponent::GetEffectiveRange() const
{
  return m_fEffectiveRange;
}

void ezRectLightComponent::SetShadowFadeOutRange(float fRange)
{
  m_fShadowFadeOutRange = ezMath::Max(fRange, 0.0f);
  InvalidateCachedRenderData();
}

float ezRectLightComponent::GetShadowFadeOutRange() const
{
  return m_fShadowFadeOutRange;
}

void ezRectLightComponent::OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const
{
  // Don't extract light render data for selection or in shadow views.
  if (msg.m_OverrideCategory != ezInvalidRenderDataCategory || msg.m_pView->GetCameraUsageHint() == ezCameraUsageHint::Shadow)
    return;

  if (m_fIntensity <= 0.0f || m_fEffectiveRange <= 0.0f)
    return;

  const ezTransform t = GetOwner()->GetGlobalTransform();
  const ezBoundingSphere bounds = ezBoundingSphere::MakeFromCenterAndRadius(t.m_vPosition, m_fEffectiveRange);
  const float fScreenSpaceSize = CalculateScreenSpaceSize(bounds, *msg.m_pView->GetCullingCamera());
  float fShadowScreenSize = 0.0f;
  const float fShadowFadeOut = CalculateShadowFadeOut(bounds, m_fShadowFadeOutRange, *msg.m_pView->GetCullingCamera(), fShadowScreenSize);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  VisualizeScreenSpaceSize(msg.m_pView->GetHandle(), bounds, fScreenSpaceSize, fShadowScreenSize, fShadowFadeOut);

  if (cvar_RenderingLightingVisScreenSpaceSize)
  {
    // Draw the emitting rectangle shape
    const ezVec3 vForward = t.m_qRotation * -ezVec3::MakeAxisX();
    const ezVec3 vRight = t.m_qRotation * ezVec3::MakeAxisY();
    const ezVec3 vUp = t.m_qRotation * ezVec3::MakeAxisZ();

    const float hw = m_fWidth * 0.5f;
    const float hh = m_fHeight * 0.5f;

    const ezVec3 corners[4] = {
      t.m_vPosition + vRight * hw + vUp * hh,
      t.m_vPosition - vRight * hw + vUp * hh,
      t.m_vPosition - vRight * hw - vUp * hh,
      t.m_vPosition + vRight * hw - vUp * hh,
    };

    ezColor c = ezColorScheme::LightUI(ezColorScheme::Yellow);

    ezDebugRendererLine lines[6];
    // Rectangle outline
    lines[0] = ezDebugRendererLine(corners[0], corners[1]);
    lines[1] = ezDebugRendererLine(corners[1], corners[2]);
    lines[2] = ezDebugRendererLine(corners[2], corners[3]);
    lines[3] = ezDebugRendererLine(corners[3], corners[0]);
    // Normal arrow from center
    lines[4] = ezDebugRendererLine(t.m_vPosition, t.m_vPosition + vForward * m_fEffectiveRange * 0.25f);
    // Cross on the emitting face
    lines[5] = ezDebugRendererLine(corners[0], corners[2]);

    ezDebugRenderer::DrawLines(msg.m_pView->GetHandle(), ezMakeArrayPtr(lines, 6), c);
  }
#endif

  auto pRenderData = msg.m_pRenderDataManager->CreateRenderDataForThisFrame<ezRectLightRenderData>(GetOwner());

  pRenderData->m_LightColor = GetEffectiveColor();
  pRenderData->m_fIntensity = m_fIntensity;
  pRenderData->m_fSpecularMultiplier = m_fSpecularMultiplier;
  pRenderData->m_fWidth = m_fWidth;
  pRenderData->m_fHeight = m_fHeight;
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

void ezRectLightComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  ezStreamWriter& s = inout_stream.GetStream();

  s << m_fWidth;
  s << m_fHeight;
  s << m_fRange;
  s << m_fShadowFadeOutRange;
}

void ezRectLightComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const ezUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  ezStreamReader& s = inout_stream.GetStream();

  s >> m_fWidth;
  s >> m_fHeight;
  s >> m_fRange;
  if (uiVersion >= 2)
  {
    s >> m_fShadowFadeOutRange;
  }
}

//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRectLightVisualizerAttribute, 1, ezRTTIDefaultAllocator<ezRectLightVisualizerAttribute>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

ezRectLightVisualizerAttribute::ezRectLightVisualizerAttribute()
  : ezVisualizerAttribute(nullptr)
{
}

ezRectLightVisualizerAttribute::ezRectLightVisualizerAttribute(
  const char* szWidthProperty, const char* szHeightProperty, const char* szRangeProperty, const char* szIntensityProperty, const char* szColorProperty)
  : ezVisualizerAttribute(szWidthProperty, szHeightProperty, szRangeProperty, szIntensityProperty, szColorProperty)
{
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_RectLightComponent);
