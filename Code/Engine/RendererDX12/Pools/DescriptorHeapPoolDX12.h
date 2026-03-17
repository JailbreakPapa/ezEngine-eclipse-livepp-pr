#pragma once

#include <RendererDX12/RendererDX12DLL.h>

#include <Foundation/Types/TypeTraits.h>

#include <d3d12.h>

EZ_DEFINE_AS_POD_TYPE(D3D12_CPU_DESCRIPTOR_HANDLE);
EZ_DEFINE_AS_POD_TYPE(D3D12_GPU_DESCRIPTOR_HANDLE);

/// A CPU + GPU handle pair for a single descriptor.
struct ezDescriptorDX12
{
  D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle = {};
  D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle = {};
  ezUInt32 m_uiIndex = 0;
};

/// A contiguous range of descriptors in a descriptor heap.
struct ezDescriptorRangeDX12
{
  D3D12_CPU_DESCRIPTOR_HANDLE m_cpuStart = {};
  D3D12_GPU_DESCRIPTOR_HANDLE m_gpuStart = {};
  ezUInt32 m_uiStartIndex = 0;
  ezUInt32 m_uiCount = 0;
};

/// Manages all D3D12 descriptor heaps: shader-visible heaps for binding and
/// staging (CPU-only) heaps for creating descriptors before copying.
///
/// Shader-visible heaps use a linear allocator that resets each frame (transient).
/// Staging heaps are CPU-only and used to build descriptors that are then copied
/// into the shader-visible heap via CopyToShaderVisible.
///
/// D3D12 allows only one CBV/SRV/UAV heap and one sampler heap bound at a time,
/// so the shader-visible heaps are shared across all draw/dispatch calls.
class EZ_RENDERERDX12_DLL ezDescriptorHeapPoolDX12
{
public:
  ezDescriptorHeapPoolDX12() = default;
  ~ezDescriptorHeapPoolDX12();

  void Initialize(ID3D12Device* pDevice);
  void DeInitialize();

  // --- Shader-visible heap operations (for binding to pipeline) ---

  /// Allocates a contiguous range from the shader-visible CBV/SRV/UAV transient allocator.
  ezDescriptorRangeDX12 AllocateTransientSrvUavCbv(ezUInt32 uiCount);

  /// Allocates a contiguous range from the shader-visible sampler transient allocator.
  ezDescriptorRangeDX12 AllocateTransientSampler(ezUInt32 uiCount);

  /// Resets the transient allocators for a new frame.
  void BeginFrame();

  // --- Staging heap operations (CPU-only, for creating descriptors before copying) ---
  // Persistent allocations survive across frames (for resource SRVs, UAVs, RTVs, DSVs).
  // Transient staging allocations are reset each frame by BeginFrame().

  /// Allocates a persistent staging SRV/UAV/CBV descriptor (survives across frames).
  D3D12_CPU_DESCRIPTOR_HANDLE AllocateStagingSrvUavCbv();

  /// Allocates a persistent staging sampler descriptor (survives across frames).
  D3D12_CPU_DESCRIPTOR_HANDLE AllocateStagingSampler();

  /// Allocates a persistent staging RTV descriptor (survives across frames).
  D3D12_CPU_DESCRIPTOR_HANDLE AllocateStagingRTV();

  /// Allocates a persistent staging DSV descriptor (survives across frames).
  D3D12_CPU_DESCRIPTOR_HANDLE AllocateStagingDSV();

  /// Copies descriptors from staging handles into a shader-visible range.
  void CopyToShaderVisible(ezUInt32 uiCount, const D3D12_CPU_DESCRIPTOR_HANDLE* pSrcHandles,
    const ezDescriptorRangeDX12& destRange);

  // --- Getters ---

  ID3D12DescriptorHeap* GetSrvUavCbvHeap() const { return m_pSrvUavCbvHeap; }
  ID3D12DescriptorHeap* GetSamplerHeap() const { return m_pSamplerHeap; }

  ezUInt32 GetSrvUavCbvDescriptorSize() const { return m_uiSrvUavCbvDescriptorSize; }
  ezUInt32 GetSamplerDescriptorSize() const { return m_uiSamplerDescriptorSize; }
  ezUInt32 GetRTVDescriptorSize() const { return m_uiRTVDescriptorSize; }
  ezUInt32 GetDSVDescriptorSize() const { return m_uiDSVDescriptorSize; }

private:
  ID3D12Device* m_pDevice = nullptr;

  // Shader-visible heaps
  static constexpr ezUInt32 MAX_SRV_UAV_CBV = 1'000'000;
  static constexpr ezUInt32 MAX_SAMPLERS = 2048;

  ID3D12DescriptorHeap* m_pSrvUavCbvHeap = nullptr;
  ID3D12DescriptorHeap* m_pSamplerHeap = nullptr;
  ezUInt32 m_uiSrvUavCbvTransientStart = 0;
  ezUInt32 m_uiSamplerTransientStart = 0;

  // Staging (non-shader-visible) heaps
  static constexpr ezUInt32 STAGING_HEAP_SIZE = 4096;

  ID3D12DescriptorHeap* m_pStagingSrvUavCbvHeap = nullptr;
  ezUInt32 m_uiStagingSrvUavCbvNext = 0;

  ID3D12DescriptorHeap* m_pStagingSamplerHeap = nullptr;
  ezUInt32 m_uiStagingSamplerNext = 0;

  ID3D12DescriptorHeap* m_pStagingRTVHeap = nullptr;
  ezUInt32 m_uiStagingRTVNext = 0;

  ID3D12DescriptorHeap* m_pStagingDSVHeap = nullptr;
  ezUInt32 m_uiStagingDSVNext = 0;

  // Descriptor sizes (device-specific)
  ezUInt32 m_uiSrvUavCbvDescriptorSize = 0;
  ezUInt32 m_uiSamplerDescriptorSize = 0;
  ezUInt32 m_uiRTVDescriptorSize = 0;
  ezUInt32 m_uiDSVDescriptorSize = 0;
};
