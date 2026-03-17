#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/Pools/StagingBufferPoolDX12.h>

#include <Foundation/Math/Math.h>

ezStagingBufferPoolDX12::~ezStagingBufferPoolDX12()
{
  DeInitialize();
}

void ezStagingBufferPoolDX12::Initialize(ID3D12Device* pDevice)
{
  m_pDevice = pDevice;
}

void ezStagingBufferPoolDX12::DeInitialize()
{
  for (auto& buf : m_FreeBuffers)
  {
    if (buf.m_pBuffer)
    {
      buf.m_pBuffer->Unmap(0, nullptr);
      EZ_GAL_DX12_RELEASE(buf.m_pBuffer);
    }
  }
  for (auto& buf : m_InFlightBuffers)
  {
    if (buf.m_pBuffer)
    {
      buf.m_pBuffer->Unmap(0, nullptr);
      EZ_GAL_DX12_RELEASE(buf.m_pBuffer);
    }
  }
  for (auto& buf : m_UsedBuffers)
  {
    if (buf.m_pBuffer)
    {
      buf.m_pBuffer->Unmap(0, nullptr);
      EZ_GAL_DX12_RELEASE(buf.m_pBuffer);
    }
  }
  m_FreeBuffers.Clear();
  m_InFlightBuffers.Clear();
  m_UsedBuffers.Clear();
  m_pDevice = nullptr;
}

ezStagingBufferDX12 ezStagingBufferPoolDX12::AllocateBuffer(ezUInt64 uiSize, ezUInt64 uiAlignment)
{
  EZ_ASSERT_DEV(m_pDevice != nullptr, "Pool not initialized");

  ezUInt64 uiAlignedSize = (uiSize + (uiAlignment - 1)) & ~(uiAlignment - 1);

  // Try to find a free buffer of sufficient size
  for (ezUInt32 i = 0; i < m_FreeBuffers.GetCount(); ++i)
  {
    if (m_FreeBuffers[i].m_uiSize >= uiAlignedSize)
    {
      StagingBuffer buf = m_FreeBuffers[i];
      m_FreeBuffers.RemoveAtAndSwap(i);

      m_InFlightBuffers.PushBack(buf);

      ezStagingBufferDX12 result;
      result.m_pBuffer = buf.m_pBuffer;
      result.m_pMappedData = buf.m_pMappedData;
      result.m_uiSize = buf.m_uiSize;
      result.m_uiOffset = 0;
      return result;
    }
  }

  // No suitable free buffer found, create a new one
  D3D12_RESOURCE_DESC bufferDesc = {};
  bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  bufferDesc.Width = uiAlignedSize;
  bufferDesc.Height = 1;
  bufferDesc.DepthOrArraySize = 1;
  bufferDesc.MipLevels = 1;
  bufferDesc.SampleDesc.Count = 1;
  bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

  D3D12_HEAP_PROPERTIES heapProps = {};
  heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

  StagingBuffer buf;
  HRESULT hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
    &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buf.m_pBuffer));
  EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create staging buffer: {}", ezArgErrorCode(hr));
  EZ_IGNORE_UNUSED(hr);

  buf.m_uiSize = uiAlignedSize;

  // Persistently map the upload buffer
  D3D12_RANGE readRange = {0, 0}; // We do not read from this resource on CPU
  hr = buf.m_pBuffer->Map(0, &readRange, &buf.m_pMappedData);
  EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to map staging buffer: {}", ezArgErrorCode(hr));

  m_InFlightBuffers.PushBack(buf);

  ezStagingBufferDX12 result;
  result.m_pBuffer = buf.m_pBuffer;
  result.m_pMappedData = buf.m_pMappedData;
  result.m_uiSize = buf.m_uiSize;
  result.m_uiOffset = 0;
  return result;
}

void ezStagingBufferPoolDX12::Reclaim(ezUInt64 uiCompletedFrame)
{
  for (ezInt32 i = (ezInt32)m_UsedBuffers.GetCount() - 1; i >= 0; --i)
  {
    if (m_UsedBuffers[i].m_uiFrame <= uiCompletedFrame)
    {
      m_FreeBuffers.PushBack(m_UsedBuffers[i]);
      m_UsedBuffers.RemoveAtAndSwap(i);
    }
  }
}

void ezStagingBufferPoolDX12::FinishFrame(ezUInt64 uiFrame)
{
  for (auto& buf : m_InFlightBuffers)
  {
    buf.m_uiFrame = uiFrame;
    m_UsedBuffers.PushBack(buf);
  }
  m_InFlightBuffers.Clear();
}
