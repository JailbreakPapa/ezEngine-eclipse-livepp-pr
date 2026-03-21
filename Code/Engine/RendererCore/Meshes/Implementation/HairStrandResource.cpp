#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Utilities/AssetFileHeader.h>
#include <RendererCore/Meshes/HairStrandResource.h>
#include <RendererFoundation/Device/Device.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezHairStrandResource, 1, ezRTTIDefaultAllocator<ezHairStrandResource>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_RESOURCE_IMPLEMENT_COMMON_CODE(ezHairStrandResource);
// clang-format on

// --- Descriptor ---

// Explicit special member definitions required for dllexport with non-trivial members (ezDynamicArray).
ezHairStrandResourceDescriptor::ezHairStrandResourceDescriptor() = default;
ezHairStrandResourceDescriptor::~ezHairStrandResourceDescriptor() = default;
ezHairStrandResourceDescriptor::ezHairStrandResourceDescriptor(const ezHairStrandResourceDescriptor&) = default;
ezHairStrandResourceDescriptor::ezHairStrandResourceDescriptor(ezHairStrandResourceDescriptor&&) = default;
ezHairStrandResourceDescriptor& ezHairStrandResourceDescriptor::operator=(const ezHairStrandResourceDescriptor&) = default;
ezHairStrandResourceDescriptor& ezHairStrandResourceDescriptor::operator=(ezHairStrandResourceDescriptor&&) = default;

static constexpr ezUInt32 s_uiHairStrandDescVersion = 1;

ezResult ezHairStrandResourceDescriptor::Serialize(ezStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(s_uiHairStrandDescVersion);
  inout_stream << m_Bounds.IsValid();

  if (m_Bounds.IsValid())
  {
    inout_stream << m_Bounds.m_vCenter;
    inout_stream << m_Bounds.m_vBoxHalfExtents;
    inout_stream << m_Bounds.m_fSphereRadius;
  }

  inout_stream << m_fDefaultWidth;
  inout_stream << m_StrandGroups.GetCount();

  for (const auto& group : m_StrandGroups)
  {
    inout_stream << group.m_uiNumStrands;
    inout_stream << group.m_uiPointsPerStrand;

    EZ_ASSERT_DEV(group.m_Points.GetCount() == group.m_uiNumStrands * group.m_uiPointsPerStrand, "Point count mismatch");
    EZ_ASSERT_DEV(group.m_Widths.GetCount() == group.m_Points.GetCount(), "Width count mismatch");
    EZ_ASSERT_DEV(group.m_RootUVs.GetCount() == group.m_uiNumStrands, "RootUV count mismatch");
    EZ_ASSERT_DEV(group.m_Randoms.GetCount() == group.m_uiNumStrands, "Random count mismatch");

    inout_stream.WriteBytes(group.m_Points.GetData(), group.m_Points.GetCount() * sizeof(ezVec3)).IgnoreResult();
    inout_stream.WriteBytes(group.m_Widths.GetData(), group.m_Widths.GetCount() * sizeof(float)).IgnoreResult();
    inout_stream.WriteBytes(group.m_RootUVs.GetData(), group.m_RootUVs.GetCount() * sizeof(ezVec2)).IgnoreResult();
    inout_stream.WriteBytes(group.m_Randoms.GetData(), group.m_Randoms.GetCount() * sizeof(float)).IgnoreResult();
  }

  return EZ_SUCCESS;
}

ezResult ezHairStrandResourceDescriptor::Deserialize(ezStreamReader& inout_stream)
{
  const auto version = inout_stream.ReadVersion(s_uiHairStrandDescVersion);
  EZ_IGNORE_UNUSED(version);

  bool bBoundsValid = false;
  inout_stream >> bBoundsValid;

  if (bBoundsValid)
  {
    ezVec3 center, halfExtents;
    float radius;
    inout_stream >> center;
    inout_stream >> halfExtents;
    inout_stream >> radius;
    m_Bounds = ezBoundingBoxSphere::MakeFromCenterExtents(center, halfExtents, radius);
  }
  else
  {
    m_Bounds = ezBoundingBoxSphere::MakeInvalid();
  }

  inout_stream >> m_fDefaultWidth;

  ezUInt32 uiGroupCount = 0;
  inout_stream >> uiGroupCount;
  m_StrandGroups.SetCount(uiGroupCount);

  for (auto& group : m_StrandGroups)
  {
    inout_stream >> group.m_uiNumStrands;
    inout_stream >> group.m_uiPointsPerStrand;

    const ezUInt32 uiTotalPoints = group.m_uiNumStrands * group.m_uiPointsPerStrand;

    group.m_Points.SetCountUninitialized(uiTotalPoints);
    group.m_Widths.SetCountUninitialized(uiTotalPoints);
    group.m_RootUVs.SetCountUninitialized(group.m_uiNumStrands);
    group.m_Randoms.SetCountUninitialized(group.m_uiNumStrands);

    inout_stream.ReadBytes(group.m_Points.GetData(), uiTotalPoints * sizeof(ezVec3));
    inout_stream.ReadBytes(group.m_Widths.GetData(), uiTotalPoints * sizeof(float));
    inout_stream.ReadBytes(group.m_RootUVs.GetData(), group.m_uiNumStrands * sizeof(ezVec2));
    inout_stream.ReadBytes(group.m_Randoms.GetData(), group.m_uiNumStrands * sizeof(float));
  }

  return EZ_SUCCESS;
}

ezUInt64 ezHairStrandResourceDescriptor::GetHeapMemoryUsage() const
{
  ezUInt64 uiTotal = 0;
  for (const auto& group : m_StrandGroups)
  {
    uiTotal += group.m_Points.GetHeapMemoryUsage();
    uiTotal += group.m_Widths.GetHeapMemoryUsage();
    uiTotal += group.m_RootUVs.GetHeapMemoryUsage();
    uiTotal += group.m_Randoms.GetHeapMemoryUsage();
  }
  uiTotal += m_StrandGroups.GetHeapMemoryUsage();
  return uiTotal;
}

void ezHairStrandResourceDescriptor::Clear()
{
  m_StrandGroups.Clear();
  m_Bounds = ezBoundingBoxSphere::MakeInvalid();
  m_fDefaultWidth = 0.001f;
}

// --- Resource ---

ezHairStrandResource::ezHairStrandResource()
  : ezResource(DoUpdate::OnGraphicsResourceThreads, 1)
{
}

ezHairStrandResource::~ezHairStrandResource()
{
  DestroyGPUBuffers();
}

EZ_RESOURCE_IMPLEMENT_CREATEABLE(ezHairStrandResource, ezHairStrandResourceDescriptor)
{
  CreateGPUBuffers(descriptor);

  ezResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = ezResourceState::Loaded;
  return res;
}

ezResourceLoadDesc ezHairStrandResource::UnloadData(Unload WhatToUnload)
{
  DestroyGPUBuffers();

  m_uiTotalStrands = 0;
  m_uiTotalPoints = 0;
  m_uiTotalTessVertices = 0;
  m_Bounds = ezBoundingBoxSphere::MakeInvalid();
  m_StrandGroupsGPU.Clear();

  ezResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = ezResourceState::Unloaded;
  return res;
}

ezResourceLoadDesc ezHairStrandResource::UpdateContent(ezStreamReader* Stream)
{
  EZ_LOG_BLOCK("ezHairStrandResource::UpdateContent", GetResourceIdOrDescription());

  ezResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  if (Stream == nullptr)
  {
    res.m_State = ezResourceState::LoadedResourceMissing;
    return res;
  }

  ezStringBuilder sAbsFilePath;
  (*Stream) >> sAbsFilePath;

  ezAssetFileHeader AssetHash;
  AssetHash.Read(*Stream).IgnoreResult();

  ezHairStrandResourceDescriptor desc;
  if (desc.Deserialize(*Stream).Failed())
  {
    res.m_State = ezResourceState::LoadedResourceMissing;
    return res;
  }

  CreateGPUBuffers(desc);

  res.m_State = ezResourceState::Loaded;
  return res;
}

void ezHairStrandResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(ezHairStrandResource) + m_StrandGroupsGPU.GetHeapMemoryUsage();
  out_NewMemoryUsage.m_uiMemoryGPU =
    m_uiTotalPoints * sizeof(ezHairStrandPointGPU) +
    m_uiTotalStrands * sizeof(ezHairStrandInfoGPU) +
    m_uiTotalTessVertices * sizeof(ezHairTessVertexGPU);
}

void ezHairStrandResource::CreateGPUBuffers(const ezHairStrandResourceDescriptor& desc)
{
  DestroyGPUBuffers();

  m_Bounds = desc.m_Bounds;

  // Count totals across all groups
  m_uiTotalStrands = 0;
  m_uiTotalPoints = 0;
  m_uiTotalTessVertices = 0;

  for (const auto& group : desc.m_StrandGroups)
  {
    m_uiTotalStrands += group.m_uiNumStrands;
    const ezUInt32 uiGroupPoints = group.m_uiNumStrands * group.m_uiPointsPerStrand;
    m_uiTotalPoints += uiGroupPoints;
    // Each strand has (pointsPerStrand) control points → (pointsPerStrand) * 2 tess vertices
    m_uiTotalTessVertices += group.m_uiNumStrands * group.m_uiPointsPerStrand * 2;
  }

  if (m_uiTotalStrands == 0 || m_uiTotalPoints == 0)
    return;

  // Build interleaved GPU data arrays
  ezDynamicArray<ezHairStrandPointGPU> pointData;
  pointData.Reserve(m_uiTotalPoints);

  ezDynamicArray<ezHairStrandInfoGPU> strandData;
  strandData.Reserve(m_uiTotalStrands);

  m_StrandGroupsGPU.Clear();
  m_StrandGroupsGPU.Reserve(desc.m_StrandGroups.GetCount());

  ezUInt32 uiGlobalPointOffset = 0;
  ezUInt32 uiGlobalStrandOffset = 0;
  ezUInt32 uiGlobalTessOffset = 0;

  for (const auto& group : desc.m_StrandGroups)
  {
    auto& gpuGroup = m_StrandGroupsGPU.ExpandAndGetRef();
    gpuGroup.m_uiStrandOffset = uiGlobalStrandOffset;
    gpuGroup.m_uiStrandCount = group.m_uiNumStrands;
    gpuGroup.m_uiPointOffset = uiGlobalPointOffset;
    gpuGroup.m_uiPointCount = group.m_uiNumStrands * group.m_uiPointsPerStrand;
    gpuGroup.m_uiTessVertexOffset = uiGlobalTessOffset;
    gpuGroup.m_uiTessVertexCount = group.m_uiNumStrands * group.m_uiPointsPerStrand * 2;

    // Pack per-point data
    for (ezUInt32 i = 0; i < gpuGroup.m_uiPointCount; ++i)
    {
      auto& pt = pointData.ExpandAndGetRef();
      pt.m_vPosition = group.m_Points[i];
      pt.m_fWidth = group.m_Widths[i];
    }

    // Pack per-strand info
    for (ezUInt32 s = 0; s < group.m_uiNumStrands; ++s)
    {
      auto& info = strandData.ExpandAndGetRef();
      info.m_uiPointOffset = uiGlobalPointOffset + s * group.m_uiPointsPerStrand;
      info.m_uiPointCount = group.m_uiPointsPerStrand;
      info.m_fRootU = group.m_RootUVs[s].x;
      info.m_fRootV = group.m_RootUVs[s].y;
      info.m_fRandom = group.m_Randoms[s];
      info.m_fPad0 = 0;
      info.m_fPad1 = 0;
      info.m_fPad2 = 0;
    }

    uiGlobalPointOffset += gpuGroup.m_uiPointCount;
    uiGlobalStrandOffset += group.m_uiNumStrands;
    uiGlobalTessOffset += gpuGroup.m_uiTessVertexCount;
  }

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  // Points buffer (SRV)
  {
    ezGALBufferCreationDescription bufferDesc;
    bufferDesc.m_uiStructSize = sizeof(ezHairStrandPointGPU);
    bufferDesc.m_uiTotalSize = pointData.GetCount() * sizeof(ezHairStrandPointGPU);
    bufferDesc.m_BufferFlags = ezGALBufferUsageFlags::StructuredBuffer | ezGALBufferUsageFlags::ShaderResource;
    bufferDesc.m_ResourceAccess.m_bImmutable = true;

    m_hPointBuffer = pDevice->CreateBuffer(bufferDesc, pointData.GetByteArrayPtr());
    if (const ezGALBuffer* pBuffer = pDevice->GetBuffer(m_hPointBuffer))
      pBuffer->SetDebugName("HairStrandPoints");
  }

  // Strand info buffer (SRV)
  {
    ezGALBufferCreationDescription bufferDesc;
    bufferDesc.m_uiStructSize = sizeof(ezHairStrandInfoGPU);
    bufferDesc.m_uiTotalSize = strandData.GetCount() * sizeof(ezHairStrandInfoGPU);
    bufferDesc.m_BufferFlags = ezGALBufferUsageFlags::StructuredBuffer | ezGALBufferUsageFlags::ShaderResource;
    bufferDesc.m_ResourceAccess.m_bImmutable = true;

    m_hStrandInfoBuffer = pDevice->CreateBuffer(bufferDesc, strandData.GetByteArrayPtr());
    if (const ezGALBuffer* pBuffer = pDevice->GetBuffer(m_hStrandInfoBuffer))
      pBuffer->SetDebugName("HairStrandInfos");
  }

  // Tessellated vertex buffer (UAV for compute, SRV for rendering)
  {
    ezGALBufferCreationDescription bufferDesc;
    bufferDesc.m_uiStructSize = sizeof(ezHairTessVertexGPU);
    bufferDesc.m_uiTotalSize = m_uiTotalTessVertices * sizeof(ezHairTessVertexGPU);
    bufferDesc.m_BufferFlags = ezGALBufferUsageFlags::StructuredBuffer | ezGALBufferUsageFlags::ShaderResource | ezGALBufferUsageFlags::UnorderedAccess;
    bufferDesc.m_ResourceAccess.m_bImmutable = false;

    m_hTessVertexBuffer = pDevice->CreateBuffer(bufferDesc);
    if (const ezGALBuffer* pBuffer = pDevice->GetBuffer(m_hTessVertexBuffer))
      pBuffer->SetDebugName("HairTessVertices");
  }
}

void ezHairStrandResource::DestroyGPUBuffers()
{
  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  if (!m_hPointBuffer.IsInvalidated())
  {
    pDevice->DestroyBuffer(m_hPointBuffer);
    m_hPointBuffer.Invalidate();
  }

  if (!m_hStrandInfoBuffer.IsInvalidated())
  {
    pDevice->DestroyBuffer(m_hStrandInfoBuffer);
    m_hStrandInfoBuffer.Invalidate();
  }

  if (!m_hTessVertexBuffer.IsInvalidated())
  {
    pDevice->DestroyBuffer(m_hTessVertexBuffer);
    m_hTessVertexBuffer.Invalidate();
  }
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_HairStrandResource);
