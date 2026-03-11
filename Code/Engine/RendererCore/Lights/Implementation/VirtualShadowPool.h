#pragma once

#include <Core/Graphics/Camera.h>
#include <Foundation/Configuration/CVar.h>
#include <RendererCore/Lights/Implementation/VirtualShadowPageTable.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererFoundation/RendererFoundationDLL.h>

struct ezVSMConstants;
struct ezRenderWorldExtractionEvent;
struct ezRenderWorldRenderEvent;
class ezView;
class ezWorld;
class ezGALCommandEncoder;

/// Manages virtual shadow maps for high-resolution directional light shadows.
///
/// Replaces the atlas-based shadow system with a clipmap-based virtual shadow map approach.
/// Each clipmap level is divided into pages (16x16 per level) that are allocated on demand
/// based on GPU feedback from the depth buffer. Physical pages are stored in a single D16
/// shadow atlas (8192x8192) and reused via LRU eviction.
///
/// The pipeline flow is:
/// 1. Page marking (GPU compute): VSMPageMarkingPass writes to PageRequestBuffer
/// 2. CPU readback: Update() reads requests and allocates physical pages
/// 3. Page table upload (GPU compute): uploads CPU allocation results
/// 4. Shadow rendering: creates views for dirty pages, renders into physical atlas
/// 5. Sampling: shaders use SampleVirtualShadowMap() to look up shadow via page table
///
/// Controlled by CVar "Rendering.Shadows.UseVirtualShadowMaps".
class EZ_RENDERERCORE_DLL ezVirtualShadowPool
{
public:
  static void Initialize();
  static void DeInitialize();

  static bool IsEnabled();

  /// Called each frame during extraction to update clipmap transforms and process page requests.
  static void Update(const ezVec3& vCameraPosition, const ezVec3& vLightDirection, ezUInt32 uiFrameCounter, const ezWorld* pWorld);

  static ezGALTextureHandle GetPageTableTexture();
  static ezGALTextureHandle GetPhysicalAtlasTexture();
  static ezGALBufferHandle GetPageRequestBuffer();

  /// Get the VSM constant buffer for shader binding.
  static ezConstantBufferStorageHandle GetConstantBuffer();

  /// Get the allocation results buffer for the page table update shader.
  static ezGALBufferHandle GetAllocationResultsBuffer();

private:
  struct ShadowView
  {
    ezViewHandle m_hView;
    ezCamera m_Camera;
    ezCamera m_CullingCamera;
  };

  static void UpdateClipmaps(const ezVec3& vCameraPosition, const ezVec3& vLightDirection);
  static void ProcessPageRequests(ezUInt32 uiFrameCounter);
  static void UploadPageTable(ezGALCommandEncoder* pCommandEncoder);
  static void UploadConstants(ezGALCommandEncoder* pCommandEncoder);
  static void CreateShadowViewsForDirtyPages(const ezVec3& vLightDirection);
  static ShadowView& GetOrCreateShadowView();

  static void OnExtractionEvent(const ezRenderWorldExtractionEvent& e);
  static void OnRenderEvent(const ezRenderWorldRenderEvent& e);

  static bool s_bInitialized;
  static ezVirtualShadowPageTable s_PageTable;
  static ezGALTextureHandle s_hPageTableTexture;
  static ezGALTextureHandle s_hPhysicalAtlas;
  static ezGALBufferHandle s_hPageRequestBuffer;
  static ezGALBufferHandle s_hAllocationResultsBuffer;
  static ezConstantBufferStorageHandle s_hConstantBuffer;
  static ezDynamicArray<ezUInt32> s_DirtyPages;
  static ezUInt32 s_uiFrameCounter;
  static ezVec3 s_vClipmapCenter;
  static ezVec3 s_vLightDirection;
  static ezVec3 s_vLightRight;
  static ezVec3 s_vLightUp;
  static ezMat4 s_ClipmapWorldToUV[8];
  static float s_fClipmapLevelSizes[8];
  static ezVec3 s_vSnappedClipmapCenter[8];

  // Shadow view pool
  static ezDeque<ShadowView> s_ShadowViews;
  static ezUInt32 s_uiUsedShadowViews;

  // Per-frame page table data for GPU upload
  static ezDynamicArray<ezUInt32> s_AllocationResultsCPU;

  // Track whether Update was called this frame
  static bool s_bUpdateCalled;
  static bool s_bAtlasNeedsFullClear;
  static const ezWorld* s_pWorld;
};
