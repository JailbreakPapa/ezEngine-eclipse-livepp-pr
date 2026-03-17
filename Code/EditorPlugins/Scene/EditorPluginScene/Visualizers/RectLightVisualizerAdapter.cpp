#include <EditorPluginScene/EditorPluginScenePCH.h>

#include <EditorFramework/Assets/AssetDocument.h>
#include <EditorPluginScene/Visualizers/RectLightVisualizerAdapter.h>
#include <RendererCore/Lights/RectLightComponent.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>

ezRectLightVisualizerAdapter::ezRectLightVisualizerAdapter() = default;

ezRectLightVisualizerAdapter::~ezRectLightVisualizerAdapter() = default;

void ezRectLightVisualizerAdapter::Finalize()
{
  auto* pDoc = m_pObject->GetDocumentObjectManager()->GetDocument()->GetMainDocument();
  const ezAssetDocument* pAssetDocument = ezDynamicCast<const ezAssetDocument*>(pDoc);
  EZ_ASSERT_DEV(pAssetDocument != nullptr, "Visualizers are only supported in ezAssetDocument.");

  // Sphere gizmo for the attenuation range
  m_hRangeGizmo.ConfigureHandle(nullptr, ezEngineGizmoHandleType::Sphere, ezColor::White, ezGizmoFlags::ShowInOrtho | ezGizmoFlags::Visualizer);
  pAssetDocument->AddSyncObject(&m_hRangeGizmo);
  m_hRangeGizmo.SetVisible(m_bVisualizerIsVisible);

  // Rectangle gizmo for the emitting face
  m_hRectGizmo.ConfigureHandle(nullptr, ezEngineGizmoHandleType::LineRect, ezColor::White, ezGizmoFlags::ShowInOrtho | ezGizmoFlags::Visualizer);
  pAssetDocument->AddSyncObject(&m_hRectGizmo);
  m_hRectGizmo.SetVisible(m_bVisualizerIsVisible);
}

void ezRectLightVisualizerAdapter::Update()
{
  m_hRangeGizmo.SetVisible(m_bVisualizerIsVisible);
  m_hRectGizmo.SetVisible(m_bVisualizerIsVisible);

  ezObjectAccessorBase* pObjectAccessor = GetObjectAccessor();
  const ezRectLightVisualizerAttribute* pAttr = static_cast<const ezRectLightVisualizerAttribute*>(m_pVisualizerAttr);

  m_fScale = 1.0f;
  m_fWidth = 1.0f;
  m_fHeight = 1.0f;

  if (!pAttr->GetRangeProperty().IsEmpty() && !pAttr->GetIntensityProperty().IsEmpty())
  {
    ezVariant range;
    pObjectAccessor->GetValue(m_pObject, GetProperty(pAttr->GetRangeProperty()), range).AssertSuccess();
    EZ_ASSERT_DEBUG(range.CanConvertTo<float>(), "Invalid property bound to ezRectLightVisualizerAttribute 'range'");

    ezVariant intensity;
    pObjectAccessor->GetValue(m_pObject, GetProperty(pAttr->GetIntensityProperty()), intensity).AssertSuccess();
    EZ_ASSERT_DEBUG(intensity.CanConvertTo<float>(), "Invalid property bound to ezRectLightVisualizerAttribute 'intensity'");

    m_fScale = ezLightComponent::CalculateEffectiveRange(range.ConvertTo<float>(), intensity.ConvertTo<float>());
  }

  if (!pAttr->GetWidthProperty().IsEmpty())
  {
    ezVariant value;
    pObjectAccessor->GetValue(m_pObject, GetProperty(pAttr->GetWidthProperty()), value).AssertSuccess();
    EZ_ASSERT_DEBUG(value.CanConvertTo<float>(), "Invalid property bound to ezRectLightVisualizerAttribute 'width'");
    m_fWidth = value.ConvertTo<float>();
  }

  if (!pAttr->GetHeightProperty().IsEmpty())
  {
    ezVariant value;
    pObjectAccessor->GetValue(m_pObject, GetProperty(pAttr->GetHeightProperty()), value).AssertSuccess();
    EZ_ASSERT_DEBUG(value.CanConvertTo<float>(), "Invalid property bound to ezRectLightVisualizerAttribute 'height'");
    m_fHeight = value.ConvertTo<float>();
  }

  if (!pAttr->GetColorProperty().IsEmpty())
  {
    ezVariant value;
    pObjectAccessor->GetValue(m_pObject, GetProperty(pAttr->GetColorProperty()), value).AssertSuccess();
    EZ_ASSERT_DEBUG(value.IsValid() && value.CanConvertTo<ezColor>(), "Invalid property bound to ezRectLightVisualizerAdapter 'color'");
    m_hRangeGizmo.SetColor(value.ConvertTo<ezColor>());
    m_hRectGizmo.SetColor(value.ConvertTo<ezColor>());
  }
}

void ezRectLightVisualizerAdapter::UpdateGizmoTransform()
{
  // Range sphere
  {
    ezTransform t = GetObjectTransform();
    t.m_vScale *= m_fScale;
    m_hRangeGizmo.SetTransformation(t);
  }

  // Emitting rectangle: rect gizmo is in the YZ plane, origin at center
  {
    ezTransform t = GetObjectTransform();
    t.m_vScale = t.m_vScale.CompMul(ezVec3(1.0f, m_fWidth * 0.5f, m_fHeight * 0.5f));
    m_hRectGizmo.SetTransformation(t);
  }
}
