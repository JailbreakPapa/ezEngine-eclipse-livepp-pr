#pragma once

#include <RendererDX12/RendererDX12DLL.h>
#include <RendererFoundation/RendererFoundationDLL.h>

#include <Foundation/Containers/Deque.h>

#include <d3d12.h>

/// Manages a D3D12 fence and event for GPU/CPU synchronization.
///
/// In D3D12, fences are the primary synchronization primitive.
/// This class wraps a single ID3D12Fence and provides an incrementing
/// fence value that is signaled on a command queue after each submission.
class EZ_RENDERERDX12_DLL ezFencePoolDX12
{
public:
  ezFencePoolDX12() = default;
  ~ezFencePoolDX12();

  void Initialize(ID3D12Device* pDevice);
  void DeInitialize();

  /// Signals the fence on the given queue and returns the fence value.
  ezGALFenceHandle InsertFence(ID3D12CommandQueue* pQueue);

  /// Returns true if the fence value has been reached by the GPU.
  bool IsFenceReached(ezGALFenceHandle hFence) const;

  /// Blocks the CPU until the fence is reached, or the timeout expires.
  ezEnum<ezGALAsyncResult> WaitForFence(ezGALFenceHandle hFence, ezTime timeout = ezTime::MakeZero());

  /// Returns the last completed fence value.
  ezUInt64 GetLastCompletedFenceValue() const;

  /// Returns the next fence value that will be used.
  ezUInt64 GetNextFenceValue() const { return m_uiNextFenceValue; }

private:
  ID3D12Fence* m_pFence = nullptr;
  HANDLE m_hFenceEvent = nullptr;
  ezUInt64 m_uiNextFenceValue = 1;
};


/// Higher-level fence queue that tracks pending fences, matching the pattern of ezFenceQueueDX11.
///
/// Wraps ezFencePoolDX12 and provides fence handle management with automatic
/// flushing of completed fences.
class EZ_RENDERERDX12_DLL ezFenceQueueDX12
{
public:
  ezFenceQueueDX12(ezAllocator* pAllocator);
  ~ezFenceQueueDX12();

  void Initialize(ID3D12Device* pDevice);
  void DeInitialize();

  /// Returns the current (not yet submitted) fence handle.
  ezGALFenceHandle GetCurrentFenceHandle();

  /// Signals the fence on the queue and advances the fence counter.
  ezGALFenceHandle SubmitCurrentFence(ID3D12CommandQueue* pQueue);

  /// Checks whether a fence has been reached by the GPU.
  ezEnum<ezGALAsyncResult> GetFenceResult(ezGALFenceHandle hFence, ezTime timeout = ezTime::MakeZero());

private:
  ezFencePoolDX12 m_FencePool;
  ezUInt64 m_uiCurrentFenceCounter = 1;
  ezUInt64 m_uiReachedFenceCounter = 0;
};
