#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/Pools/CommandListPoolDX12.h>

ezCommandListPoolDX12::~ezCommandListPoolDX12()
{
  DeInitialize();
}

void ezCommandListPoolDX12::Initialize(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type)
{
  m_pDevice = pDevice;
  m_Type = type;
}

void ezCommandListPoolDX12::DeInitialize()
{
  for (auto& cl : m_FreeCommandLists)
  {
    EZ_GAL_DX12_RELEASE(cl.m_pCommandList);
    EZ_GAL_DX12_RELEASE(cl.m_pAllocator);
  }
  m_FreeCommandLists.Clear();
  m_pDevice = nullptr;
}

ezCommandListDX12 ezCommandListPoolDX12::RequestCommandList()
{
  EZ_ASSERT_DEV(m_pDevice != nullptr, "Pool not initialized");

  if (!m_FreeCommandLists.IsEmpty())
  {
    ezCommandListDX12 cl = m_FreeCommandLists.PeekBack();
    m_FreeCommandLists.PopBack();

    HRESULT hr = cl.m_pAllocator->Reset();
    EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to reset command allocator: {}", ezArgErrorCode(hr));

    hr = cl.m_pCommandList->Reset(cl.m_pAllocator, nullptr);
    EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to reset command list: {}", ezArgErrorCode(hr));
    EZ_IGNORE_UNUSED(hr);

    return cl;
  }

  ezCommandListDX12 cl;
  HRESULT hr = m_pDevice->CreateCommandAllocator(m_Type, IID_PPV_ARGS(&cl.m_pAllocator));
  EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create command allocator: {}", ezArgErrorCode(hr));

  hr = m_pDevice->CreateCommandList(0, m_Type, cl.m_pAllocator, nullptr, IID_PPV_ARGS(&cl.m_pCommandList));
  EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create command list: {}", ezArgErrorCode(hr));
  EZ_IGNORE_UNUSED(hr);

  return cl;
}

void ezCommandListPoolDX12::ReclaimCommandList(ezCommandListDX12& ref_commandList)
{
  if (ref_commandList.m_pCommandList != nullptr)
  {
    m_FreeCommandLists.PushBack(ref_commandList);
  }
  ref_commandList = {};
}
