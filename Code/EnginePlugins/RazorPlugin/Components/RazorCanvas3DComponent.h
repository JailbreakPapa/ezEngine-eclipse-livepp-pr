#pragma once

#include <RendererCore/Material/MaterialResource.h>
#include <RendererCore/Meshes/MeshResource.h>
#include <RazorPlugin/Components/RazorCanvasComponentBase.h>

#include <RazorCore/Render/RazorDrawCommand.h>
#include <RazorCore/Render/RazorPaintGenerator.h>

using ezCpuMeshResourceHandle = ezTypedResourceHandle<class ezCpuMeshResource>;

using ezRazorCanvas3DComponentManager = ezComponentManagerSimple<class ezRazorCanvas3DComponent, ezComponentUpdateType::Always, ezBlockStorageType::Compact, ezWorldUpdatePhase::PostTransform>;

/// Renders a Razor UI document onto a 3D surface in world space.
///
/// The component renders the UI to an offscreen texture, then applies that texture
/// to a material slot on the owning game object's mesh. This allows UI to appear on
/// in-game screens, panels, or any other 3D surface.
///
/// For input, use RaycastInput() with a world-space ray to determine the texture-space
/// hit position against the proxy mesh (or the mesh on the owning object).
class EZ_RAZORPLUGIN_DLL ezRazorCanvas3DComponent : public ezRazorCanvasComponentBase
{
  EZ_DECLARE_COMPONENT_TYPE(ezRazorCanvas3DComponent, ezRazorCanvasComponentBase, ezRazorCanvas3DComponentManager);

public:
  ezRazorCanvas3DComponent();
  ~ezRazorCanvas3DComponent();

  ezRazorCanvas3DComponent& operator=(ezRazorCanvas3DComponent&& rhs);

  virtual void OnActivated() override;
  virtual void OnDeactivated() override;

  void Update() final override;

  void SetProxyMesh(const ezMeshResourceHandle& hMesh) { m_hProxyMesh = hMesh; } // [ property ]
  const ezMeshResourceHandle& GetProxyMesh() const { return m_hProxyMesh; }       // [ property ]

  EZ_ADD_RESOURCEHANDLE_ACCESSORS_WITH_SETTER(ProxyMesh, m_hProxyMesh, SetProxyMesh);

  void SetBaseMaterial(const ezMaterialResourceHandle& hMaterial);                    // [ property ]
  const ezMaterialResourceHandle& GetBaseMaterial() const { return m_hBaseMaterial; } // [ property ]

  EZ_ADD_RESOURCEHANDLE_ACCESSORS_WITH_SETTER(BaseMaterial, m_hBaseMaterial, SetBaseMaterial);

  void SetMaterialIndex(ezUInt32 uiMaterialIndex);                        // [ property ]
  ezUInt32 GetMaterialIndex() const { return m_uiMaterialIndex; }         // [ property ]

  void SetTextureSlotName(ezStringView sName);                            // [ property ]
  ezStringView GetTextureSlotName() const { return m_sTextureSlotName; }  // [ property ]

  void SetTextureSize(const ezVec2U32& vSize);                            // [ property ]
  const ezVec2U32& GetTextureSize() const { return m_vSize; }             // [ property ]

  void SetDpiScale(float fDpiScale);                                      // [ property ]
  float GetDpiScale() const { return m_fDpiScale; }                       // [ property ]

  void SetInteractive(bool bIsInteractive);                               // [ property ]
  bool IsInteractive() const { return m_bIsInteractive; }                 // [ property ]

  /// Performs a raycast against the proxy mesh (or owning mesh) and, on hit,
  /// converts the hit texture coordinate to a canvas-space position.
  bool RaycastInput(const ezVec3& vRayOrigin, const ezVec3& vRayDir);

  virtual void SerializeComponent(ezWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(ezWorldReader& inout_stream) override;

  virtual ezResult GetLocalBounds(ezBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, ezMsgUpdateLocalBounds& ref_msg) override;

protected:
  void OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const override;

  bool UpdateTextureAndMaterial();

  static bool RaycastMeshTexCoords(const class ezCpuMeshResource* pMesh, ezUInt32 uiSubMeshIndex,
                                   const ezVec3& vRayOrigin, const ezVec3& vRayDir,
                                   ezVec2& out_vTexCoords, float fEpsilon = 0.00001f);

  // properties
  ezMeshResourceHandle m_hProxyMesh;
  ezMaterialResourceHandle m_hBaseMaterial;
  ezUInt32 m_uiMaterialIndex = 0;
  ezHashedString m_sTextureSlotName;
  float m_fDpiScale = 1.0f;
  bool m_bIsInteractive = true;

  // runtime data
  ezCpuMeshResourceHandle m_hCachedCpuMesh;
  ezMaterialResourceHandle m_hMaterial;
  ezTexture2DResourceHandle m_hTexture;

  ezRazorPaintGenerator m_PaintGenerator;
  mutable ezRazorRenderBatch m_RenderBatch;
  mutable bool m_bPaintDirty = true;
  mutable ezUInt64 m_uiLastLayoutVersion = 0;  ///< Last document layout version we rendered.
  mutable ezUInt64 m_uiLastExtractedFrame = 0; ///< Frame when last extracted, to avoid duplicate GPU work.
};
