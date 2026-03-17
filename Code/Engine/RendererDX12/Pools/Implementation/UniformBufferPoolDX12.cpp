#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/Pools/UniformBufferPoolDX12.h>

#include <Foundation/Math/Math.h>

/// D3D12 requires constant buffer offsets to be 256-byte aligned.
static constexpr ezUInt64 CB_ALIGNMENT = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

ezUniformBufferPoolDX12::~ezUniformBufferPoolDX12()
{
  DeInitialize();
}

void ezUniformBufferPoolDX12::Initialize(ID3D12Device* pDevice, ezUInt64 uiPoolSize)
{
  m_uiPoolSize = uiPoolSize;

  D3D12_RESOURCE_DESC bufferDesc = {};
  bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  bufferDesc.Width = uiPoolSize;
  bufferDesc.Height = 1;
  bufferDesc.DepthOrArraySize = 1;
  bufferDesc.MipLevels = 1;
  bufferDesc.SampleDesc.Count = 1;
  bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

  D3D12_HEAP_PROPERTIES heapProps = {};
  heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

  HRESULT hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
    &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pBuffer));
  EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to create uniform buffer pool: {}", ezArgErrorCode(hr));
  EZ_IGNORE_UNUSED(hr);

  m_gpuBaseAddress = m_pBuffer->GetGPUVirtualAddress();

  // Persistently map the buffer
  D3D12_RANGE readRange = {0, 0};
  hr = m_pBuffer->Map(0, &readRange, &m_pMappedData);
  EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to map uniform buffer pool: {}", ezArgErrorCode(hr));

  m_uiCurrentOffset = 0;
}

void ezUniformBufferPoolDX12::DeInitialize()
{
  if (m_pBuffer)
  {
    m_pBuffer->Unmap(0, nullptr);
    m_pMappedData = nullptr;
  }
  EZ_GAL_DX12_RELEASE(m_pBuffer);
  m_gpuBaseAddress = 0;
  m_uiPoolSize = 0;
  m_uiCurrentOffset = 0;
}

ezUniformBufferPoolDX12::Allocation ezUniformBufferPoolDX12::Allocate(ezUInt64 uiSize)
{
  ezUInt64 uiAlignedOffset = (m_uiCurrentOffset + (CB_ALIGNMENT - 1)) & ~(CB_ALIGNMENT - 1);
  ezUInt64 uiAlignedSize = (uiSize + (CB_ALIGNMENT - 1)) & ~(CB_ALIGNMENT - 1);

  EZ_ASSERT_DEV(uiAlignedOffset + uiAlignedSize <= m_uiPoolSize,
    "Uniform buffer pool exhausted. Current offset: {}, requested size: {}, pool size: {}",
    uiAlignedOffset, uiAlignedSize, m_uiPoolSize);

  Allocation alloc;
  alloc.m_pBuffer = m_pBuffer;
  alloc.m_uiOffset = uiAlignedOffset;
  alloc.m_gpuAddress = m_gpuBaseAddress + uiAlignedOffset;
  alloc.m_pMappedData = static_cast<ezUInt8*>(m_pMappedData) + uiAlignedOffset;

  m_uiCurrentOffset = uiAlignedOffset + uiAlignedSize;
  return alloc;
}

void ezUniformBufferPoolDX12::BeginFrame()
{
  m_uiCurrentOffset = 0;
}
