#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/Pools/QueryPoolDX12.h>

ezQueryPoolDX12::~ezQueryPoolDX12()
{
  DeInitialize();
}

void ezQueryPoolDX12::Initialize(ID3D12Device* pDevice, ezUInt32 uiTimestampCount, ezUInt32 uiOcclusionCount)
{
  m_uiMaxTimestamps = uiTimestampCount;
  m_uiMaxOcclusions = uiOcclusionCount;

  // Create timestamp query heap
  {
    D3D12_QUERY_HEAP_DESC desc = {};
    desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    desc.Count = uiTimestampCount;

    HRESULT hr = pDevice->CreateQueryHeap(&desc, IID_PPV_ARGS(&m_pTimestampHeap));
    EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create timestamp query heap: {}", ezArgErrorCode(hr));
    EZ_IGNORE_UNUSED(hr);
  }

  // Create occlusion query heap
  {
    D3D12_QUERY_HEAP_DESC desc = {};
    desc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
    desc.Count = uiOcclusionCount;

    HRESULT hr = pDevice->CreateQueryHeap(&desc, IID_PPV_ARGS(&m_pOcclusionHeap));
    EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create occlusion query heap: {}", ezArgErrorCode(hr));
    EZ_IGNORE_UNUSED(hr);
  }

  // Create readback buffer for timestamps (each result is a ezUInt64)
  {
    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = uiTimestampCount * sizeof(ezUInt64);
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_READBACK;

    HRESULT hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
      &bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pTimestampReadbackBuffer));
    EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create timestamp readback buffer: {}", ezArgErrorCode(hr));
    EZ_IGNORE_UNUSED(hr);
  }

  // Create readback buffer for occlusion queries
  {
    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = uiOcclusionCount * sizeof(ezUInt64);
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_READBACK;

    HRESULT hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
      &bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pOcclusionReadbackBuffer));
    EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create occlusion readback buffer: {}", ezArgErrorCode(hr));
    EZ_IGNORE_UNUSED(hr);
  }
}

void ezQueryPoolDX12::DeInitialize()
{
  if (m_bTimestampsMapped && m_pTimestampReadbackBuffer)
  {
    m_pTimestampReadbackBuffer->Unmap(0, nullptr);
    m_bTimestampsMapped = false;
  }
  if (m_bOcclusionsMapped && m_pOcclusionReadbackBuffer)
  {
    m_pOcclusionReadbackBuffer->Unmap(0, nullptr);
    m_bOcclusionsMapped = false;
  }

  EZ_GAL_DX12_RELEASE(m_pTimestampHeap);
  EZ_GAL_DX12_RELEASE(m_pOcclusionHeap);
  EZ_GAL_DX12_RELEASE(m_pTimestampReadbackBuffer);
  EZ_GAL_DX12_RELEASE(m_pOcclusionReadbackBuffer);

  m_pTimestampReadbackData = nullptr;
  m_pOcclusionReadbackData = nullptr;
  m_uiMaxTimestamps = 0;
  m_uiMaxOcclusions = 0;
  m_uiNextTimestamp = 0;
  m_uiNextOcclusion = 0;
}

void ezQueryPoolDX12::InsertTimestamp(ID3D12GraphicsCommandList* pCommandList, ezUInt32& out_uiIndex)
{
  EZ_ASSERT_DEV(m_uiNextTimestamp < m_uiMaxTimestamps, "Timestamp query pool exhausted");

  out_uiIndex = m_uiNextTimestamp++;
  pCommandList->EndQuery(m_pTimestampHeap, D3D12_QUERY_TYPE_TIMESTAMP, out_uiIndex);
}

void ezQueryPoolDX12::BeginOcclusionQuery(ID3D12GraphicsCommandList* pCommandList, ezUInt32& out_uiIndex, bool bBinaryOcclusion)
{
  EZ_ASSERT_DEV(m_uiNextOcclusion < m_uiMaxOcclusions, "Occlusion query pool exhausted");

  out_uiIndex = m_uiNextOcclusion++;
  D3D12_QUERY_TYPE queryType = bBinaryOcclusion ? D3D12_QUERY_TYPE_BINARY_OCCLUSION : D3D12_QUERY_TYPE_OCCLUSION;
  pCommandList->BeginQuery(m_pOcclusionHeap, queryType, out_uiIndex);
}

void ezQueryPoolDX12::EndOcclusionQuery(ID3D12GraphicsCommandList* pCommandList, ezUInt32 uiIndex, bool bBinaryOcclusion)
{
  D3D12_QUERY_TYPE queryType = bBinaryOcclusion ? D3D12_QUERY_TYPE_BINARY_OCCLUSION : D3D12_QUERY_TYPE_OCCLUSION;
  pCommandList->EndQuery(m_pOcclusionHeap, queryType, uiIndex);
}

void ezQueryPoolDX12::ResolveQueries(ID3D12GraphicsCommandList* pCommandList)
{
  // Unmap before resolving new data
  if (m_bTimestampsMapped && m_pTimestampReadbackBuffer)
  {
    m_pTimestampReadbackBuffer->Unmap(0, nullptr);
    m_bTimestampsMapped = false;
    m_pTimestampReadbackData = nullptr;
  }
  if (m_bOcclusionsMapped && m_pOcclusionReadbackBuffer)
  {
    m_pOcclusionReadbackBuffer->Unmap(0, nullptr);
    m_bOcclusionsMapped = false;
    m_pOcclusionReadbackData = nullptr;
  }

  if (m_uiNextTimestamp > 0)
  {
    pCommandList->ResolveQueryData(m_pTimestampHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, m_uiNextTimestamp,
      m_pTimestampReadbackBuffer, 0);
  }

  if (m_uiNextOcclusion > 0)
  {
    pCommandList->ResolveQueryData(m_pOcclusionHeap, D3D12_QUERY_TYPE_OCCLUSION, 0, m_uiNextOcclusion,
      m_pOcclusionReadbackBuffer, 0);
  }
}

bool ezQueryPoolDX12::GetTimestampResult(ezUInt32 uiIndex, ezUInt64& out_uiResult) const
{
  out_uiResult = 0;
  if (uiIndex >= m_uiNextTimestamp)
    return false;

  if (!m_bTimestampsMapped)
  {
    D3D12_RANGE readRange = {0, m_uiNextTimestamp * sizeof(ezUInt64)};
    HRESULT hr = m_pTimestampReadbackBuffer->Map(0, &readRange, &m_pTimestampReadbackData);
    if (FAILED(hr))
      return false;
    m_bTimestampsMapped = true;
  }

  const ezUInt64* pData = static_cast<const ezUInt64*>(m_pTimestampReadbackData);
  out_uiResult = pData[uiIndex];
  return true;
}

bool ezQueryPoolDX12::GetOcclusionResult(ezUInt32 uiIndex, ezUInt64& out_uiResult) const
{
  out_uiResult = 0;
  if (uiIndex >= m_uiNextOcclusion)
    return false;

  if (!m_bOcclusionsMapped)
  {
    D3D12_RANGE readRange = {0, m_uiNextOcclusion * sizeof(ezUInt64)};
    HRESULT hr = m_pOcclusionReadbackBuffer->Map(0, &readRange, &m_pOcclusionReadbackData);
    if (FAILED(hr))
      return false;
    m_bOcclusionsMapped = true;
  }

  const ezUInt64* pData = static_cast<const ezUInt64*>(m_pOcclusionReadbackData);
  out_uiResult = pData[uiIndex];
  return true;
}

void ezQueryPoolDX12::BeginFrame()
{
  m_uiNextTimestamp = 0;
  m_uiNextOcclusion = 0;
}
