#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Configuration/CVar.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Utilities/GraphicsUtils.h>
#include <RendererCore/Lights/Implementation/VirtualShadowPool.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererFoundation/CommandEncoder/CommandEncoder.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Resources/Texture.h>

#include <RendererCore/../../../Data/Base/Shaders/Common/VirtualShadowMapData.h>

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(RendererCore, VirtualShadowPool)
  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core",
    "RenderWorld"
  END_SUBSYSTEM_DEPENDENCIES

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
    ezVirtualShadowPool::Initialize();
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    ezVirtualShadowPool::DeInitialize();
  }
EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

ezCVarBool cvar_UseVirtualShadowMaps("Rendering.Shadows.UseVirtualShadowMaps", false, ezCVarFlags::Default, "Enable virtual shadow maps for directional lights");
ezCVarFloat cvar_RenderingVSMDepthBias("Rendering.Shadows.VSMDepthBias", 0.0003f, ezCVarFlags::Default, "VSM constant depth bias");
ezCVarFloat cvar_RenderingVSMNormalBias("Rendering.Shadows.VSMNormalBias", 0.05f, ezCVarFlags::Default, "VSM normal offset bias");

// Shadow camera parameters: must be consistent between rendering (CreateShadowViewsForDirtyPages)
// and sampling (ClipmapWorldToUV z-component in UpdateClipmaps).
// The depth range should cover geometry at reasonable distances along the light direction.
// Keep it as tight as possible to preserve D16 precision (total range / 65535 = meters per step).
static constexpr float VSM_CAMERA_NEAR_OFFSET = 500.0f;
static constexpr float VSM_SHADOW_FAR_PLANE = VSM_CAMERA_NEAR_OFFSET + 500.0f;

bool ezVirtualShadowPool::s_bInitialized = false;
ezVirtualShadowPageTable ezVirtualShadowPool::s_PageTable;
ezGALTextureHandle ezVirtualShadowPool::s_hPageTableTexture;
ezGALTextureHandle ezVirtualShadowPool::s_hPhysicalAtlas;
ezGALBufferHandle ezVirtualShadowPool::s_hPageRequestBuffer;
ezGALBufferHandle ezVirtualShadowPool::s_hAllocationResultsBuffer;
ezConstantBufferStorageHandle ezVirtualShadowPool::s_hConstantBuffer;
ezDynamicArray<ezUInt32> ezVirtualShadowPool::s_DirtyPages;
ezUInt32 ezVirtualShadowPool::s_uiFrameCounter = 0;
ezVec3 ezVirtualShadowPool::s_vClipmapCenter = ezVec3::MakeZero();
ezVec3 ezVirtualShadowPool::s_vLightDirection = ezVec3(0, 0, -1);
ezVec3 ezVirtualShadowPool::s_vLightRight = ezVec3(1, 0, 0);
ezVec3 ezVirtualShadowPool::s_vLightUp = ezVec3(0, 1, 0);
ezMat4 ezVirtualShadowPool::s_ClipmapWorldToUV[8] = {};
float ezVirtualShadowPool::s_fClipmapLevelSizes[8] = {};
float ezVirtualShadowPool::s_fLightPenumbraSize = 0.05f;
ezDeque<ezVirtualShadowPool::ShadowView> ezVirtualShadowPool::s_ShadowViews;
ezUInt32 ezVirtualShadowPool::s_uiUsedShadowViews = 0;
ezDynamicArray<ezUInt32> ezVirtualShadowPool::s_AllocationResultsCPU;
bool ezVirtualShadowPool::s_bUpdateCalled = false;
bool ezVirtualShadowPool::s_bAtlasNeedsFullClear = true;
const ezWorld* ezVirtualShadowPool::s_pWorld = nullptr;

void ezVirtualShadowPool::Initialize()
{
  if (s_bInitialized)
    return;

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  // Page table: Texture2DArray<uint> - one slice per clipmap level
  {
    ezGALTextureCreationDescription desc;
    desc.m_uiWidth = VSM_PAGES_PER_LEVEL;
    desc.m_uiHeight = VSM_PAGES_PER_LEVEL;
    desc.m_uiArraySize = VSM_MAX_CLIPMAP_LEVELS;
    desc.m_Type = ezGALTextureType::Texture2DArray;
    desc.m_Format = ezGALResourceFormat::RUInt;
    desc.m_TextureFlags = ezGALTextureUsageFlags::ShaderResource;
    desc.m_ResourceAccess.m_bImmutable = false;

    s_hPageTableTexture = pDevice->CreateTexture(desc);
    EZ_ASSERT_DEV(!s_hPageTableTexture.IsInvalidated(), "Failed to create VSM page table texture");
  }

  // Physical shadow atlas: D16 format
  {
    ezGALTextureCreationDescription desc;
    desc.m_uiWidth = VSM_PHYSICAL_ATLAS_SIZE;
    desc.m_uiHeight = VSM_PHYSICAL_ATLAS_SIZE;
    desc.m_Format = ezGALResourceFormat::D16;
    desc.m_TextureFlags = ezGALTextureUsageFlags::ShaderResource | ezGALTextureUsageFlags::RenderTarget;
    desc.m_ResourceAccess.m_bImmutable = false;

    s_hPhysicalAtlas = pDevice->CreateTexture(desc);
    EZ_ASSERT_DEV(!s_hPhysicalAtlas.IsInvalidated(), "Failed to create VSM physical atlas texture");
  }

  // Page request buffer: one uint per virtual page across all levels
  {
    const ezUInt32 totalPages = VSM_MAX_CLIPMAP_LEVELS * VSM_PAGES_PER_LEVEL * VSM_PAGES_PER_LEVEL;

    ezGALBufferCreationDescription desc;
    desc.m_uiStructSize = sizeof(ezUInt32);
    desc.m_uiTotalSize = desc.m_uiStructSize * totalPages;
    desc.m_BufferFlags = ezGALBufferUsageFlags::StructuredBuffer | ezGALBufferUsageFlags::ShaderResource | ezGALBufferUsageFlags::UnorderedAccess;
    desc.m_ResourceAccess.m_bImmutable = false;

    s_hPageRequestBuffer = pDevice->CreateBuffer(desc);
  }

  // Allocation results buffer: CPU writes allocation decisions, GPU reads in page table update shader
  {
    const ezUInt32 totalPages = VSM_MAX_CLIPMAP_LEVELS * VSM_PAGES_PER_LEVEL * VSM_PAGES_PER_LEVEL;

    ezGALBufferCreationDescription desc;
    desc.m_uiStructSize = sizeof(ezUInt32);
    desc.m_uiTotalSize = desc.m_uiStructSize * totalPages;
    desc.m_BufferFlags = ezGALBufferUsageFlags::StructuredBuffer | ezGALBufferUsageFlags::ShaderResource;
    desc.m_ResourceAccess.m_bImmutable = false;

    s_hAllocationResultsBuffer = pDevice->CreateBuffer(desc);
  }

  s_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezVSMConstants>();

  s_PageTable.Initialize(VSM_PAGES_PER_LEVEL, VSM_MAX_CLIPMAP_LEVELS, VSM_MAX_PHYSICAL_PAGES);

  // Initialize clipmap level sizes (each level covers 2x the area of the previous)
  float baseSize = 20.0f;
  for (ezUInt32 i = 0; i < VSM_MAX_CLIPMAP_LEVELS; ++i)
  {
    s_fClipmapLevelSizes[i] = baseSize;
    baseSize *= 2.0f;
  }

  const ezUInt32 totalPages = VSM_MAX_CLIPMAP_LEVELS * VSM_PAGES_PER_LEVEL * VSM_PAGES_PER_LEVEL;
  s_AllocationResultsCPU.SetCount(totalPages);

  // Register event handlers following the same pattern as ezShadowPool
  ezRenderWorld::GetExtractionEvent().AddEventHandler(OnExtractionEvent);
  ezRenderWorld::GetRenderEvent().AddEventHandler(OnRenderEvent);

  s_bInitialized = true;
}

void ezVirtualShadowPool::DeInitialize()
{
  if (!s_bInitialized)
    return;

  ezRenderWorld::GetExtractionEvent().RemoveEventHandler(OnExtractionEvent);
  ezRenderWorld::GetRenderEvent().RemoveEventHandler(OnRenderEvent);

  for (auto& sv : s_ShadowViews)
  {
    ezRenderWorld::DeleteView(sv.m_hView);
  }
  s_ShadowViews.Clear();

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
  pDevice->DestroyTexture(s_hPageTableTexture);
  pDevice->DestroyTexture(s_hPhysicalAtlas);
  pDevice->DestroyBuffer(s_hPageRequestBuffer);
  pDevice->DestroyBuffer(s_hAllocationResultsBuffer);

  ezRenderContext::DeleteConstantBufferStorage(s_hConstantBuffer);

  s_PageTable.Reset();
  s_AllocationResultsCPU.Clear();
  s_pWorld = nullptr;
  s_bInitialized = false;
}

bool ezVirtualShadowPool::IsEnabled()
{
  return cvar_UseVirtualShadowMaps && s_bInitialized;
}

void ezVirtualShadowPool::Update(const ezVec3& vCameraPosition, const ezVec3& vLightDirection, ezUInt32 uiFrameCounter, const ezWorld* pWorld, float fLightPenumbraSize)
{
  if (!s_bInitialized)
    return;

  s_uiFrameCounter = uiFrameCounter;
  s_fLightPenumbraSize = fLightPenumbraSize;

  // Detect scene change: when the world pointer changes, all cached shadow data is invalid.
  if (pWorld != s_pWorld)
  {
    s_PageTable.Reset();
    s_bAtlasNeedsFullClear = true;
  }

  s_pWorld = pWorld;

  // vLightDirection is m_vDirection (rotation * -X), the direction light rays travel (toward scene).
  // The shadow camera convention (matching CSM) uses the opposite: the direction the light "faces"
  // (rotation * +X = GetGlobalDirForwards), where the camera is placed behind the light looking forward.
  // Negate here so all internal code uses the CSM-compatible "light forward" convention.
  const ezVec3 vLightForward = -vLightDirection;
  s_vLightDirection = vLightForward;

  UpdateClipmaps(vCameraPosition, vLightForward);
  ProcessPageRequests(uiFrameCounter);

  // Create shadow views for dirty pages during extraction (matching ShadowPool's pattern).
  // This ensures views are added before EndExtraction, avoiding "extracted multiple times" issues.
  s_uiUsedShadowViews = 0;
  CreateShadowViewsForDirtyPages(vLightForward);

  s_bUpdateCalled = true;
}

void ezVirtualShadowPool::UpdateClipmaps(const ezVec3& vCameraPosition, const ezVec3& vLightDirection)
{
  ezVec3 lightDir = vLightDirection.IsZero() ? ezVec3(0, 0, -1) : vLightDirection.GetNormalized();
  ezVec3 right = lightDir.CrossRH(ezVec3(0, 0, 1));
  if (right.GetLengthSquared() < 0.001f)
    right = lightDir.CrossRH(ezVec3(0, 1, 0));
  right.Normalize();
  // Compute up so that LookAt's left-handed basis has xaxis=right, yaxis=up, zaxis=lightDir.
  // LookAt computes xaxis = up.CrossRH(lightDir), so we need up such that up.CrossRH(lightDir) = right.
  // Setting up = lightDir.CrossRH(right) satisfies this: (lightDir x right) x lightDir = right.
  ezVec3 up = lightDir.CrossRH(right);
  up.Normalize();

  // Store light-space basis for use by CreateShadowViewsForDirtyPages
  s_vLightRight = right;
  s_vLightUp = up;

  // SVSM reference: origin-centered sample matrix.
  //
  // The sample matrix (ClipmapWorldToUV) uses NO camera-dependent translation in XY.
  // This makes virtual UVs completely stable across camera movement — only the light
  // direction affects them. The shader uses frac() for toroidal wrapping since UVs
  // can exceed [0,1].
  //
  // The view matrix is positioned at -lightDir * NEAR_OFFSET (along the light axis only,
  // zero XY offset) so the Z depth matches the per-page render cameras.
  //
  // When the camera moves, render cameras wrap toroidally: each page is rendered at the
  // world region closest to the camera that maps to that virtual page via frac().
  // Only pages whose wrapped world region changed (wrap offset differs) need re-rendering.

  ezVec3 sampleOrigin = -lightDir * VSM_CAMERA_NEAR_OFFSET;
  ezMat4 viewMatrixSample = ezGraphicsUtils::CreateLookAtViewMatrix(
    sampleOrigin, sampleOrigin + lightDir, up, ezHandedness::LeftHanded);

  for (ezUInt32 level = 0; level < VSM_MAX_CLIPMAP_LEVELS; ++level)
  {
    float levelSize = s_fClipmapLevelSizes[level];

    ezMat4 projMatrix = ezGraphicsUtils::CreateOrthographicProjectionMatrix(
      levelSize, levelSize, 0.0f, VSM_SHADOW_FAR_PLANE,
      ezClipSpaceDepthRange::ZeroToOne, ezClipSpaceYMode::Regular, ezHandedness::LeftHanded);

    ezMat4 viewProjSample = projMatrix * viewMatrixSample;

    // Remap XY from clip-space [-1,1] to virtual UV [0,1].
    // Y is FLIPPED (DX11 convention, matching CSM's atlasScaleOffset.y = -0.5).
    // Z passes through unchanged (already [0,1] from the ZeroToOne projection).
    ezMat4 clipToUV = ezMat4::MakeIdentity();
    clipToUV.Element(0, 0) = 0.5f;
    clipToUV.Element(1, 1) = -0.5f;
    clipToUV.Element(3, 0) = 0.5f;
    clipToUV.Element(3, 1) = 0.5f;

    s_ClipmapWorldToUV[level] = clipToUV * viewProjSample;

    // Toroidal dirty tracking: compute each page's current wrap offset.
    // The origin-centered sample matrix maps page (px, py) to a fixed region in light-space.
    // The render camera wraps that region to be near the camera. The wrap offset is:
    //   wrapX = round((camera_right - page_origin_right) / levelSize)
    //   wrapY = round((camera_up - page_origin_up) / levelSize)
    // When this changes, the page covers a different world region and must be re-rendered.
    float pageWorldSize = levelSize / float(VSM_PAGES_PER_LEVEL);
    float halfSize = levelSize * 0.5f;
    float projRight = right.Dot(vCameraPosition);
    float projUp = up.Dot(vCameraPosition);

    for (ezUInt32 py = 0; py < VSM_PAGES_PER_LEVEL; ++py)
    {
      for (ezUInt32 px = 0; px < VSM_PAGES_PER_LEVEL; ++px)
      {
        // Page-to-world mapping derived from the sample matrix UV formulas:
        //   uv_x = right.dot(P)/levelSize + 0.5   (xaxis = right in LH view matrix)
        //   uv_y = -up.dot(P)/levelSize + 0.5      (yaxis = up, but Y-flip in clipToUV)
        //
        // For page (px, py), the UV center is ((px+0.5)/16, (py+0.5)/16), so:
        //   right.dot(pageCenter) = -halfSize + (px+0.5)*pageWorldSize
        //   up.dot(pageCenter)    = halfSize - (py+0.5)*pageWorldSize
        float originRight = -halfSize + (px + 0.5f) * pageWorldSize;
        float originUp = halfSize - (py + 0.5f) * pageWorldSize;

        ezInt32 wrapX = (ezInt32)ezMath::Round((projRight - originRight) / levelSize);
        ezInt32 wrapY = (ezInt32)ezMath::Round((projUp - originUp) / levelSize);

        const auto& pageInfo = s_PageTable.GetPageInfo(level, px, py);
        if (pageInfo.m_iWrapOffsetX != wrapX || pageInfo.m_iWrapOffsetY != wrapY)
        {
          s_PageTable.MarkPageDirty(level, px, py);
        }
      }
    }
  }

  // Store the camera position in light-space for render camera positioning
  s_vClipmapCenter = vCameraPosition;
}

void ezVirtualShadowPool::ProcessPageRequests(ezUInt32 uiFrameCounter)
{
  const ezUInt32 totalPages = VSM_MAX_CLIPMAP_LEVELS * VSM_PAGES_PER_LEVEL * VSM_PAGES_PER_LEVEL;

  // CPU-side page request heuristic: request all pages within each clipmap level's coverage.
  // This avoids needing GPU readback with its frame latency. For a 16x16 page grid with 8 levels
  // this is 2048 pages total, well within budget of 4096 physical pages.
  for (ezUInt32 level = 0; level < VSM_MAX_CLIPMAP_LEVELS; ++level)
  {
    for (ezUInt32 py = 0; py < VSM_PAGES_PER_LEVEL; ++py)
    {
      for (ezUInt32 px = 0; px < VSM_PAGES_PER_LEVEL; ++px)
      {
        s_PageTable.MarkPageRequested(level, px, py, uiFrameCounter);
      }
    }
  }

  s_PageTable.AllocateRequestedPages(uiFrameCounter, s_DirtyPages);

  // Build page table texture data for GPU upload (proper page table entries with valid bit + page index)
  for (ezUInt32 i = 0; i < totalPages; ++i)
  {
    s_AllocationResultsCPU[i] = VSM_PAGE_INVALID;
  }

  for (ezUInt32 level = 0; level < VSM_MAX_CLIPMAP_LEVELS; ++level)
  {
    for (ezUInt32 py = 0; py < VSM_PAGES_PER_LEVEL; ++py)
    {
      for (ezUInt32 px = 0; px < VSM_PAGES_PER_LEVEL; ++px)
      {
        const auto& pageInfo = s_PageTable.GetPageInfo(level, px, py);
        if (pageInfo.m_uiPhysicalPageIndex != 0xFFFF)
        {
          ezUInt32 flatIdx = level * VSM_PAGES_PER_LEVEL * VSM_PAGES_PER_LEVEL + py * VSM_PAGES_PER_LEVEL + px;
          s_AllocationResultsCPU[flatIdx] = VSM_PAGE_VALID_BIT | (pageInfo.m_uiPhysicalPageIndex & VSM_PAGE_INDEX_MASK);
          if (pageInfo.m_bDirty)
            s_AllocationResultsCPU[flatIdx] |= VSM_PAGE_DIRTY_BIT;
        }
      }
    }
  }
}

void ezVirtualShadowPool::UploadPageTable(ezGALCommandEncoder* pCommandEncoder)
{
  // Upload page table data directly to the texture, one slice per clipmap level
  const ezUInt32 pagesPerLevel = VSM_PAGES_PER_LEVEL * VSM_PAGES_PER_LEVEL;

  for (ezUInt32 level = 0; level < VSM_MAX_CLIPMAP_LEVELS; ++level)
  {
    ezGALTextureSubresource subResource;
    subResource.m_uiMipLevel = 0;
    subResource.m_uiArraySlice = level;

    ezBoundingBoxu32 destBox;
    destBox.m_vMin = ezVec3U32(0, 0, 0);
    destBox.m_vMax = ezVec3U32(VSM_PAGES_PER_LEVEL, VSM_PAGES_PER_LEVEL, 1);

    ezGALSystemMemoryDescription memDesc;
    memDesc.m_pData = ezMakeByteBlobPtr(&s_AllocationResultsCPU[level * pagesPerLevel], pagesPerLevel);
    memDesc.m_uiRowPitch = VSM_PAGES_PER_LEVEL * sizeof(ezUInt32);
    memDesc.m_uiSlicePitch = pagesPerLevel * sizeof(ezUInt32);

    pCommandEncoder->UpdateTexture(s_hPageTableTexture, subResource, destBox, memDesc);
  }
}

void ezVirtualShadowPool::UploadConstants(ezGALCommandEncoder* pCommandEncoder)
{
  auto* cb = ezRenderContext::GetConstantBufferData<ezVSMConstants>(s_hConstantBuffer);
  cb->PageSize = VSM_PAGE_SIZE;
  cb->PhysicalAtlasSize = VSM_PHYSICAL_ATLAS_SIZE;
  cb->NumClipmapLevels = VSM_MAX_CLIPMAP_LEVELS;
  cb->VSMPadding2 = 0;

  cb->ClipmapCenter.Set(s_vClipmapCenter.x, s_vClipmapCenter.y, s_vClipmapCenter.z, 0.0f);

  for (ezUInt32 i = 0; i < VSM_MAX_CLIPMAP_LEVELS; ++i)
  {
    cb->ClipmapLevelWorldSize[i].Set(s_fClipmapLevelSizes[i], s_fClipmapLevelSizes[i], s_fClipmapLevelSizes[i], 0.0f);
    cb->ClipmapWorldToUV[i] = s_ClipmapWorldToUV[i];
  }

  cb->VSMDepthBias = cvar_RenderingVSMDepthBias;
  cb->VSMNormalBias = cvar_RenderingVSMNormalBias;
  cb->VSMLightSize = s_fLightPenumbraSize;
  cb->VSMPadding1 = 0;
}

ezVirtualShadowPool::ShadowView& ezVirtualShadowPool::GetOrCreateShadowView()
{
  if (s_uiUsedShadowViews < static_cast<ezUInt32>(s_ShadowViews.GetCount()))
  {
    return s_ShadowViews[s_uiUsedShadowViews++];
  }

  auto& sv = s_ShadowViews.ExpandAndGetRef();

  ezView* pView = nullptr;
  sv.m_hView = ezRenderWorld::CreateView("VSM Shadow", pView);

  pView->SetCameraUsageHint(ezCameraUsageHint::Shadow);

  ezGALRenderTargets renderTargets;
  renderTargets.m_hDSTarget = s_hPhysicalAtlas;
  pView->SetRenderTargets(renderTargets);

  // ShadowMapRenderPipeline.ezRenderPipelineAsset
  pView->SetRenderPipelineResource(
    ezResourceManager::LoadResource<ezRenderPipelineResource>("{ 4f4d9f16-3d47-4c67-b821-a778f11dcaf5 }"));

  pView->SetViewport(ezRectFloat(0.0f, 0.0f, (float)VSM_PAGE_SIZE, (float)VSM_PAGE_SIZE));

  const ezTag& tagCastShadows = ezTagRegistry::GetGlobalRegistry().RegisterTag("CastShadow");
  pView->m_IncludeTags.Set(tagCastShadows);
  pView->m_ExcludeTags.SetByName("EditorHidden");

  s_uiUsedShadowViews++;
  return sv;
}

void ezVirtualShadowPool::CreateShadowViewsForDirtyPages(const ezVec3& vLightDirection)
{
  EZ_PROFILE_SCOPE("VSM Create Shadow Views");

  ezVec3 lightDir = vLightDirection.IsZero() ? ezVec3(0, 0, -1) : vLightDirection.GetNormalized();
  const ezVec3& right = s_vLightRight;
  const ezVec3& up = s_vLightUp;

  // s_vClipmapCenter stores the current camera position (set in UpdateClipmaps)
  const ezVec3& vCameraPosition = s_vClipmapCenter;
  float projRight = right.Dot(vCameraPosition);
  float projUp = up.Dot(vCameraPosition);

  // Render all dirty pages this frame. With the origin-centered sample matrix and toroidal
  // scrolling, only pages at the scroll boundary are dirty (typically 1 row + 1 column per level
  // per frame of camera movement = ~31 pages), so no budget limit is needed.
  for (ezUInt32 dirtyIdx = 0; dirtyIdx < s_DirtyPages.GetCount(); ++dirtyIdx)
  {
    const ezUInt32 flatPageIdx = s_DirtyPages[dirtyIdx];

    const ezUInt32 pagesPerLevel = VSM_PAGES_PER_LEVEL * VSM_PAGES_PER_LEVEL;
    const ezUInt32 level = flatPageIdx / pagesPerLevel;
    const ezUInt32 remainder = flatPageIdx % pagesPerLevel;
    const ezUInt32 py = remainder / VSM_PAGES_PER_LEVEL;
    const ezUInt32 px = remainder % VSM_PAGES_PER_LEVEL;

    if (level >= VSM_MAX_CLIPMAP_LEVELS)
      continue;

    const auto& pageInfo = s_PageTable.GetPageInfo(level, px, py);
    if (pageInfo.m_uiPhysicalPageIndex == 0xFFFF)
      continue;

    float levelSize = s_fClipmapLevelSizes[level];
    float halfSize = levelSize * 0.5f;
    float pageWorldSize = levelSize / float(VSM_PAGES_PER_LEVEL);

    // Page-to-world mapping derived from the sample matrix UV formulas:
    //   uv_x = right.dot(P)/levelSize + 0.5   (xaxis = right in LH view matrix)
    //   uv_y = -up.dot(P)/levelSize + 0.5      (yaxis = up, but Y-flip in clipToUV)
    // So: right.dot(pageCenter) = -halfSize + (px+0.5)*pageWorldSize
    //     up.dot(pageCenter)    = halfSize - (py+0.5)*pageWorldSize
    float originRight = -halfSize + (px + 0.5f) * pageWorldSize;
    float originUp = halfSize - (py + 0.5f) * pageWorldSize;

    // Toroidal wrap: offset by integer multiples of levelSize to get the world region
    // closest to the camera that maps to this virtual page via frac()
    ezInt32 wrapX = (ezInt32)ezMath::Round((projRight - originRight) / levelSize);
    ezInt32 wrapY = (ezInt32)ezMath::Round((projUp - originUp) / levelSize);

    float renderRight = originRight + wrapX * levelSize;
    float renderUp = originUp + wrapY * levelSize;

    ezVec3 pageCenter = right * renderRight + up * renderUp;
    ezVec3 shadowCameraPos = pageCenter - lightDir * VSM_CAMERA_NEAR_OFFSET;

    ShadowView& sv = GetOrCreateShadowView();

    sv.m_Camera.LookAt(shadowCameraPos, pageCenter, up);
    sv.m_Camera.SetCameraMode(ezCameraMode::OrthoFixedWidth, pageWorldSize, 0.0f, VSM_SHADOW_FAR_PLANE);

    // Extend culling camera behind the shadow camera to capture geometry at any distance along light direction
    sv.m_CullingCamera = sv.m_Camera;
    sv.m_CullingCamera.SetCameraMode(ezCameraMode::OrthoFixedWidth, pageWorldSize, -VSM_CAMERA_NEAR_OFFSET, VSM_SHADOW_FAR_PLANE);

    // Compute atlas coordinates from the physical page index
    const ezUInt32 physPage = pageInfo.m_uiPhysicalPageIndex;
    const ezUInt32 atlasX = (physPage % VSM_PAGES_PER_ATLAS_DIM) * VSM_PAGE_SIZE;
    const ezUInt32 atlasY = (physPage / VSM_PAGES_PER_ATLAS_DIM) * VSM_PAGE_SIZE;

    ezView* pView;
    if (ezRenderWorld::TryGetView(sv.m_hView, pView))
    {
      if (s_pWorld != nullptr)
        pView->SetWorld(const_cast<ezWorld*>(s_pWorld));

      pView->SetCamera(&sv.m_Camera);
      pView->SetCullingCamera(&sv.m_CullingCamera);
      pView->SetViewport(ezRectFloat((float)atlasX, (float)atlasY, (float)VSM_PAGE_SIZE, (float)VSM_PAGE_SIZE));

      ezRenderWorld::AddViewToRender(sv.m_hView);
    }

    // Store the wrap offset so we know what world region this page was rendered for.
    // Clear the dirty flag now that we've queued this page for rendering.
    s_PageTable.SetWrapOffset(level, px, py, wrapX, wrapY);
    s_PageTable.ClearDirtyFlag(level, px, py);
  }
}

void ezVirtualShadowPool::OnExtractionEvent(const ezRenderWorldExtractionEvent& e)
{
  if (e.m_Type != ezRenderWorldExtractionEvent::Type::EndExtraction)
    return;

  if (!IsEnabled() || !s_bUpdateCalled)
    return;

  // Shadow views are now created during Update() (called from ClusteredDataExtractor).
  // Just reset the flag here.
  s_bUpdateCalled = false;
}

void ezVirtualShadowPool::OnRenderEvent(const ezRenderWorldRenderEvent& e)
{
  if (e.m_Type != ezRenderWorldRenderEvent::Type::BeginRender)
    return;

  if (!IsEnabled())
    return;

  EZ_PROFILE_SCOPE("VSM Begin Render");

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
  ezGALCommandEncoder* pCommandEncoder = pDevice->BeginCommands("VSM Update");

  // Clear the entire atlas when first enabled or when the page table is rebuilt
  if (s_bAtlasNeedsFullClear)
  {
    auto rtv = pDevice->GetDefaultRenderTargetView(s_hPhysicalAtlas);
    ezGALRenderingSetup renderingSetup;
    renderingSetup.SetDepthStencilTarget(rtv);
    renderingSetup.SetClearDepth();

    pCommandEncoder->BeginRendering(renderingSetup);
    pCommandEncoder->EndRendering();

    s_bAtlasNeedsFullClear = false;
  }

  UploadPageTable(pCommandEncoder);
  UploadConstants(pCommandEncoder);

  pDevice->EndCommands(pCommandEncoder);
}

ezGALTextureHandle ezVirtualShadowPool::GetPageTableTexture() { return s_hPageTableTexture; }
ezGALTextureHandle ezVirtualShadowPool::GetPhysicalAtlasTexture() { return s_hPhysicalAtlas; }
ezGALBufferHandle ezVirtualShadowPool::GetPageRequestBuffer() { return s_hPageRequestBuffer; }
ezConstantBufferStorageHandle ezVirtualShadowPool::GetConstantBuffer() { return s_hConstantBuffer; }
ezGALBufferHandle ezVirtualShadowPool::GetAllocationResultsBuffer() { return s_hAllocationResultsBuffer; }

EZ_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_VirtualShadowPool);
