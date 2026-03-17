#pragma once

#include <RendererDX12/RendererDX12DLL.h>

#include <d3d12.h>

/// Linear allocator for per-frame constant buffer data.
///
/// Uses a single large upload heap buffer as a ring buffer. Each frame resets the
/// offset to zero. All allocations are 256-byte aligned as required by D3D12 for
/// constant buffer views. The buffer is persistently mapped for the lifetime of the pool.
///
/// The pool size must be large enough to hold all constant buffer data for a single frame.
/// If the pool runs out of space, an assert fires in debug builds.
class EZ_RENDERERDX12_DLL ezUniformBufferPoolDX12
{
public:
  ezUniformBufferPoolDX12() = default;
  ~ezUniformBufferPoolDX12();

  void Initialize(ID3D12Device* pDevice, ezUInt64 uiPoolSize = 4 * 1024 * 1024);
  void DeInitialize();

  struct Allocation
  {
    ID3D12Resource* m_pBuffer = nullptr;
    ezUInt64 m_uiOffset = 0;
    D3D12_GPU_VIRTUAL_ADDRESS m_gpuAddress = 0;
    void* m_pMappedData = nullptr;
  };

  /// Allocates constant buffer memory. Returns the buffer, offset, GPU virtual address,
  /// and a CPU-writable pointer. The allocation is 256-byte aligned.
  Allocation Allocate(ezUInt64 uiSize);

  /// Resets the allocator for a new frame. All previous allocations become invalid.
  void BeginFrame();

private:
  ID3D12Resource* m_pBuffer = nullptr;
  void* m_pMappedData = nullptr;
  D3D12_GPU_VIRTUAL_ADDRESS m_gpuBaseAddress = 0;
  ezUInt64 m_uiPoolSize = 0;
  ezUInt64 m_uiCurrentOffset = 0;
};
