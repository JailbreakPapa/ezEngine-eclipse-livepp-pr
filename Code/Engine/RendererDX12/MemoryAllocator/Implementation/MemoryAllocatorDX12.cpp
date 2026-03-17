#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/MemoryAllocator/MemoryAllocatorDX12.h>

#include <Foundation/Logging/Log.h>

#include <D3D12MemAlloc.h>

D3D12MA::Allocator* ezMemoryAllocatorDX12::s_pAllocator = nullptr;

HRESULT ezMemoryAllocatorDX12::Initialize(ID3D12Device* pDevice, IDXGIAdapter* pAdapter)
{
  D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
  allocatorDesc.pDevice = pDevice;
  allocatorDesc.pAdapter = pAdapter;

  return D3D12MA::CreateAllocator(&allocatorDesc, &s_pAllocator);
}

void ezMemoryAllocatorDX12::DeInitialize()
{
  if (s_pAllocator)
  {
    s_pAllocator->Release();
    s_pAllocator = nullptr;
  }
}

HRESULT ezMemoryAllocatorDX12::CreateBuffer(const D3D12_RESOURCE_DESC& resourceDesc, const ezDX12AllocationInfo& allocInfo,
  D3D12_RESOURCE_STATES initialState, ID3D12Resource** ppResource, D3D12MA::Allocation** ppAllocation)
{
  EZ_ASSERT_DEV(s_pAllocator != nullptr, "Memory allocator not initialized");

  D3D12MA::ALLOCATION_DESC allocDesc = {};
  allocDesc.HeapType = allocInfo.m_HeapType;

  return s_pAllocator->CreateResource(&allocDesc, &resourceDesc, initialState, nullptr, ppAllocation, IID_PPV_ARGS(ppResource));
}

HRESULT ezMemoryAllocatorDX12::CreateTexture(const D3D12_RESOURCE_DESC& resourceDesc, const ezDX12AllocationInfo& allocInfo,
  D3D12_RESOURCE_STATES initialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue,
  ID3D12Resource** ppResource, D3D12MA::Allocation** ppAllocation)
{
  EZ_ASSERT_DEV(s_pAllocator != nullptr, "Memory allocator not initialized");

  D3D12MA::ALLOCATION_DESC allocDesc = {};
  allocDesc.HeapType = allocInfo.m_HeapType;

  return s_pAllocator->CreateResource(&allocDesc, &resourceDesc, initialState, pOptimizedClearValue, ppAllocation, IID_PPV_ARGS(ppResource));
}

void ezMemoryAllocatorDX12::DestroyResource(ID3D12Resource*& pResource, D3D12MA::Allocation*& pAllocation)
{
  if (pResource)
  {
    pResource->Release();
    pResource = nullptr;
  }
  if (pAllocation)
  {
    pAllocation->Release();
    pAllocation = nullptr;
  }
}
