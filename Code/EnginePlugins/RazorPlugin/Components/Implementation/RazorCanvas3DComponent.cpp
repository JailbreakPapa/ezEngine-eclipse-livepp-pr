#include <RazorPlugin/RazorPluginPCH.h>

#include <RazorPlugin/Components/RazorCanvas3DComponent.h>
#include <RazorPlugin/Rendering/RazorGeometryBuilder.h>
#include <RazorPlugin/Rendering/RazorRenderData.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/Math/Intersection.h>
#include <RazorCore/DOM/RazorDocument.h>
#include <RendererCore/Material/MaterialResource.h>
#include <RendererCore/Meshes/CpuMeshResource.h>
#include <RendererCore/Meshes/MeshComponent.h>
#include <RendererCore/Pipeline/RenderData.h>
#include <RendererCore/Pipeline/RenderDataManager.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/Textures/Texture2DResource.h>
#include <RendererFoundation/Device/Device.h>

static ezAtomicInteger32 s_RazorResourceCounter;

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezRazorCanvas3DComponent, 1, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_RESOURCE_ACCESSOR_PROPERTY("ProxyMesh", GetProxyMesh, SetProxyMesh)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Mesh_Static")),
    EZ_RESOURCE_ACCESSOR_PROPERTY("BaseMaterial", GetBaseMaterial, SetBaseMaterial)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Material"), new ezDefaultValueAttribute("{ 05af8d07-0b38-44a6-8d50-49731ae2625d }")),
    EZ_ACCESSOR_PROPERTY("MaterialIndex", GetMaterialIndex, SetMaterialIndex)->AddAttributes(new ezDefaultValueAttribute(0)),
    EZ_ACCESSOR_PROPERTY("TextureSlotName", GetTextureSlotName, SetTextureSlotName)->AddAttributes(new ezDefaultValueAttribute("BaseTexture")),
    EZ_ACCESSOR_PROPERTY("TextureSize", GetTextureSize, SetTextureSize)->AddAttributes(new ezSuffixAttribute("px"), new ezDefaultValueAttribute(ezVec2U32(512, 512)), new ezClampValueAttribute(ezVec2U32(0), ezVec2U32(4096))),
    EZ_ACCESSOR_PROPERTY("DpiScale", GetDpiScale, SetDpiScale)->AddAttributes(new ezDefaultValueAttribute(1.0f)),
    EZ_ACCESSOR_PROPERTY("IsInteractive", IsInteractive, SetInteractive)->AddAttributes(new ezDefaultValueAttribute(true)),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Input/Razor"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_COMPONENT_TYPE
// clang-format on

ezRazorCanvas3DComponent::ezRazorCanvas3DComponent()
{
  m_vSize = ezVec2U32(512, 512);
}

ezRazorCanvas3DComponent::~ezRazorCanvas3DComponent() = default;

ezRazorCanvas3DComponent& ezRazorCanvas3DComponent::operator=(ezRazorCanvas3DComponent&& rhs)
{
  SUPER::operator=(std::move(rhs));
  m_hProxyMesh = std::move(rhs.m_hProxyMesh);
  m_hBaseMaterial = std::move(rhs.m_hBaseMaterial);
  m_uiMaterialIndex = rhs.m_uiMaterialIndex;
  m_sTextureSlotName = std::move(rhs.m_sTextureSlotName);
  m_fDpiScale = rhs.m_fDpiScale;
  m_bIsInteractive = rhs.m_bIsInteractive;
  m_hCachedCpuMesh = std::move(rhs.m_hCachedCpuMesh);
  m_hMaterial = std::move(rhs.m_hMaterial);
  m_hTexture = std::move(rhs.m_hTexture);
  m_bPaintDirty = true;
  return *this;
}

void ezRazorCanvas3DComponent::OnActivated()
{
  SUPER::OnActivated();

  if (m_sTextureSlotName.IsEmpty())
  {
    m_sTextureSlotName.Assign("BaseTexture");
  }

  if (!m_hBaseMaterial.IsValid())
  {
    m_hBaseMaterial = ezResourceManager::LoadResource<ezMaterialResource>("{ 05af8d07-0b38-44a6-8d50-49731ae2625d }");
  }

  Update();

  if (m_hMaterial.IsValid())
  {
    ezMsgSetMeshMaterial msg;
    msg.m_hMaterial = m_hMaterial;
    msg.m_uiMaterialSlot = m_uiMaterialIndex;
    GetOwner()->PostMessageRecursive(msg, ezTime::MakeZero());
  }
}

void ezRazorCanvas3DComponent::OnDeactivated()
{
  SUPER::OnDeactivated();

  if (m_hMaterial.IsValid())
  {
    ezMsgSetMeshMaterial msg;
    msg.m_uiMaterialSlot = m_uiMaterialIndex;
    GetOwner()->PostMessageRecursive(msg, ezTime::MakeZero());
  }

  m_hTexture.Invalidate();
  m_hMaterial.Invalidate();
  m_hCachedCpuMesh.Invalidate();
}

void ezRazorCanvas3DComponent::Update()
{
  UpdateTextureAndMaterial();

  SUPER::Update();

  // Only mark paint dirty if the document layout actually changed.
  if (m_pDocument != nullptr && m_pDocument->GetLayoutVersion() != m_uiLastLayoutVersion)
  {
    m_bPaintDirty = true;
  }
}

void ezRazorCanvas3DComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  auto& s = inout_stream.GetStream();

  s << m_hBaseMaterial;
  s << m_uiMaterialIndex;
  s << m_sTextureSlotName;
  s << m_vSize;
  s << m_bIsInteractive;
  s << m_fDpiScale;
  s << m_hProxyMesh;
}

void ezRazorCanvas3DComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  auto& s = inout_stream.GetStream();

  s >> m_hBaseMaterial;
  s >> m_uiMaterialIndex;
  s >> m_sTextureSlotName;
  s >> m_vSize;
  s >> m_bIsInteractive;
  s >> m_fDpiScale;
  s >> m_hProxyMesh;
}

ezResult ezRazorCanvas3DComponent::GetLocalBounds(ezBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, ezMsgUpdateLocalBounds& ref_msg)
{
  // Not always visible — rely on the mesh component's bounds for culling.
  ref_bAlwaysVisible = false;
  return EZ_FAILURE;
}

void ezRazorCanvas3DComponent::OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const
{
  if (msg.m_pView->GetCameraUsageHint() != ezCameraUsageHint::MainView &&
      msg.m_pView->GetCameraUsageHint() != ezCameraUsageHint::EditorView &&
      msg.m_pView->GetCameraUsageHint() != ezCameraUsageHint::Thumbnail)
    return;

  if (msg.m_OverrideCategory != ezInvalidRenderDataCategory)
    return;

  if (m_pDocument == nullptr || !m_hTexture.IsValid())
    return;

  // Guard against duplicate offscreen rendering within the same frame.
  const ezUInt64 uiCurrentFrame = ezGALDevice::GetDefaultDevice()->GetCurrentFrame();
  const bool bAlreadyExtractedThisFrame = (m_uiLastExtractedFrame == uiCurrentFrame);

  // Rebuild paint commands if dirty.
  if (m_bPaintDirty)
  {
    const_cast<ezRazorPaintGenerator&>(m_PaintGenerator).Generate(*m_pDocument, m_RenderBatch);
    m_uiLastLayoutVersion = m_pDocument->GetLayoutVersion();
    m_bPaintDirty = false;
  }
  else if (bAlreadyExtractedThisFrame)
  {
    // Content hasn't changed and we've already submitted render data this frame.
    return;
  }

  m_uiLastExtractedFrame = uiCurrentFrame;

  if (m_RenderBatch.GetCommands().IsEmpty())
    return;

  // Convert paint commands to GPU vertex/index data.
  auto pRenderData = msg.m_pRenderDataManager->CreateRenderDataForThisFrame<ezRazorRenderData>(GetOwner());

  ezRazorGeometryBuilder builder;
  builder.Build(m_RenderBatch, *pRenderData);

  // Set the target texture so the renderer knows we want offscreen rendering.
  {
    ezResourceLock<ezTexture2DResource> pTexture(m_hTexture, ezResourceAcquireMode::AllowLoadingFallback);
    pRenderData->m_hTargetTexture = pTexture->GetGALTexture();
  }

  if (!pRenderData->m_Vertices.IsEmpty())
  {
    msg.AddRenderData(pRenderData, ezDefaultRenderDataCategories::GUI, ezRenderData::Caching::Never);
  }
}

bool ezRazorCanvas3DComponent::RaycastInput(const ezVec3& vRayOrigin, const ezVec3& vRayDir)
{
  if (m_pDocument == nullptr || !IsInteractive())
    return false;

  if (!m_hCachedCpuMesh.IsValid())
  {
    ezMeshResourceHandle hMesh = m_hProxyMesh;

    if (!m_hProxyMesh.IsValid())
    {
      ezMeshComponent* pMeshComponent = nullptr;
      if (GetOwner()->TryGetComponentOfBaseType(pMeshComponent))
      {
        hMesh = pMeshComponent->GetMesh();
      }
    }

    if (!hMesh.IsValid())
    {
      ezLog::Error("ezRazorCanvas3DComponent '{}' has no mesh to raycast against.", GetOwner()->GetName());
      SetInteractive(false);
      return false;
    }

    m_hCachedCpuMesh = ezResourceManager::LoadResource<ezCpuMeshResource>(hMesh.GetResourceID());
  }

  ezResourceLock<ezCpuMeshResource> pMesh(m_hCachedCpuMesh, ezResourceAcquireMode::AllowLoadingFallback);
  if (pMesh.GetAcquireResult() == ezResourceAcquireResult::LoadingFallback)
    return false;

  if (pMesh.GetAcquireResult() != ezResourceAcquireResult::Final)
  {
    ezLog::Error("ezRazorCanvas3DComponent '{}' failed to get CPU mesh for raycast.", GetOwner()->GetName());
    SetInteractive(false);
    return false;
  }

  const ezTransform tOwner = GetOwner()->GetGlobalTransform();

  if (ezMath::IsZero(tOwner.GetMaxScale(), 0.001f))
    return false;

  const ezTransform worldToLocal = tOwner.GetInverse();
  const ezVec3 vLocalOrigin = worldToLocal.TransformPosition(vRayOrigin);
  const ezVec3 vLocalDir = worldToLocal.TransformDirection(vRayDir).GetNormalized();

  const ezMeshResourceDescriptor& desc = pMesh->GetDescriptor();
  for (ezUInt32 uiSubMeshIndex = 0; uiSubMeshIndex < desc.GetSubMeshes().GetCount(); ++uiSubMeshIndex)
  {
    const ezMeshResourceDescriptor::SubMesh& submesh = desc.GetSubMeshes()[uiSubMeshIndex];
    if (submesh.m_uiMaterialIndex != m_uiMaterialIndex)
      continue;

    ezVec2 vTexCoords;
    if (!RaycastMeshTexCoords(pMesh.GetPointer(), uiSubMeshIndex, vLocalOrigin, vLocalDir, vTexCoords))
      continue;

    // TODO: Route hit position into the Razor input system.
    // Convert texture coords to document-space coords:
    // float fDocX = m_vSize.x * vTexCoords.x;
    // float fDocY = m_vSize.y * vTexCoords.y;
    return true;
  }

  return false;
}

// --- Property setters ---

void ezRazorCanvas3DComponent::SetBaseMaterial(const ezMaterialResourceHandle& hMaterial)
{
  if (hMaterial == m_hBaseMaterial)
    return;

  m_hBaseMaterial = hMaterial;
  m_hMaterial.Invalidate();
}

void ezRazorCanvas3DComponent::SetMaterialIndex(ezUInt32 uiMaterialIndex)
{
  if (uiMaterialIndex == m_uiMaterialIndex)
    return;

  const ezUInt32 uiPrevIndex = m_uiMaterialIndex;
  m_uiMaterialIndex = uiMaterialIndex;

  if (m_hMaterial.IsValid() && IsActiveAndInitialized())
  {
    ezMsgSetMeshMaterial msg;

    msg.m_uiMaterialSlot = uiPrevIndex;
    GetOwner()->PostMessageRecursive(msg, ezTime::MakeZero());

    msg.m_uiMaterialSlot = m_uiMaterialIndex;
    msg.m_hMaterial = m_hMaterial;
    GetOwner()->PostMessageRecursive(msg, ezTime::MakeZero());
  }
}

void ezRazorCanvas3DComponent::SetTextureSlotName(ezStringView sName)
{
  if (m_sTextureSlotName != sName)
  {
    m_hMaterial.Invalidate();
    m_sTextureSlotName.Assign(sName);
  }
}

void ezRazorCanvas3DComponent::SetTextureSize(const ezVec2U32& vSize)
{
  if (m_vSize != vSize)
  {
    m_vSize.x = ezMath::Min(vSize.x, 4096u);
    m_vSize.y = ezMath::Min(vSize.y, 4096u);

    m_hTexture.Invalidate();
  }
}

void ezRazorCanvas3DComponent::SetDpiScale(float fDpiScale)
{
  fDpiScale = fDpiScale > 0.0f ? fDpiScale : 1.0f;

  if (fDpiScale == m_fDpiScale)
    return;

  m_fDpiScale = fDpiScale;
}

void ezRazorCanvas3DComponent::SetInteractive(bool bIsInteractive)
{
  m_bIsInteractive = bIsInteractive;
}

bool ezRazorCanvas3DComponent::UpdateTextureAndMaterial()
{
  if (m_vSize.x == 0 || m_vSize.y == 0 || !m_hBaseMaterial.IsValid() || m_sTextureSlotName.IsEmpty())
    return false;

  bool bNeedsUpdate = false;

  if (!m_hTexture.IsValid())
  {
    bNeedsUpdate = true;

    ezTexture2DResourceDescriptor desc;
    desc.m_DescGAL.m_uiWidth = m_vSize.x;
    desc.m_DescGAL.m_uiHeight = m_vSize.y;
    desc.m_DescGAL.m_Format = ezGALResourceFormat::RGBAUByteNormalized;
    desc.m_DescGAL.m_ResourceAccess.m_bImmutable = false;
    desc.m_SamplerDesc.m_AddressU = ezImageAddressMode::ClampBorder;
    desc.m_SamplerDesc.m_AddressV = ezImageAddressMode::ClampBorder;
    desc.m_SamplerDesc.m_AddressW = ezImageAddressMode::ClampBorder;
    desc.m_SamplerDesc.m_BorderColor = ezColor::MakeZero();
    desc.m_SamplerDesc.m_MinFilter = ezGALTextureFilterMode::Anisotropic;
    desc.m_SamplerDesc.m_MagFilter = ezGALTextureFilterMode::Anisotropic;
    desc.m_SamplerDesc.m_uiMaxAnisotropy = 8;

    if (ezMath::IsPowerOf2(m_vSize.x) && ezMath::IsPowerOf2(m_vSize.y))
    {
      desc.m_DescGAL.m_uiMipLevelCount = ezMath::Max(ezMath::Log2i(m_vSize.x), ezMath::Log2i(m_vSize.y)) - 2;
      desc.m_DescGAL.m_TextureFlags.Add(ezGALTextureUsageFlags::DynamicMipGeneration);
    }

    ezStringBuilder resourceName = "RazorCanvas3D_Texture";
    resourceName.AppendFormat("_{0}", s_RazorResourceCounter.Increment());

    m_hTexture = ezResourceManager::CreateResource<ezTexture2DResource>(resourceName, std::move(desc));

    m_hMaterial.Invalidate();
  }

  if (!m_hMaterial.IsValid())
  {
    bNeedsUpdate = true;

    ezMaterialResourceDescriptor desc;
    desc.m_hBaseMaterial = m_hBaseMaterial;
    auto& tb = desc.m_Texture2DBindings.ExpandAndGetRef();
    tb.m_Name = m_sTextureSlotName;
    tb.m_Value = m_hTexture;

    ezStringBuilder resourceName = "RazorCanvas3D_Material";
    resourceName.AppendFormat("_{0}", s_RazorResourceCounter.Increment());

    m_hMaterial = ezResourceManager::CreateResource<ezMaterialResource>(resourceName, std::move(desc));

    ezMsgSetMeshMaterial msg;
    msg.m_hMaterial = m_hMaterial;
    msg.m_uiMaterialSlot = m_uiMaterialIndex;
    GetOwner()->PostMessageRecursive(msg, ezTime::MakeZero());
  }

  return bNeedsUpdate;
}

bool ezRazorCanvas3DComponent::RaycastMeshTexCoords(
  const ezCpuMeshResource* pMesh, ezUInt32 uiSubMeshIndex,
  const ezVec3& vRayOrigin, const ezVec3& vRayDir,
  ezVec2& out_vTexCoords, float fEpsilon)
{
  const ezMeshBufferResourceDescriptor& mesh = pMesh->GetDescriptor().MeshBufferDesc();

  if (mesh.GetTopology() != ezGALPrimitiveTopology::Triangles)
  {
    ezLog::Warning("Topology '{}' is not supported for raycasting.", mesh.GetTopology());
    return false;
  }

  const ezUInt16* pIndexBuffer = reinterpret_cast<const ezUInt16*>(mesh.GetIndexBufferData().GetPtr());
  ezUInt32 uiNumIndices = mesh.GetIndexBufferData().GetCount() / 2;
  if (mesh.Uses32BitIndices())
  {
    ezLog::Warning("Meshes with 32 bit indices are not supported for raycasting.");
    return false;
  }

  ezMeshResourceDescriptor::SubMesh submesh = pMesh->GetDescriptor().GetSubMeshes()[uiSubMeshIndex];
  ezUInt32 uiFirstIndex = submesh.m_uiFirstPrimitive * 3;
  ezUInt32 uiLastIndex = uiFirstIndex + submesh.m_uiPrimitiveCount * 3;
  EZ_ASSERT_DEV(uiLastIndex <= uiNumIndices, "something is wrong");

  float fClosestDist = 1e20f;
  ezUInt16 uiClosestIndex0 = 0, uiClosestIndex1 = 0, uiClosestIndex2 = 0;
  ezVec3 vClosestPos;

  for (ezUInt32 i = uiFirstIndex; i + 2 < uiLastIndex; i += 3)
  {
    ezUInt16 uiIndex0 = pIndexBuffer[i];
    ezUInt16 uiIndex1 = pIndexBuffer[i + 1];
    ezUInt16 uiIndex2 = pIndexBuffer[i + 2];

    ezVec3 vVertex0 = mesh.GetPosition(uiIndex0);
    ezVec3 vVertex1 = mesh.GetPosition(uiIndex1);
    ezVec3 vVertex2 = mesh.GetPosition(uiIndex2);

    float fDist;
    ezVec3 vPos;

    bool bHit = ezIntersectionUtils::RayTriangleIntersectionCullBackface(vRayOrigin, vRayDir, vVertex0, vVertex1, vVertex2, vPos, &fDist, nullptr);
    if (!bHit || fDist > fClosestDist)
      continue;

    fClosestDist = fDist;
    uiClosestIndex0 = uiIndex0;
    uiClosestIndex1 = uiIndex1;
    uiClosestIndex2 = uiIndex2;
    vClosestPos = vPos;
  }

  if (fClosestDist < 1e20f)
  {
    out_vTexCoords = ezVec2::MakeZero();
    out_vTexCoords += mesh.GetTexCoord0(uiClosestIndex0) * vClosestPos.x;
    out_vTexCoords += mesh.GetTexCoord0(uiClosestIndex1) * vClosestPos.y;
    out_vTexCoords += mesh.GetTexCoord0(uiClosestIndex2) * vClosestPos.z;
    out_vTexCoords.x = ezMath::Fraction(ezMath::Abs(out_vTexCoords.x));
    out_vTexCoords.y = ezMath::Fraction(ezMath::Abs(out_vTexCoords.y));

    return true;
  }

  return false;
}

EZ_STATICLINK_FILE(RazorPlugin, RazorPlugin_Components_RazorCanvas3DComponent);
