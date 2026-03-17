#pragma once

#include <RendererCore/Lights/LightComponent.h>

using ezTubeLightComponentManager = ezComponentManager<class ezTubeLightComponent, ezBlockStorageType::Compact>;

/// Render data for tube (capsule) area lights.
class EZ_RENDERERCORE_DLL ezTubeLightRenderData : public ezLightRenderData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezTubeLightRenderData, ezLightRenderData);

public:
  float m_fLength;
  float m_fRadius;
  float m_fRange;
  ezQuat m_qGlobalRotation;
};

/// Tube (capsule) area light that emits light from a cylindrical shape.
///
/// The tube axis is oriented along the component's local Y axis.
/// Shadows are approximated by treating the light as a point light at its center.
class EZ_RENDERERCORE_DLL ezTubeLightComponent : public ezLightComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezTubeLightComponent, ezLightComponent, ezTubeLightComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(ezWorldReader& inout_stream) override;

  //////////////////////////////////////////////////////////////////////////
  // ezRenderComponent

public:
  virtual ezResult GetLocalBounds(ezBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, ezMsgUpdateLocalBounds& ref_msg) override;

  //////////////////////////////////////////////////////////////////////////
  // ezTubeLightComponent

public:
  ezTubeLightComponent();
  ~ezTubeLightComponent();

  void SetLength(float fLength); // [ property ]
  float GetLength() const;      // [ property ]

  /// \brief Radius of the tube's cross-section. Affects the size of specular highlights.
  void SetRadius(float fRadius); // [ property ]
  float GetRadius() const;      // [ property ]

  /// \brief Sets the light range. If zero, the range is automatically determined from the intensity.
  void SetRange(float fRange);   // [ property ]
  float GetRange() const;        // [ property ]

  float GetEffectiveRange() const;

  void SetShadowFadeOutRange(float fRange); // [ property ]
  float GetShadowFadeOutRange() const;      // [ property ]

protected:
  void OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const;

  float m_fLength = 1.0f;
  float m_fRadius = 0.05f;
  float m_fRange = 0.0f;
  float m_fEffectiveRange = 0.0f;
  float m_fShadowFadeOutRange = 0.0f;
};

/// Visualizer attribute for tube (capsule) area lights.
///
/// Shows a wireframe capsule with the tube's dimensions and range in the editor.
class EZ_RENDERERCORE_DLL ezTubeLightVisualizerAttribute : public ezVisualizerAttribute
{
  EZ_ADD_DYNAMIC_REFLECTION(ezTubeLightVisualizerAttribute, ezVisualizerAttribute);

public:
  ezTubeLightVisualizerAttribute();
  ezTubeLightVisualizerAttribute(
    const char* szLengthProperty, const char* szRadiusProperty, const char* szRangeProperty, const char* szIntensityProperty, const char* szColorProperty);

  const ezUntrackedString& GetLengthProperty() const { return m_sProperty1; }
  const ezUntrackedString& GetRadiusProperty() const { return m_sProperty2; }
  const ezUntrackedString& GetRangeProperty() const { return m_sProperty3; }
  const ezUntrackedString& GetIntensityProperty() const { return m_sProperty4; }
  const ezUntrackedString& GetColorProperty() const { return m_sProperty5; }
};
