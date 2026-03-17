#pragma once

#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/UniquePtr.h>
#include <RendererDX12/RendererDX12DLL.h>
#include <RendererFoundation/Device/Device.h>

#include <Foundation/Platform/Win/Utils/IncludeWindows.h>
#include <dxgi1_6.h>

struct ID3D12Device;
struct ID3D12Device5;
struct ID3D12CommandQueue;
struct ID3D12Debug;
struct ID3D12CommandSignature;
struct ID3D12InfoQueue;
struct IDXGIFactory6;
struct IDXGIAdapter1;

class ezGALCommandEncoderImplDX12;
class ezFenceQueueDX12;
class ezQueryPoolDX12;
class ezCommandListPoolDX12;
class ezDescriptorHeapPoolDX12;
class ezStagingBufferPoolDX12;
class ezUniformBufferPoolDX12;
class ezInitContextDX12;

using ezGALFormatLookupEntryDX12 = ezGALFormatLookupEntry<DXGI_FORMAT, (DXGI_FORMAT)0>;
using ezGALFormatLookupTableDX12 = ezGALFormatLookupTable<ezGALFormatLookupEntryDX12>;

/// The D3D12 device implementation of the graphics abstraction layer.
///
/// Manages the D3D12 device, command queue, and all subsystem pools (fences, command lists,
/// descriptor heaps, staging buffers, uniform buffers). Resource creation and destruction
/// follows the same pattern as the DX11 backend: allocate a DX12 subclass, call InitPlatform,
/// and return the base pointer. Frame lifecycle uses a ring buffer of fence values to track
/// GPU completion and reclaim per-frame resources.
class EZ_RENDERERDX12_DLL ezGALDeviceDX12 : public ezGALDevice
{
private:
  friend ezInternal::NewInstance<ezGALDevice> CreateDX12Device(ezAllocator* pAllocator, const ezGALDeviceCreationDescription& description);
  ezGALDeviceDX12(const ezGALDeviceCreationDescription& Description);

public:
  virtual ~ezGALDeviceDX12();

public:
  ID3D12Device5* GetDXDevice() const;
  ID3D12CommandQueue* GetGraphicsQueue() const;
  IDXGIFactory6* GetDXGIFactory() const;
  ezGALCommandEncoder* GetCommandEncoder() const;

  ezFenceQueueDX12& GetFenceQueue() const;
  ezQueryPoolDX12& GetQueryPool() const;
  ezCommandListPoolDX12& GetCommandListPool() const;
  ezDescriptorHeapPoolDX12& GetDescriptorHeapPool() const;
  ezStagingBufferPoolDX12& GetStagingBufferPool() const;
  ezUniformBufferPoolDX12& GetUniformBufferPool() const;
  ezInitContextDX12& GetInitContext() const;

  ID3D12CommandSignature* GetDrawIndirectSignature() const { return m_pDrawIndirectSignature; }
  ID3D12CommandSignature* GetDrawIndexedIndirectSignature() const { return m_pDrawIndexedIndirectSignature; }
  ID3D12CommandSignature* GetDispatchIndirectSignature() const { return m_pDispatchIndirectSignature; }

  const ezGALFormatLookupTableDX12& GetFormatLookupTable() const;

  void ReportLiveGpuObjects();
  void FlushDeadObjects();

protected:
  virtual ezStringView GetRendererPlatform() override;
  virtual ezResult InitPlatform() override;
  virtual ezResult ShutdownPlatform() override;

  virtual ezGALCommandEncoder* BeginCommandsPlatform(const char* szName) override;
  virtual void EndCommandsPlatform(ezGALCommandEncoder* pPass) override;

  virtual void FlushPlatform() override;

  // State creation methods

  virtual ezGALBlendState* CreateBlendStatePlatform(const ezGALBlendStateCreationDescription& Description) override;
  virtual void DestroyBlendStatePlatform(ezGALBlendState* pBlendState) override;

  virtual ezGALDepthStencilState* CreateDepthStencilStatePlatform(const ezGALDepthStencilStateCreationDescription& Description) override;
  virtual void DestroyDepthStencilStatePlatform(ezGALDepthStencilState* pDepthStencilState) override;

  virtual ezGALRasterizerState* CreateRasterizerStatePlatform(const ezGALRasterizerStateCreationDescription& Description) override;
  virtual void DestroyRasterizerStatePlatform(ezGALRasterizerState* pRasterizerState) override;

  virtual ezGALSamplerState* CreateSamplerStatePlatform(const ezGALSamplerStateCreationDescription& Description) override;
  virtual void DestroySamplerStatePlatform(ezGALSamplerState* pSamplerState) override;

  virtual ezGALBindGroupLayout* CreateBindGroupLayoutPlatform(const ezGALBindGroupLayoutCreationDescription& Description) override;
  virtual void DestroyBindGroupLayoutPlatform(ezGALBindGroupLayout* pBindGroupLayout) override;

  virtual ezGALBindGroup* CreateBindGroupPlatform(const ezGALBindGroupCreationDescription& Description) override;
  virtual void DestroyBindGroupPlatform(ezGALBindGroup* pBindGroup) override;

  virtual ezGALPipelineLayout* CreatePipelineLayoutPlatform(const ezGALPipelineLayoutCreationDescription& Description) override;
  virtual void DestroyPipelineLayoutPlatform(ezGALPipelineLayout* pPipelineLayout) override;

  virtual ezGALGraphicsPipeline* CreateGraphicsPipelinePlatform(const ezGALGraphicsPipelineCreationDescription& Description) override;
  virtual void DestroyGraphicsPipelinePlatform(ezGALGraphicsPipeline* pGraphicsPipeline) override;

  virtual ezGALComputePipeline* CreateComputePipelinePlatform(const ezGALComputePipelineCreationDescription& Description) override;
  virtual void DestroyComputePipelinePlatform(ezGALComputePipeline* pComputePipeline) override;

  // Resource creation

  virtual ezGALShader* CreateShaderPlatform(const ezGALShaderCreationDescription& Description) override;
  virtual void DestroyShaderPlatform(ezGALShader* pShader) override;

  virtual ezGALBuffer* CreateBufferPlatform(const ezGALBufferCreationDescription& Description, ezArrayPtr<const ezUInt8> pInitialData) override;
  virtual void DestroyBufferPlatform(ezGALBuffer* pBuffer) override;

  virtual ezGALTexture* CreateTexturePlatform(const ezGALTextureCreationDescription& Description, ezArrayPtr<ezGALSystemMemoryDescription> pInitialData) override;
  virtual void DestroyTexturePlatform(ezGALTexture* pTexture) override;

  virtual ezGALTexture* CreateSharedTexturePlatform(const ezGALTextureCreationDescription& Description, ezArrayPtr<ezGALSystemMemoryDescription> pInitialData, ezEnum<ezGALSharedTextureType> sharedType, ezGALPlatformSharedHandle handle) override;
  virtual void DestroySharedTexturePlatform(ezGALTexture* pTexture) override;

  virtual ezGALReadbackBuffer* CreateReadbackBufferPlatform(const ezGALBufferCreationDescription& Description) override;
  virtual void DestroyReadbackBufferPlatform(ezGALReadbackBuffer* pReadbackBuffer) override;

  virtual ezGALReadbackTexture* CreateReadbackTexturePlatform(const ezGALTextureCreationDescription& Description) override;
  virtual void DestroyReadbackTexturePlatform(ezGALReadbackTexture* pReadbackTexture) override;

  virtual ezGALRenderTargetView* CreateRenderTargetViewPlatform(ezGALTexture* pTexture, const ezGALRenderTargetViewCreationDescription& Description) override;
  virtual void DestroyRenderTargetViewPlatform(ezGALRenderTargetView* pRenderTargetView) override;

  virtual ezGALVertexDeclaration* CreateVertexDeclarationPlatform(const ezGALVertexDeclarationCreationDescription& Description) override;
  virtual void DestroyVertexDeclarationPlatform(ezGALVertexDeclaration* pVertexDeclaration) override;

  // Resource update

  virtual void UpdateBufferForNextFramePlatform(const ezGALBuffer* pBuffer, ezConstByteArrayPtr sourceData, ezUInt32 uiDestOffset) override;
  virtual void UpdateTextureForNextFramePlatform(const ezGALTexture* pTexture, const ezGALSystemMemoryDescription& sourceData, const ezGALTextureSubresource& destinationSubResource, const ezBoundingBoxu32& destinationBox) override;

  // GPU -> CPU queries

  virtual ezEnum<ezGALAsyncResult> GetTimestampResultPlatform(ezGALTimestampHandle hTimestamp, ezTime& out_result) override;
  virtual ezEnum<ezGALAsyncResult> GetOcclusionResultPlatform(ezGALOcclusionHandle hOcclusion, ezUInt64& out_uiResult) override;
  virtual ezEnum<ezGALAsyncResult> GetFenceResultPlatform(ezGALFenceHandle hFence, ezTime timeout) override;
  virtual ezResult LockBufferPlatform(const ezGALReadbackBuffer* pBuffer, ezArrayPtr<const ezUInt8>& out_Memory) const override;
  virtual void UnlockBufferPlatform(const ezGALReadbackBuffer* pBuffer) const override;
  virtual ezResult LockTexturePlatform(const ezGALReadbackTexture* pTexture, const ezArrayPtr<const ezGALTextureSubresource>& subResources, ezDynamicArray<ezGALSystemMemoryDescription>& out_Memory) const override;
  virtual void UnlockTexturePlatform(const ezGALReadbackTexture* pTexture, const ezArrayPtr<const ezGALTextureSubresource>& subResources) const override;

  // Misc

  virtual void BeginFramePlatform(ezArrayPtr<ezGALSwapChain*> swapchains, const ezUInt64 uiAppFrame) override;
  virtual void EndFramePlatform(ezArrayPtr<ezGALSwapChain*> swapchains) override;
  virtual ezUInt64 GetCurrentFramePlatform() const override;
  virtual ezUInt64 GetSafeFramePlatform() const override;
  virtual void FillCapabilitiesPlatform() override;
  virtual void WaitIdlePlatform() override;
  virtual const ezGALSharedTexture* GetSharedTexture(ezGALTextureHandle hTexture) const override;

private:
  friend class ezGALCommandEncoderImplDX12;

  void FillFormatLookupTable();

  static constexpr ezUInt32 FRAMES = 4;

  ID3D12Device5* m_pDevice = nullptr;
  ID3D12CommandQueue* m_pGraphicsQueue = nullptr;
  ID3D12Debug* m_pDebug = nullptr;
  IDXGIFactory6* m_pDXGIFactory = nullptr;
  IDXGIAdapter1* m_pDXGIAdapter = nullptr;

  ezUniquePtr<ezFenceQueueDX12> m_pFenceQueue;
  ezUniquePtr<ezQueryPoolDX12> m_pQueryPool;
  ezUniquePtr<ezCommandListPoolDX12> m_pCommandListPool;
  ezUniquePtr<ezDescriptorHeapPoolDX12> m_pDescriptorHeapPool;
  ezUniquePtr<ezStagingBufferPoolDX12> m_pStagingBufferPool;
  ezUniquePtr<ezUniformBufferPoolDX12> m_pUniformBufferPool;
  ezUniquePtr<ezInitContextDX12> m_pInitContext;

  ID3D12CommandSignature* m_pDrawIndirectSignature = nullptr;
  ID3D12CommandSignature* m_pDrawIndexedIndirectSignature = nullptr;
  ID3D12CommandSignature* m_pDispatchIndirectSignature = nullptr;

  ezGALFormatLookupTableDX12 m_FormatLookupTable;

  ezUniquePtr<ezGALCommandEncoderImplDX12> m_pCommandEncoderImpl;
  ezUniquePtr<ezGALCommandEncoder> m_pCommandEncoder;

  struct PerFrameData
  {
    ezGALFenceHandle m_hFence = {};
    ezUInt64 m_uiFrame = ezUInt64(-1);
  };

  PerFrameData m_PerFrameData[FRAMES];
  ezUInt64 m_uiFrameCounter = 1;
  ezUInt64 m_uiSafeFrame = 0;
  ezUInt8 m_uiCurrentPerFrameData = m_uiFrameCounter % FRAMES;

  struct PendingBufferUpdate
  {
    const ezGALBuffer* m_pBuffer = nullptr;
    ezDynamicArray<ezUInt8> m_Data;
    ezUInt32 m_uiDestOffset = 0;
  };

  struct PendingTextureUpdate
  {
    const ezGALTexture* m_pTexture = nullptr;
    ezGALSystemMemoryDescription m_SourceData;
    ezGALTextureSubresource m_DestSubResource;
    ezBoundingBoxu32 m_DestBox;
    ezDynamicArray<ezUInt8> m_DataCopy;
  };

  ezDynamicArray<PendingBufferUpdate, ezLocalAllocatorWrapper> m_PendingBufferUpdates;
  ezDynamicArray<PendingTextureUpdate, ezLocalAllocatorWrapper> m_PendingTextureUpdates;

  struct GPUTimingScope* m_pFrameTimingScope = nullptr;
  struct GPUTimingScope* m_pPassTimingScope = nullptr;
};

#include <RendererDX12/Device/Implementation/DeviceDX12_inl.h>
