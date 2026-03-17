#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/Device/InitContext.h>
#include <RendererDX12/Device/DeviceDX12.h>

#include <Foundation/Profiling/Profiling.h>

EZ_DEFINE_AS_POD_TYPE(D3D12_PLACED_SUBRESOURCE_FOOTPRINT);

ezInitContextDX12::~ezInitContextDX12()
{
  DeInitialize();
}

void ezInitContextDX12::Initialize(ezGALDeviceDX12* pDevice)
{
  m_pDevice = pDevice;
  m_pDXDevice = pDevice->GetDXDevice();

  // Create a dedicated copy queue for async uploads
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
  queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

  HRESULT hr = m_pDXDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCopyQueue));
  EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create copy command queue");

  hr = m_pDXDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&m_pCommandAllocator));
  EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create copy command allocator");

  hr = m_pDXDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_pCommandAllocator, nullptr, IID_PPV_ARGS(&m_pCommandList));
  EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create copy command list");

  // Command list starts open, close it immediately since we open on demand
  m_pCommandList->Close();
  m_bCommandListOpen = false;

  hr = m_pDXDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
  EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create init context fence");

  m_hFenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  EZ_ASSERT_DEV(m_hFenceEvent != nullptr, "Failed to create fence event");
}

void ezInitContextDX12::DeInitialize()
{
  if (m_bCommandListOpen)
  {
    ClosePendingCommandList();
  }

  // Wait for all pending copies
  if (m_pFence && m_pCopyQueue && m_uiFenceValue > 0)
  {
    m_pCopyQueue->Signal(m_pFence, m_uiFenceValue);
    if (m_pFence->GetCompletedValue() < m_uiFenceValue)
    {
      m_pFence->SetEventOnCompletion(m_uiFenceValue, m_hFenceEvent);
      WaitForSingleObject(m_hFenceEvent, INFINITE);
    }
  }

  // Release any pending upload buffers.
  for (ID3D12Resource* pBuffer : m_PendingUploadBuffers)
  {
    pBuffer->Release();
  }
  m_PendingUploadBuffers.Clear();

  if (m_hFenceEvent)
  {
    CloseHandle(m_hFenceEvent);
    m_hFenceEvent = nullptr;
  }

  EZ_GAL_DX12_RELEASE(m_pCommandList);
  EZ_GAL_DX12_RELEASE(m_pCommandAllocator);
  EZ_GAL_DX12_RELEASE(m_pFence);
  EZ_GAL_DX12_RELEASE(m_pCopyQueue);

  m_pDevice = nullptr;
  m_pDXDevice = nullptr;
}

void ezInitContextDX12::EnsureCommandListOpen()
{
  if (!m_bCommandListOpen)
  {
    m_pCommandAllocator->Reset();
    m_pCommandList->Reset(m_pCommandAllocator, nullptr);
    m_bCommandListOpen = true;
  }
}

void ezInitContextDX12::ClosePendingCommandList()
{
  if (m_bCommandListOpen)
  {
    m_pCommandList->Close();
    m_bCommandListOpen = false;
  }
}

void ezInitContextDX12::UploadBufferData(ID3D12Resource* pDestBuffer, ezConstByteArrayPtr sourceData, D3D12_RESOURCE_STATES afterState)
{
  EZ_LOCK(m_Mutex);
  EZ_PROFILE_SCOPE("UploadBufferData");

  // Create an upload buffer
  D3D12_RESOURCE_DESC uploadDesc = {};
  uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  uploadDesc.Width = sourceData.GetCount();
  uploadDesc.Height = 1;
  uploadDesc.DepthOrArraySize = 1;
  uploadDesc.MipLevels = 1;
  uploadDesc.Format = DXGI_FORMAT_UNKNOWN;
  uploadDesc.SampleDesc = {1, 0};
  uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

  D3D12_HEAP_PROPERTIES uploadHeapProps = {};
  uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

  ID3D12Resource* pUploadBuffer = nullptr;
  HRESULT hr = m_pDXDevice->CreateCommittedResource(
    &uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadDesc,
    D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&pUploadBuffer));

  if (FAILED(hr))
  {
    EZ_REPORT_FAILURE("Failed to create upload buffer for init context");
    return;
  }

  // Map and copy data
  void* pMappedData = nullptr;
  D3D12_RANGE readRange = {0, 0};
  pUploadBuffer->Map(0, &readRange, &pMappedData);
  ezMemoryUtils::Copy(static_cast<ezUInt8*>(pMappedData), sourceData.GetPtr(), static_cast<size_t>(sourceData.GetCount()));
  pUploadBuffer->Unmap(0, nullptr);

  EnsureCommandListOpen();
  m_pCommandList->CopyBufferRegion(pDestBuffer, 0, pUploadBuffer, 0, sourceData.GetCount());

  // Keep upload buffer alive until the command list is submitted and the fence signals.
  m_PendingUploadBuffers.PushBack(pUploadBuffer);
}

void ezInitContextDX12::UploadTextureData(ID3D12Resource* pDestTexture, const ezArrayPtr<ezGALSystemMemoryDescription>& sourceData,
  const D3D12_RESOURCE_DESC& textureDesc, DXGI_FORMAT format)
{
  EZ_LOCK(m_Mutex);
  EZ_PROFILE_SCOPE("UploadTextureData");

  const UINT numSubresources = static_cast<UINT>(sourceData.GetCount());

  if (numSubresources == 0)
    return;

  // Get the required upload buffer size
  ezDynamicArray<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts;
  ezDynamicArray<UINT> numRows;
  ezDynamicArray<UINT64> rowSizesInBytes;
  layouts.SetCount(numSubresources);
  numRows.SetCount(numSubresources);
  rowSizesInBytes.SetCount(numSubresources);

  UINT64 uiUploadSize = 0;
  m_pDXDevice->GetCopyableFootprints(&textureDesc, 0, numSubresources, 0,
    layouts.GetData(), numRows.GetData(), rowSizesInBytes.GetData(), &uiUploadSize);

  // Create upload buffer
  D3D12_RESOURCE_DESC uploadDesc = {};
  uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  uploadDesc.Width = uiUploadSize;
  uploadDesc.Height = 1;
  uploadDesc.DepthOrArraySize = 1;
  uploadDesc.MipLevels = 1;
  uploadDesc.Format = DXGI_FORMAT_UNKNOWN;
  uploadDesc.SampleDesc = {1, 0};
  uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

  D3D12_HEAP_PROPERTIES uploadHeapProps = {};
  uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

  ID3D12Resource* pUploadBuffer = nullptr;
  HRESULT hr = m_pDXDevice->CreateCommittedResource(
    &uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadDesc,
    D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&pUploadBuffer));

  if (FAILED(hr))
  {
    EZ_REPORT_FAILURE("Failed to create upload buffer for texture init");
    return;
  }

  // Map and copy each subresource
  void* pMappedData = nullptr;
  D3D12_RANGE readRange = {0, 0};
  pUploadBuffer->Map(0, &readRange, &pMappedData);

  for (UINT i = 0; i < numSubresources; ++i)
  {
    const auto& layout = layouts[i];
    const auto& src = sourceData[i];

    ezUInt8* pDst = static_cast<ezUInt8*>(pMappedData) + layout.Offset;
    const ezUInt8* pSrc = src.m_pData.GetPtr();

    for (UINT row = 0; row < numRows[i]; ++row)
    {
      ezMemoryUtils::Copy(pDst + row * layout.Footprint.RowPitch,
        pSrc + row * src.m_uiRowPitch,
        static_cast<size_t>(rowSizesInBytes[i]));
    }
  }

  pUploadBuffer->Unmap(0, nullptr);

  // Record copy commands
  EnsureCommandListOpen();
  for (UINT i = 0; i < numSubresources; ++i)
  {
    D3D12_TEXTURE_COPY_LOCATION dst = {};
    dst.pResource = pDestTexture;
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex = i;

    D3D12_TEXTURE_COPY_LOCATION src = {};
    src.pResource = pUploadBuffer;
    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src.PlacedFootprint = layouts[i];

    m_pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
  }

  // Keep upload buffer alive until the command list is submitted and the fence signals.
  m_PendingUploadBuffers.PushBack(pUploadBuffer);
}

void ezInitContextDX12::SubmitPendingCommands()
{
  EZ_LOCK(m_Mutex);

  if (!m_bCommandListOpen)
    return;

  ClosePendingCommandList();

  ID3D12CommandList* ppCommandLists[] = {m_pCommandList};
  m_pCopyQueue->ExecuteCommandLists(1, ppCommandLists);

  m_uiFenceValue++;
  m_pCopyQueue->Signal(m_pFence, m_uiFenceValue);

  // Wait for copy completion
  if (m_pFence->GetCompletedValue() < m_uiFenceValue)
  {
    m_pFence->SetEventOnCompletion(m_uiFenceValue, m_hFenceEvent);
    WaitForSingleObject(m_hFenceEvent, INFINITE);
  }

  // Now that the GPU is done, release all upload buffers.
  for (ID3D12Resource* pBuffer : m_PendingUploadBuffers)
  {
    pBuffer->Release();
  }
  m_PendingUploadBuffers.Clear();
}
