#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Foundation/Math/BoundingBoxSphere.h>
#include <RendererCore/RendererCoreDLL.h>
#include <RendererFoundation/RendererFoundationDLL.h>

using ezHairStrandResourceHandle = ezTypedResourceHandle<class ezHairStrandResource>;

/// A group of hair strands sharing the same point count per strand.
///
/// All strands within a group have exactly m_uiPointsPerStrand control points,
/// stored interleaved in the m_Points array: strand0[p0,p1,...], strand1[p0,p1,...], etc.
/// Widths are stored per-point in the same interleaved order.
struct EZ_RENDERERCORE_DLL ezHairStrandGroup
{
  ezUInt32 m_uiNumStrands = 0;
  ezUInt32 m_uiPointsPerStrand = 0;

  /// Control point positions, interleaved by strand. Total count = m_uiNumStrands * m_uiPointsPerStrand.
  ezDynamicArray<ezVec3> m_Points;

  /// Per-point width values. Same count as m_Points. Width tapers from root to tip.
  ezDynamicArray<float> m_Widths;

  /// Per-strand root UV coordinate for texture lookups (e.g. scalp position). Count = m_uiNumStrands.
  ezDynamicArray<ezVec2> m_RootUVs;

  /// Per-strand random value in [0,1] for procedural variation. Count = m_uiNumStrands.
  ezDynamicArray<float> m_Randoms;
};

/// Serializable descriptor for hair strand data.
///
/// Holds one or more strand groups (e.g. different scalp regions) and global bounding information.
/// The asset transform pipeline creates this from an Alembic file and serializes it to ezBinHairStrands.
struct EZ_RENDERERCORE_DLL ezHairStrandResourceDescriptor
{
  ezHairStrandResourceDescriptor();
  ~ezHairStrandResourceDescriptor();
  ezHairStrandResourceDescriptor(const ezHairStrandResourceDescriptor&);
  ezHairStrandResourceDescriptor(ezHairStrandResourceDescriptor&&);
  ezHairStrandResourceDescriptor& operator=(const ezHairStrandResourceDescriptor&);
  ezHairStrandResourceDescriptor& operator=(ezHairStrandResourceDescriptor&&);

  ezDynamicArray<ezHairStrandGroup> m_StrandGroups;
  ezBoundingBoxSphere m_Bounds;
  float m_fDefaultWidth = 0.001f;

  ezResult Serialize(ezStreamWriter& inout_stream) const;
  ezResult Deserialize(ezStreamReader& inout_stream);
  ezUInt64 GetHeapMemoryUsage() const;
  void Clear();
};

/// GPU-side data layout for a single strand control point.
struct ezHairStrandPointGPU
{
  ezVec3 m_vPosition;
  float m_fWidth;
};

/// GPU-side per-strand metadata.
struct ezHairStrandInfoGPU
{
  ezUInt32 m_uiPointOffset;
  ezUInt32 m_uiPointCount;
  float m_fRootU;
  float m_fRootV;
  float m_fRandom;
  float m_fPad0;
  float m_fPad1;
  float m_fPad2;
};

/// GPU-side tessellated vertex output from the compute shader.
struct ezHairTessVertexGPU
{
  ezVec3 m_vPosition;
  float m_fStrandParam; // 0=root, 1=tip
  ezVec3 m_vNormal;
  float m_fPad0;
  ezVec3 m_vTangent;
  float m_fPad1;
  ezVec2 m_vTexCoord;
  float m_fPad2;
  float m_fPad3;
};

/// Per-group GPU metadata for dispatching/drawing.
struct ezHairStrandGroupGPU
{
  ezUInt32 m_uiStrandOffset = 0;
  ezUInt32 m_uiStrandCount = 0;
  ezUInt32 m_uiPointOffset = 0;
  ezUInt32 m_uiPointCount = 0;
  ezUInt32 m_uiTessVertexOffset = 0;
  ezUInt32 m_uiTessVertexCount = 0;
};

EZ_DEFINE_AS_POD_TYPE(ezHairStrandPointGPU);
EZ_DEFINE_AS_POD_TYPE(ezHairStrandInfoGPU);
EZ_DEFINE_AS_POD_TYPE(ezHairTessVertexGPU);
EZ_DEFINE_AS_POD_TYPE(ezHairStrandGroupGPU);

/// Runtime resource that holds hair strand data on the GPU.
///
/// Created from an ezHairStrandResourceDescriptor (loaded from ezBinHairStrands files).
/// Uploads strand control points and per-strand info into StructuredBuffers, and
/// allocates a UAV buffer for the compute tessellation output.
class EZ_RENDERERCORE_DLL ezHairStrandResource : public ezResource
{
  EZ_ADD_DYNAMIC_REFLECTION(ezHairStrandResource, ezResource);
  EZ_RESOURCE_DECLARE_COMMON_CODE(ezHairStrandResource);
  EZ_RESOURCE_DECLARE_CREATEABLE(ezHairStrandResource, ezHairStrandResourceDescriptor);

public:
  ezHairStrandResource();
  ~ezHairStrandResource();

  ezGALBufferHandle GetPointBuffer() const { return m_hPointBuffer; }
  ezGALBufferHandle GetStrandInfoBuffer() const { return m_hStrandInfoBuffer; }
  ezGALBufferHandle GetTessVertexBuffer() const { return m_hTessVertexBuffer; }

  ezUInt32 GetTotalStrandCount() const { return m_uiTotalStrands; }
  ezUInt32 GetTotalPointCount() const { return m_uiTotalPoints; }
  ezUInt32 GetTotalTessVertexCount() const { return m_uiTotalTessVertices; }
  const ezBoundingBoxSphere& GetBounds() const { return m_Bounds; }

  ezArrayPtr<const ezHairStrandGroupGPU> GetStrandGroups() const { return m_StrandGroupsGPU; }

private:
  virtual ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual ezResourceLoadDesc UpdateContent(ezStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  void CreateGPUBuffers(const ezHairStrandResourceDescriptor& desc);
  void DestroyGPUBuffers();

  ezGALBufferHandle m_hPointBuffer;
  ezGALBufferHandle m_hStrandInfoBuffer;
  ezGALBufferHandle m_hTessVertexBuffer;

  ezUInt32 m_uiTotalStrands = 0;
  ezUInt32 m_uiTotalPoints = 0;
  ezUInt32 m_uiTotalTessVertices = 0;
  ezBoundingBoxSphere m_Bounds;

  ezDynamicArray<ezHairStrandGroupGPU> m_StrandGroupsGPU;
};
