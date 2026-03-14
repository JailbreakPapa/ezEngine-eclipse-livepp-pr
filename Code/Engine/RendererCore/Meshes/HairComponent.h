#pragma once

#include <RendererCore/Meshes/MeshComponentBase.h>

using ezHairComponentManager = ezComponentManager<class ezHairComponent, ezBlockStorageType::Compact>;

/// Renders hair using the hair cards approach with the HairMaterial shader.
///
/// Hair cards are standard mesh geometry (quads, strips, or shells) authored in a DCC tool
/// and shaded with HairMaterial.ezShader to approximate individual hair strands using the
/// Marschner BSDF. This component packs wind and gravity parameters into the per-instance
/// CustomData vector so the shader can animate hair strands.
///
/// The referenced mesh should contain hair card geometry and the material should use
/// HairMaterial.ezShader. Wind influence and gravity strength are passed to the shader
/// via CustomData.x and CustomData.y respectively.
class EZ_RENDERERCORE_DLL ezHairComponent : public ezMeshComponentBase
{
  EZ_DECLARE_COMPONENT_TYPE(ezHairComponent, ezMeshComponentBase, ezHairComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(ezWorldReader& inout_stream) override;

  //////////////////////////////////////////////////////////////////////////
  // ezHairComponent

public:
  ezHairComponent();
  ~ezHairComponent();

  void SetWindInfluence(float fInfluence);       // [ property ]
  float GetWindInfluence() const;                // [ property ]

  void SetGravityStrength(float fStrength);      // [ property ]
  float GetGravityStrength() const;              // [ property ]

protected:
  void OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const;

  float m_fWindInfluence = 0.5f;
  float m_fGravityStrength = 1.0f;
};
