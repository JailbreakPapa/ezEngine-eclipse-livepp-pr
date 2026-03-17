#pragma once

#include <RendererDX12/RendererDX12DLL.h>

#include <Foundation/Containers/DynamicArray.h>

#include <d3d12.h>

/// Describes a staging buffer allocation for CPU-to-GPU data upload.
struct ezStagingBufferDX12
{
  ID3D12Resource* m_pBuffer = nullptr;
  void* m_pMappedData = nullptr;
  ezUInt64 m_uiSize = 0;
  ezUInt64 m_uiOffset = 0;
};

/// Pool for D3D12 upload heap buffers used for staging data transfers.
///
/// Allocates buffers from D3D12_HEAP_TYPE_UPLOAD. Buffers are persistently
/// mapped and can be written to directly by the CPU. After the GPU is done
/// with a frame, the buffers used in that frame can be reclaimed for reuse.
class EZ_RENDERERDX12_DLL ezStagingBufferPoolDX12
{
public:
  ezStagingBufferPoolDX12() = default;
  ~ezStagingBufferPoolDX12();

  void Initialize(ID3D12Device* pDevice);
  void DeInitialize();

  /// Allocates staging memory for upload. Returns a mapped pointer and staging buffer info.
  /// Tries to reuse a free buffer of sufficient size before creating a new one.
  ezStagingBufferDX12 AllocateBuffer(ezUInt64 uiSize, ezUInt64 uiAlignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

  /// Reclaims all staging buffers from completed frames (those with frame number <= uiCompletedFrame).
  void Reclaim(ezUInt64 uiCompletedFrame);

  /// Marks current in-flight allocations as belonging to the given frame.
  void FinishFrame(ezUInt64 uiFrame);

private:
  struct StagingBuffer
  {
    ID3D12Resource* m_pBuffer = nullptr;
    void* m_pMappedData = nullptr;
    ezUInt64 m_uiSize = 0;
    ezUInt64 m_uiFrame = 0;
  };

  ID3D12Device* m_pDevice = nullptr;
  ezDynamicArray<StagingBuffer> m_FreeBuffers;
  ezDynamicArray<StagingBuffer> m_UsedBuffers;
  ezDynamicArray<StagingBuffer> m_InFlightBuffers;
};
