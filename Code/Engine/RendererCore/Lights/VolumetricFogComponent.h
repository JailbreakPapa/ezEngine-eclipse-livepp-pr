#pragma once

#include <Core/World/World.h>
#include <RendererCore/Components/RenderComponent.h>
#include <RendererCore/Pipeline/RenderData.h>

struct ezMsgUpdateLocalBounds;

/// Render data for a volumetric fog volume.
class EZ_RENDERERCORE_DLL ezVolumetricFogRenderData : public ezRenderData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezVolumetricFogRenderData, ezRenderData);

public:
  float m_fDensity;
  float m_fAnisotropy;
  float m_fHeightFalloff;
  float m_fStartDistance;
  float m_fNearPlane;
  float m_fFarPlane;
  ezColor m_Albedo;
  ezColor m_AmbientLight;
  float m_fTemporalBlendWeight;
  float m_fFalloffExponent;

  /// The world-space transform of the fog volume (position + rotation + scale).
  ezTransform m_GlobalTransform;

  /// Half-extents of the fog volume in local space (the full box is 2x this).
  ezVec3 m_vHalfExtents;
};

using ezVolumetricFogComponentManager = ezComponentManager<class ezVolumetricFogComponent, ezBlockStorageType::Compact>;

/// A volumetric fog volume defined by an oriented bounding box.
///
/// Place one or more of these in a scene to create localized fog regions.
/// The fog is rendered using a froxel-based approach with per-light scattering.
/// Each volume contributes density within its bounding box, with smooth falloff
/// at the edges. Height-based density falloff is in world space relative to the
/// game object's Z position.
class EZ_RENDERERCORE_DLL ezVolumetricFogComponent : public ezRenderComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezVolumetricFogComponent, ezRenderComponent, ezVolumetricFogComponentManager);

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
  // ezRenderComponent

public:
  virtual ezResult GetLocalBounds(ezBoundingBoxSphere& out_bounds, bool& out_bAlwaysVisible, ezMsgUpdateLocalBounds& ref_msg) override;

  //////////////////////////////////////////////////////////////////////////
  // ezVolumetricFogComponent

public:
  ezVolumetricFogComponent();
  ~ezVolumetricFogComponent();

  void SetExtents(const ezVec3& vExtents);       // [ property ]
  const ezVec3& GetExtents() const;              // [ property ]

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

  void SetFalloffExponent(float fExponent);    // [ property ]
  float GetFalloffExponent() const;            // [ property ]

protected:
  void OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const;

  ezVec3 m_vExtents = ezVec3(10.0f);
  float m_fDensity = 0.5f;
  float m_fAnisotropy = 0.3f;
  float m_fHeightFalloff = 0.0f;
  ezColor m_Albedo = ezColor::White;
  ezColor m_AmbientLight = ezColor(0.15f, 0.15f, 0.2f);
  float m_fStartDistance = 0.0f;
  float m_fNearPlane = 0.5f;
  float m_fFarPlane = 500.0f;
  float m_fTemporalBlendWeight = 0.05f;
  float m_fFalloffExponent = 1.0f;
};
