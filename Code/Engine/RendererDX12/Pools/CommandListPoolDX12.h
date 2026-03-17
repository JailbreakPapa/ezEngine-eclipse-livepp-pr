#pragma once

#include <RendererDX12/RendererDX12DLL.h>

#include <Foundation/Containers/HybridArray.h>

#include <d3d12.h>

/// Holds a paired command allocator and command list.
///
/// In D3D12, a command allocator manages the memory backing a command list.
/// Both must be recycled together after the GPU finishes executing the list.
struct ezCommandListDX12
{
  ID3D12CommandAllocator* m_pAllocator = nullptr;
  ID3D12GraphicsCommandList* m_pCommandList = nullptr;
};

/// Pool for reusing D3D12 command allocators and command lists.
///
/// After a command list is submitted and the GPU is done with it,
/// call ReclaimCommandList to return it to the pool. The next call to
/// RequestCommandList will reset and reuse it instead of creating a new one.
class EZ_RENDERERDX12_DLL ezCommandListPoolDX12
{
public:
  ezCommandListPoolDX12() = default;
  ~ezCommandListPoolDX12();

  void Initialize(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
  void DeInitialize();

  /// Returns a reset command list ready for recording.
  /// The returned command list is already in the recording state.
  ezCommandListDX12 RequestCommandList();

  /// Reclaims a command list after its associated fence has been reached.
  /// The command list must not be in use by the GPU.
  void ReclaimCommandList(ezCommandListDX12& ref_commandList);

private:
  ID3D12Device* m_pDevice = nullptr;
  D3D12_COMMAND_LIST_TYPE m_Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  ezHybridArray<ezCommandListDX12, 4> m_FreeCommandLists;
};
