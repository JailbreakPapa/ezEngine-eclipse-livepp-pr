#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/Utils/ResourceBarrierDX12.h>

void ezResourceBarrierDX12::SetCommandList(ID3D12GraphicsCommandList* pCommandList)
{
  EZ_ASSERT_DEV(m_PendingBarriers.IsEmpty(), "Pending barriers must be flushed before changing command lists");
  m_pCommandList = pCommandList;
}

void ezResourceBarrierDX12::BufferBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
  if (stateBefore == stateAfter)
    return;

  D3D12_RESOURCE_BARRIER& barrier = m_PendingBarriers.ExpandAndGetRef();
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource = pResource;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  barrier.Transition.StateBefore = stateBefore;
  barrier.Transition.StateAfter = stateAfter;
}

void ezResourceBarrierDX12::TextureBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
  if (stateBefore == stateAfter)
    return;

  D3D12_RESOURCE_BARRIER& barrier = m_PendingBarriers.ExpandAndGetRef();
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource = pResource;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  barrier.Transition.StateBefore = stateBefore;
  barrier.Transition.StateAfter = stateAfter;
}

void ezResourceBarrierDX12::SubresourceBarrier(ID3D12Resource* pResource, ezUInt32 uiSubresource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
  if (stateBefore == stateAfter)
    return;

  D3D12_RESOURCE_BARRIER& barrier = m_PendingBarriers.ExpandAndGetRef();
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource = pResource;
  barrier.Transition.Subresource = uiSubresource;
  barrier.Transition.StateBefore = stateBefore;
  barrier.Transition.StateAfter = stateAfter;
}

void ezResourceBarrierDX12::UAVBarrier(ID3D12Resource* pResource)
{
  D3D12_RESOURCE_BARRIER& barrier = m_PendingBarriers.ExpandAndGetRef();
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.UAV.pResource = pResource;
}

void ezResourceBarrierDX12::AliasingBarrier(ID3D12Resource* pResourceBefore, ID3D12Resource* pResourceAfter)
{
  D3D12_RESOURCE_BARRIER& barrier = m_PendingBarriers.ExpandAndGetRef();
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Aliasing.pResourceBefore = pResourceBefore;
  barrier.Aliasing.pResourceAfter = pResourceAfter;
}

void ezResourceBarrierDX12::Flush()
{
  if (m_PendingBarriers.IsEmpty())
    return;

  EZ_ASSERT_DEV(m_pCommandList != nullptr, "Command list must be set before flushing barriers");
  m_pCommandList->ResourceBarrier(m_PendingBarriers.GetCount(), m_PendingBarriers.GetData());
  m_PendingBarriers.Clear();
}

void ezResourceBarrierDX12::Reset()
{
  m_PendingBarriers.Clear();
}
