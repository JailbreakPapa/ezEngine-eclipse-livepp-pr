#pragma once

#include <Core/World/SettingsComponent.h>
#include <Core/World/SettingsComponentManager.h>
#include <RendererCore/Pipeline/RenderData.h>

struct ezMsgUpdateLocalBounds;

using ezVolumetricFogComponentManager = ezSettingsComponentManager<class ezVolumetricFogComponent>;

/// Render data for volumetric fog.
class EZ_RENDERERCORE_DLL ezVolumetricFogRenderData : public ezRenderData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezVolumetricFogRenderData, ezRenderData);

public:
  float m_fDensity;
  float m_fAnisotropy;
  float m_fBaseHeight;
  float m_fHeightFalloff;
  float m_fStartDistance;
  float m_fNearPlane;
  float m_fFarPlane;
  ezColor m_Albedo;
  ezColor m_AmbientLight;
  float m_fTemporalBlendWeight;
};

/// A world component configuring volumetric fog.
///
/// Place this in a scene to enable froxel-based volumetric fog with per-light
/// scattering. Controls density, scattering anisotropy, height falloff, and
/// temporal reprojection weight. Only one instance per scene is needed (uses
/// ezSettingsComponent pattern like ezFogComponent).
class EZ_RENDERERCORE_DLL ezVolumetricFogComponent : public ezSettingsComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezVolumetricFogComponent, ezSettingsComponent, ezVolumetricFogComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(ezWorldReader& inout_stream) override;

protected:
  virtual void Deinitialize() override;
  virtual void OnActivated() override;
  virtual void OnDeactivated() override;

  //////////////////////////////////////////////////////////////////////////
  // ezVolumetricFogComponent

public:
  ezVolumetricFogComponent();
  ~ezVolumetricFogComponent();

  void SetDensity(float fDensity);             // [ property ]
  float GetDensity() const;                    // [ property ]

  void SetAnisotropy(float fAnisotropy);       // [ property ]
  float GetAnisotropy() const;                 // [ property ]

  void SetHeightFalloff(float fHeightFalloff); // [ property ]
  float GetHeightFalloff() const;              // [ property ]

  void SetAlbedo(ezColor color);               // [ property ]
  ezColor GetAlbedo() const;                   // [ property ]

  void SetAmbientLight(ezColor color);         // [ property ]
  ezColor GetAmbientLight() const;             // [ property ]

  void SetStartDistance(float fDistance);       // [ property ]
  float GetStartDistance() const;              // [ property ]

  void SetNearPlane(float fNear);              // [ property ]
  float GetNearPlane() const;                  // [ property ]

  void SetFarPlane(float fFar);                // [ property ]
  float GetFarPlane() const;                   // [ property ]

  void SetTemporalBlendWeight(float fWeight);  // [ property ]
  float GetTemporalBlendWeight() const;        // [ property ]

protected:
  void OnUpdateLocalBounds(ezMsgUpdateLocalBounds& msg);
  void OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const;

  float m_fDensity = 0.5f;
  float m_fAnisotropy = 0.3f;
  float m_fHeightFalloff = 0.1f;
  ezColor m_Albedo = ezColor::White;
  ezColor m_AmbientLight = ezColor(0.05f, 0.05f, 0.08f);
  float m_fStartDistance = 0.0f;
  float m_fNearPlane = 0.5f;
  float m_fFarPlane = 500.0f;
  float m_fTemporalBlendWeight = 0.05f;
};
