#pragma once

#include <RendererCore/Components/RenderComponent.h>
#include <RendererCore/Material/MaterialResource.h>
#include <RendererCore/Meshes/HairStrandResource.h>
#include <RendererCore/Meshes/MeshResource.h>

using ezHairComponentManager = ezComponentManager<class ezHairComponent, ezBlockStorageType::Compact>;

/// Rendering mode for the hair component.
struct ezHairRenderMode
{
  using StorageType = ezUInt8;

  enum Enum : ezUInt8
  {
    Strands, ///< Strand-based rendering from an ezHairStrandResource. Uses compute tessellation.
    Cards,   ///< Hair card rendering from a standard mesh with HairMaterial.ezShader.

    Default = Strands
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_RENDERERCORE_DLL, ezHairRenderMode);

/// Renders hair using either strand-based compute tessellation or traditional hair cards.
///
/// In strand mode, this component loads an ezHairStrandResource (from an Alembic .abc file)
/// and dispatches a compute shader to tessellate the strand control points into camera-facing
/// triangle strips. The resulting geometry is shaded with HairStrandMaterial.ezShader using
/// an energy-conserving Marschner BSDF.
///
/// In card mode (backward compatible), it renders standard mesh geometry using
/// HairMaterial.ezShader, just like the previous version of this component.
///
/// Both modes share wind, gravity, and stiffness parameters that animate the strands.
class EZ_RENDERERCORE_DLL ezHairComponent : public ezRenderComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezHairComponent, ezRenderComponent, ezHairComponentManager);

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
  // ezHairComponent

public:
  ezHairComponent();
  ~ezHairComponent();

  // --- Mode ---

  void SetRenderMode(ezHairRenderMode::Enum mode);                    // [ property ]
  ezHairRenderMode::Enum GetRenderMode() const;                       // [ property ]

  // --- Strand mode ---

  void SetStrandResource(const ezHairStrandResourceHandle& hResource); // [ property ]
  const ezHairStrandResourceHandle& GetStrandResource() const;         // [ property ]

  void SetStrandMaterial(const ezMaterialResourceHandle& hMaterial);   // [ property ]
  const ezMaterialResourceHandle& GetStrandMaterial() const;           // [ property ]

  // --- Card mode ---

  void SetCardMesh(const ezMeshResourceHandle& hMesh);                // [ property ]
  const ezMeshResourceHandle& GetCardMesh() const;                    // [ property ]

  ezUInt32 CardMaterials_GetCount() const;                             // [ property ]
  ezString CardMaterials_GetValue(ezUInt32 uiIndex) const;             // [ property ]
  void CardMaterials_SetValue(ezUInt32 uiIndex, ezString sValue);     // [ property ]
  void CardMaterials_Insert(ezUInt32 uiIndex, ezString sValue);       // [ property ]
  void CardMaterials_Remove(ezUInt32 uiIndex);                        // [ property ]

  // --- Shared properties ---

  void SetColor(const ezColor& color);                                // [ property ]
  const ezColor& GetColor() const;                                    // [ property ]

  void SetSortingDepthOffset(float fOffset);                          // [ property ]
  float GetSortingDepthOffset() const;                                // [ property ]

  void SetWindInfluence(float fInfluence);                            // [ property ]
  float GetWindInfluence() const;                                     // [ property ]

  void SetGravityStrength(float fStrength);                           // [ property ]
  float GetGravityStrength() const;                                   // [ property ]

  void SetStiffness(float fStiffness);                                // [ property ]
  float GetStiffness() const;                                         // [ property ]

  // --- Strand-specific properties ---

  void SetWidthScale(float fScale);                                   // [ property ]
  float GetWidthScale() const;                                        // [ property ]

  void SetStrandDensity(float fDensity);                              // [ property ]
  float GetStrandDensity() const;                                     // [ property ]

protected:
  void OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const;

  void ExtractStrandRenderData(ezMsgExtractRenderData& msg) const;
  void ExtractCardRenderData(ezMsgExtractRenderData& msg) const;

  ezEnum<ezHairRenderMode> m_RenderMode;

  // Strand mode
  ezHairStrandResourceHandle m_hStrandResource;
  ezMaterialResourceHandle m_hStrandMaterial;

  // Card mode
  ezMeshResourceHandle m_hCardMesh;
  ezDynamicArray<ezMaterialResourceHandle> m_CardMaterials;

  // Shared
  ezColor m_Color = ezColor::White;
  float m_fSortingDepthOffset = 0.0f;
  float m_fWindInfluence = 0.5f;
  float m_fGravityStrength = 1.0f;
  float m_fStiffness = 0.5f;

  // Strand-specific
  float m_fWidthScale = 1.0f;
  float m_fStrandDensity = 1.0f;
};
