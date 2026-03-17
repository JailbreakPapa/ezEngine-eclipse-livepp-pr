#pragma once

#include <RendererDX12/RendererDX12DLL.h>
#include <RendererFoundation/CommandEncoder/CommandEncoderPlatformInterface.h>
#include <RendererFoundation/Resources/RenderTargetSetup.h>
#include <RendererDX12/Utils/ResourceBarrierDX12.h>

#include <d3d12.h>

class ezGALDeviceDX12;
class ezGALPipelineLayoutDX12;

class EZ_RENDERERDX12_DLL ezGALCommandEncoderImplDX12 final : public ezGALCommandEncoderCommonPlatformInterface
{
public:
  ezGALCommandEncoderImplDX12(ezGALDeviceDX12& ref_deviceDX12);
  ~ezGALCommandEncoderImplDX12();

  void BeginCommands(const char* szName);
  void EndCommands();
  void EndFrame();

  ID3D12GraphicsCommandList* GetCommandList() const { return m_pCommandList; }

  /// Returns the command list that was closed by the most recent EndCommands call.
  /// Only valid between EndCommands and the next BeginCommands.
  ID3D12GraphicsCommandList* GetClosedCommandList() const { return m_pClosedCommandList; }

  /// Provides access to the barrier tracker for issuing resource transitions.
  ezResourceBarrierDX12& GetBarrierTracker() { return m_BarrierTracker; }

  // ezGALCommandEncoderCommonPlatformInterface
  // Resource binding
  virtual void SetBindGroupPlatform(ezUInt32 uiBindGroup, const ezGALBindGroupCreationDescription& bindGroup) override;
  virtual void SetBindGroupPlatform(ezUInt32 uiBindGroup, const ezGALBindGroup* pBindGroup) override;
  virtual void SetPushConstantsPlatform(ezArrayPtr<const ezUInt8> data) override;

  // GPU -> CPU query functions
  virtual ezGALTimestampHandle InsertTimestampPlatform() override;
  virtual ezGALOcclusionHandle BeginOcclusionQueryPlatform(ezEnum<ezGALQueryType> type) override;
  virtual void EndOcclusionQueryPlatform(ezGALOcclusionHandle hOcclusion) override;
  virtual ezGALFenceHandle InsertFencePlatform() override;

  // Resource copy/update
  virtual void CopyBufferPlatform(const ezGALBuffer* pDestination, const ezGALBuffer* pSource) override;
  virtual void CopyBufferRegionPlatform(const ezGALBuffer* pDestination, ezUInt32 uiDestOffset, const ezGALBuffer* pSource, ezUInt32 uiSourceOffset, ezUInt32 uiByteCount) override;
  virtual void UpdateBufferPlatform(const ezGALBuffer* pDestination, ezUInt32 uiDestOffset, ezArrayPtr<const ezUInt8> sourceData, ezGALUpdateMode::Enum updateMode) override;
  virtual void CopyTexturePlatform(const ezGALTexture* pDestination, const ezGALTexture* pSource) override;
  virtual void CopyTextureRegionPlatform(const ezGALTexture* pDestination, const ezGALTextureSubresource& destinationSubResource, const ezVec3U32& vDestinationPoint, const ezGALTexture* pSource, const ezGALTextureSubresource& sourceSubResource, const ezBoundingBoxu32& box) override;
  virtual void UpdateTexturePlatform(const ezGALTexture* pDestination, const ezGALTextureSubresource& destinationSubResource, const ezBoundingBoxu32& destinationBox, const ezGALSystemMemoryDescription& sourceData) override;
  virtual void ResolveTexturePlatform(const ezGALTexture* pDestination, const ezGALTextureSubresource& destinationSubResource, const ezGALTexture* pSource, const ezGALTextureSubresource& sourceSubResource) override;
  virtual void ReadbackTexturePlatform(const ezGALReadbackTexture* pDestination, const ezGALTexture* pSource) override;
  virtual void ReadbackBufferPlatform(const ezGALReadbackBuffer* pDestination, const ezGALBuffer* pSource) override;
  virtual void GenerateMipMapsPlatform(const ezGALTexture* pTexture, ezGALTextureRange range) override;

  // Misc
  virtual void FlushPlatform() override;

  // Debug helper functions
  virtual void PushMarkerPlatform(const char* szMarker) override;
  virtual void PopMarkerPlatform() override;
  virtual void InsertEventMarkerPlatform(const char* szMarker) override;

  // Compute
  virtual void BeginComputePlatform() override;
  virtual void EndComputePlatform() override;
  virtual ezResult DispatchPlatform(ezUInt32 uiThreadGroupCountX, ezUInt32 uiThreadGroupCountY, ezUInt32 uiThreadGroupCountZ) override;
  virtual ezResult DispatchIndirectPlatform(const ezGALBuffer* pIndirectArgumentBuffer, ezUInt32 uiArgumentOffsetInBytes) override;

  // Rendering
  virtual void BeginRenderingPlatform(const ezGALRenderingSetup& renderingSetup) override;
  virtual void EndRenderingPlatform() override;
  virtual void ClearPlatform(const ezColor& clearColor, ezUInt32 uiRenderTargetClearMask, bool bClearDepth, bool bClearStencil, float fDepthClear, ezUInt8 uiStencilClear) override;
  virtual ezResult DrawPlatform(ezUInt32 uiVertexCount, ezUInt32 uiStartVertex) override;
  virtual ezResult DrawIndexedPlatform(ezUInt32 uiIndexCount, ezUInt32 uiStartIndex) override;
  virtual ezResult DrawIndexedInstancedPlatform(ezUInt32 uiIndexCountPerInstance, ezUInt32 uiInstanceCount, ezUInt32 uiStartIndex) override;
  virtual ezResult DrawIndexedInstancedIndirectPlatform(const ezGALBuffer* pIndirectArgumentBuffer, ezUInt32 uiArgumentOffsetInBytes) override;
  virtual ezResult DrawInstancedPlatform(ezUInt32 uiVertexCountPerInstance, ezUInt32 uiInstanceCount, ezUInt32 uiStartVertex) override;
  virtual ezResult DrawInstancedIndirectPlatform(const ezGALBuffer* pIndirectArgumentBuffer, ezUInt32 uiArgumentOffsetInBytes) override;

  // State
  virtual void SetIndexBufferPlatform(const ezGALBuffer* pIndexBuffer) override;
  virtual void SetVertexBufferPlatform(ezUInt32 uiSlot, const ezGALBuffer* pVertexBuffer, ezUInt32 uiOffset) override;
  virtual void SetGraphicsPipelinePlatform(const ezGALGraphicsPipeline* pGraphicsPipeline) override;
  virtual void SetComputePipelinePlatform(const ezGALComputePipeline* pComputePipeline) override;
  virtual void SetViewportPlatform(const ezRectFloat& rect, float fMinDepth, float fMaxDepth) override;
  virtual void SetScissorRectPlatform(const ezRectU32& rect) override;
  virtual void SetStencilReferencePlatform(ezUInt8 uiStencilRefValue) override;

private:
  void FlushDeferredStateChanges();
  void SetDescriptorHeaps();

  ezGALDeviceDX12& m_Device;
  ID3D12GraphicsCommandList* m_pCommandList = nullptr;
  ID3D12GraphicsCommandList* m_pClosedCommandList = nullptr;
  ezResourceBarrierDX12 m_BarrierTracker;

  // Rendering state
  bool m_bInsideRendering = false;
  bool m_bInsideCompute = false;
  ezGALRenderingSetup m_RenderTargetSetup;
  ezHybridArray<D3D12_CPU_DESCRIPTOR_HANDLE, 8> m_CurrentRTVs;
  D3D12_CPU_DESCRIPTOR_HANDLE m_CurrentDSV = {};
  bool m_bHasDSV = false;

  // Pipeline state
  const ezGALPipelineLayoutDX12* m_pCurrentPipelineLayout = nullptr;
  bool m_bGraphicsPipeline = false;
  D3D_PRIMITIVE_TOPOLOGY m_CurrentTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

  // Vertex/Index buffer state
  D3D12_INDEX_BUFFER_VIEW m_CurrentIndexBufferView = {};
  bool m_bIndexBufferDirty = false;
  D3D12_VERTEX_BUFFER_VIEW m_CurrentVertexBufferViews[EZ_GAL_MAX_VERTEX_BUFFER_COUNT] = {};
  bool m_bVertexBuffersDirty = false;
  bool m_bHasCommandListMarker = false;
};
