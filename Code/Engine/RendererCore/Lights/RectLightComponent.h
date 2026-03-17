#pragma once

#include <RendererCore/Lights/LightComponent.h>

using ezRectLightComponentManager = ezComponentManager<class ezRectLightComponent, ezBlockStorageType::Compact>;

/// Render data for rectangular area lights.
class EZ_RENDERERCORE_DLL ezRectLightRenderData : public ezLightRenderData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRectLightRenderData, ezLightRenderData);

public:
  float m_fWidth;
  float m_fHeight;
  float m_fRange;
  ezQuat m_qGlobalRotation;
};

/// Rectangular area light that emits light from a flat surface.
///
/// The rectangle's normal points along the component's local -X axis (forward direction),
/// with width along the local Y axis and height along local Z. Light is emitted from the front face only.
/// Shadows are approximated by treating the light as a point light at its center.
class EZ_RENDERERCORE_DLL ezRectLightComponent : public ezLightComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezRectLightComponent, ezLightComponent, ezRectLightComponentManager);

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
  // ezRectLightComponent

public:
  ezRectLightComponent();
  ~ezRectLightComponent();

  void SetWidth(float fWidth);   // [ property ]
  float GetWidth() const;        // [ property ]

  void SetHeight(float fHeight); // [ property ]
  float GetHeight() const;       // [ property ]

  /// \brief Sets the light range. If zero, the range is automatically determined from the intensity.
  void SetRange(float fRange);   // [ property ]
  float GetRange() const;        // [ property ]

  float GetEffectiveRange() const;

  void SetShadowFadeOutRange(float fRange); // [ property ]
  float GetShadowFadeOutRange() const;      // [ property ]

protected:
  void OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const;

  float m_fWidth = 1.0f;
  float m_fHeight = 1.0f;
  float m_fRange = 0.0f;
  float m_fEffectiveRange = 0.0f;
  float m_fShadowFadeOutRange = 0.0f;
};

/// Visualizer attribute for rectangular area lights.
///
/// Shows a wireframe rectangle with the light's dimensions and range in the editor.
class EZ_RENDERERCORE_DLL ezRectLightVisualizerAttribute : public ezVisualizerAttribute
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRectLightVisualizerAttribute, ezVisualizerAttribute);

public:
  ezRectLightVisualizerAttribute();
  ezRectLightVisualizerAttribute(
    const char* szWidthProperty, const char* szHeightProperty, const char* szRangeProperty, const char* szIntensityProperty, const char* szColorProperty);

  const ezUntrackedString& GetWidthProperty() const { return m_sProperty1; }
  const ezUntrackedString& GetHeightProperty() const { return m_sProperty2; }
  const ezUntrackedString& GetRangeProperty() const { return m_sProperty3; }
  const ezUntrackedString& GetIntensityProperty() const { return m_sProperty4; }
  const ezUntrackedString& GetColorProperty() const { return m_sProperty5; }
};
