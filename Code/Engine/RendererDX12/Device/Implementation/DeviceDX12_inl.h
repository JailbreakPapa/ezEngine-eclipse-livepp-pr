#pragma once

EZ_ALWAYS_INLINE ID3D12Device5* ezGALDeviceDX12::GetDXDevice() const
{
  return m_pDevice;
}

EZ_ALWAYS_INLINE ID3D12CommandQueue* ezGALDeviceDX12::GetGraphicsQueue() const
{
  return m_pGraphicsQueue;
}

EZ_ALWAYS_INLINE IDXGIFactory6* ezGALDeviceDX12::GetDXGIFactory() const
{
  return m_pDXGIFactory;
}

EZ_ALWAYS_INLINE ezGALCommandEncoder* ezGALDeviceDX12::GetCommandEncoder() const
{
  return m_pCommandEncoder.Borrow();
}

EZ_ALWAYS_INLINE ezFenceQueueDX12& ezGALDeviceDX12::GetFenceQueue() const
{
  return *m_pFenceQueue;
}

EZ_ALWAYS_INLINE ezQueryPoolDX12& ezGALDeviceDX12::GetQueryPool() const
{
  return *m_pQueryPool;
}

EZ_ALWAYS_INLINE ezCommandListPoolDX12& ezGALDeviceDX12::GetCommandListPool() const
{
  return *m_pCommandListPool;
}

EZ_ALWAYS_INLINE ezDescriptorHeapPoolDX12& ezGALDeviceDX12::GetDescriptorHeapPool() const
{
  return *m_pDescriptorHeapPool;
}

EZ_ALWAYS_INLINE ezStagingBufferPoolDX12& ezGALDeviceDX12::GetStagingBufferPool() const
{
  return *m_pStagingBufferPool;
}

EZ_ALWAYS_INLINE ezUniformBufferPoolDX12& ezGALDeviceDX12::GetUniformBufferPool() const
{
  return *m_pUniformBufferPool;
}

EZ_ALWAYS_INLINE ezInitContextDX12& ezGALDeviceDX12::GetInitContext() const
{
  return *m_pInitContext;
}

EZ_ALWAYS_INLINE const ezGALFormatLookupTableDX12& ezGALDeviceDX12::GetFormatLookupTable() const
{
  return m_FormatLookupTable;
}
