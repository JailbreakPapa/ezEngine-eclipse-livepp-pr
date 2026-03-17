#pragma once

#include <RendererDX12/RendererDX12DLL.h>

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Types/TypeTraits.h>

#include <d3d12.h>

EZ_DEFINE_AS_POD_TYPE(D3D12_RESOURCE_BARRIER);

class EZ_RENDERERDX12_DLL ezResourceBarrierDX12
{
public:
  void SetCommandList(ID3D12GraphicsCommandList* pCommandList);

  /// Transitions a buffer to the given state. If the buffer is already in that state, no barrier is recorded.
  void BufferBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);

  /// Transitions a texture (all subresources) to the given state.
  void TextureBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);

  /// Transitions a single subresource of a texture.
  void SubresourceBarrier(ID3D12Resource* pResource, ezUInt32 uiSubresource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);

  /// Inserts a UAV barrier for the given resource (or all resources if nullptr).
  void UAVBarrier(ID3D12Resource* pResource = nullptr);

  /// Inserts an aliasing barrier.
  void AliasingBarrier(ID3D12Resource* pResourceBefore, ID3D12Resource* pResourceAfter);

  /// Submits all pending barriers to the command list.
  void Flush();

  /// Clears all pending state. Called when the command list is reset or closed.
  void Reset();

  bool HasPendingBarriers() const { return !m_PendingBarriers.IsEmpty(); }

private:
  ID3D12GraphicsCommandList* m_pCommandList = nullptr;
  ezHybridArray<D3D12_RESOURCE_BARRIER, 16> m_PendingBarriers;
};
