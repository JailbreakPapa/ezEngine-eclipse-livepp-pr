#pragma once

#include <RendererDX12/RendererDX12DLL.h>
#include <RendererFoundation/RendererFoundationDLL.h>

#include <d3d12.h>

/// Manages D3D12 query heaps for timestamps and occlusion queries.
///
/// Queries are written by the GPU into a query heap, then resolved into a readback
/// buffer so the CPU can read the results. Call ResolveQueries before reading results.
/// BeginFrame must be called at the start of each frame to reset the allocation counters.
class EZ_RENDERERDX12_DLL ezQueryPoolDX12
{
public:
  ezQueryPoolDX12() = default;
  ~ezQueryPoolDX12();

  void Initialize(ID3D12Device* pDevice, ezUInt32 uiTimestampCount = 1024, ezUInt32 uiOcclusionCount = 256);
  void DeInitialize();

  /// Inserts a timestamp query into the command list.
  void InsertTimestamp(ID3D12GraphicsCommandList* pCommandList, ezUInt32& out_uiIndex);

  /// Begins an occlusion query. If bBinaryOcclusion is true, uses BINARY_OCCLUSION which only
  /// reports whether any samples passed (cheaper than counting exact samples).
  void BeginOcclusionQuery(ID3D12GraphicsCommandList* pCommandList, ezUInt32& out_uiIndex, bool bBinaryOcclusion);

  /// Ends a previously started occlusion query.
  void EndOcclusionQuery(ID3D12GraphicsCommandList* pCommandList, ezUInt32 uiIndex, bool bBinaryOcclusion);

  /// Resolves all issued queries into the readback buffers. Must be called before reading results.
  void ResolveQueries(ID3D12GraphicsCommandList* pCommandList);

  /// Gets a timestamp result. Returns true if the result is available.
  bool GetTimestampResult(ezUInt32 uiIndex, ezUInt64& out_uiResult) const;

  /// Gets an occlusion query result. Returns true if available.
  bool GetOcclusionResult(ezUInt32 uiIndex, ezUInt64& out_uiResult) const;

  /// Resets pool state for a new frame.
  void BeginFrame();

  void SetGPUFrequency(ezUInt64 uiFrequency) { m_uiGPUFrequency = uiFrequency; }
  ezUInt64 GetGPUFrequency() const { return m_uiGPUFrequency; }

private:
  ID3D12QueryHeap* m_pTimestampHeap = nullptr;
  ID3D12QueryHeap* m_pOcclusionHeap = nullptr;
  ID3D12Resource* m_pTimestampReadbackBuffer = nullptr;
  ID3D12Resource* m_pOcclusionReadbackBuffer = nullptr;

  ezUInt32 m_uiMaxTimestamps = 0;
  ezUInt32 m_uiMaxOcclusions = 0;
  ezUInt32 m_uiNextTimestamp = 0;
  ezUInt32 m_uiNextOcclusion = 0;
  ezUInt64 m_uiGPUFrequency = 1;

  mutable void* m_pTimestampReadbackData = nullptr;
  mutable void* m_pOcclusionReadbackData = nullptr;
  mutable bool m_bTimestampsMapped = false;
  mutable bool m_bOcclusionsMapped = false;
};
