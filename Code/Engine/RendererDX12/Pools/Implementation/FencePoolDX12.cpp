#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/Pools/FencePoolDX12.h>

#include <Foundation/Threading/ThreadUtils.h>

// ---- ezFencePoolDX12 ----

ezFencePoolDX12::~ezFencePoolDX12()
{
  DeInitialize();
}

void ezFencePoolDX12::Initialize(ID3D12Device* pDevice)
{
  HRESULT hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
  EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create D3D12 fence: {}", ezArgErrorCode(hr));
  EZ_IGNORE_UNUSED(hr);

  m_hFenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  EZ_ASSERT_DEV(m_hFenceEvent != nullptr, "Failed to create fence event");

  m_uiNextFenceValue = 1;
}

void ezFencePoolDX12::DeInitialize()
{
  if (m_hFenceEvent)
  {
    CloseHandle(m_hFenceEvent);
    m_hFenceEvent = nullptr;
  }
  EZ_GAL_DX12_RELEASE(m_pFence);
  m_uiNextFenceValue = 1;
}

ezGALFenceHandle ezFencePoolDX12::InsertFence(ID3D12CommandQueue* pQueue)
{
  ezUInt64 uiFenceValue = m_uiNextFenceValue++;
  HRESULT hr = pQueue->Signal(m_pFence, uiFenceValue);
  EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to signal fence: {}", ezArgErrorCode(hr));
  EZ_IGNORE_UNUSED(hr);
  return uiFenceValue;
}

bool ezFencePoolDX12::IsFenceReached(ezGALFenceHandle hFence) const
{
  return m_pFence->GetCompletedValue() >= hFence;
}

ezEnum<ezGALAsyncResult> ezFencePoolDX12::WaitForFence(ezGALFenceHandle hFence, ezTime timeout)
{
  if (m_pFence->GetCompletedValue() >= hFence)
    return ezGALAsyncResult::Ready;

  HRESULT hr = m_pFence->SetEventOnCompletion(hFence, m_hFenceEvent);
  if (FAILED(hr))
    return ezGALAsyncResult::Expired;

  DWORD dwTimeout = timeout.IsZeroOrNegative() ? INFINITE : static_cast<DWORD>(timeout.GetMilliseconds());
  DWORD result = WaitForSingleObject(m_hFenceEvent, dwTimeout);

  if (result == WAIT_OBJECT_0)
    return ezGALAsyncResult::Ready;

  return ezGALAsyncResult::Pending;
}

ezUInt64 ezFencePoolDX12::GetLastCompletedFenceValue() const
{
  return m_pFence ? m_pFence->GetCompletedValue() : 0;
}


// ---- ezFenceQueueDX12 ----

ezFenceQueueDX12::ezFenceQueueDX12(ezAllocator* pAllocator)
{
  EZ_IGNORE_UNUSED(pAllocator);
}

ezFenceQueueDX12::~ezFenceQueueDX12()
{
  DeInitialize();
}

void ezFenceQueueDX12::Initialize(ID3D12Device* pDevice)
{
  m_FencePool.Initialize(pDevice);
  m_uiCurrentFenceCounter = 1;
  m_uiReachedFenceCounter = 0;
}

void ezFenceQueueDX12::DeInitialize()
{
  m_FencePool.DeInitialize();
}

ezGALFenceHandle ezFenceQueueDX12::GetCurrentFenceHandle()
{
  return m_uiCurrentFenceCounter;
}

ezGALFenceHandle ezFenceQueueDX12::SubmitCurrentFence(ID3D12CommandQueue* pQueue)
{
  // Flush any completed fences first
  m_uiReachedFenceCounter = m_FencePool.GetLastCompletedFenceValue();

  ezGALFenceHandle hCurrent = m_FencePool.InsertFence(pQueue);
  m_uiCurrentFenceCounter++;
  return hCurrent;
}

ezEnum<ezGALAsyncResult> ezFenceQueueDX12::GetFenceResult(ezGALFenceHandle hFence, ezTime timeout)
{
  if (hFence <= m_uiReachedFenceCounter)
    return ezGALAsyncResult::Ready;

  // Try to update the reached counter without blocking
  m_uiReachedFenceCounter = m_FencePool.GetLastCompletedFenceValue();
  if (hFence <= m_uiReachedFenceCounter)
    return ezGALAsyncResult::Ready;

  EZ_ASSERT_DEBUG(hFence < m_uiCurrentFenceCounter, "Invalid fence handle: fence has not been submitted yet");

  return m_FencePool.WaitForFence(hFence, timeout);
}
