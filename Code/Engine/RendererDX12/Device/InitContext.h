#pragma once

#include <RendererDX12/RendererDX12DLL.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Threading/Mutex.h>
#include <d3d12.h>

class ezGALDeviceDX12;
class ezStagingBufferPoolDX12;

/// Thread-safe context for uploading initial resource data.
/// Used during buffer/texture creation to copy data from CPU to GPU memory.
class EZ_RENDERERDX12_DLL ezInitContextDX12
{
public:
  ezInitContextDX12() = default;
  ~ezInitContextDX12();

  void Initialize(ezGALDeviceDX12* pDevice);
  void DeInitialize();

  /// Uploads initial data to a buffer. Thread-safe.
  void UploadBufferData(ID3D12Resource* pDestBuffer, ezConstByteArrayPtr sourceData, D3D12_RESOURCE_STATES afterState);

  /// Uploads initial data to a texture. Thread-safe.
  void UploadTextureData(ID3D12Resource* pDestTexture, const ezArrayPtr<ezGALSystemMemoryDescription>& sourceData,
    const D3D12_RESOURCE_DESC& textureDesc, DXGI_FORMAT format);

  /// Submits all pending copy commands. Called from the device during BeginFrame.
  void SubmitPendingCommands();

private:
  void EnsureCommandListOpen();
  void ClosePendingCommandList();

  ezGALDeviceDX12* m_pDevice = nullptr;
  ID3D12Device* m_pDXDevice = nullptr;
  ID3D12CommandQueue* m_pCopyQueue = nullptr;
  ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
  ID3D12GraphicsCommandList* m_pCommandList = nullptr;
  ID3D12Fence* m_pFence = nullptr;
  HANDLE m_hFenceEvent = nullptr;
  ezUInt64 m_uiFenceValue = 0;

  bool m_bCommandListOpen = false;
  ezMutex m_Mutex;

  /// Upload buffers that are referenced by pending command lists.
  /// Released after the fence signals in SubmitPendingCommands.
  ezDynamicArray<ID3D12Resource*> m_PendingUploadBuffers;
};
