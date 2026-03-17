#pragma once

#include <RendererDX12/RendererDX12DLL.h>
#include <d3d12.h>

namespace D3D12MA
{
  class Allocator;
  class Allocation;
  struct ALLOCATION_DESC;
} // namespace D3D12MA

struct IDXGIAdapter;

struct ezDX12AllocationInfo
{
  D3D12_HEAP_TYPE m_HeapType = D3D12_HEAP_TYPE_DEFAULT;
  D3D12_HEAP_FLAGS m_ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;
};

class EZ_RENDERERDX12_DLL ezMemoryAllocatorDX12
{
public:
  static HRESULT Initialize(ID3D12Device* pDevice, IDXGIAdapter* pAdapter);
  static void DeInitialize();

  static HRESULT CreateBuffer(const D3D12_RESOURCE_DESC& resourceDesc, const ezDX12AllocationInfo& allocInfo,
    D3D12_RESOURCE_STATES initialState, ID3D12Resource** ppResource, D3D12MA::Allocation** ppAllocation);

  static HRESULT CreateTexture(const D3D12_RESOURCE_DESC& resourceDesc, const ezDX12AllocationInfo& allocInfo,
    D3D12_RESOURCE_STATES initialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue,
    ID3D12Resource** ppResource, D3D12MA::Allocation** ppAllocation);

  static void DestroyResource(ID3D12Resource*& pResource, D3D12MA::Allocation*& pAllocation);

private:
  static D3D12MA::Allocator* s_pAllocator;
};
