#pragma once

#include <RendererFoundation/Resources/Buffer.h>

#include <d3d12.h>
#include <dxgi.h>

struct ID3D12Resource;
class ezGALDeviceDX12;

namespace D3D12MA
{
  class Allocation;
} // namespace D3D12MA

class EZ_RENDERERDX12_DLL ezGALBufferDX12 : public ezGALBuffer
{
public:
  EZ_ALWAYS_INLINE ID3D12Resource* GetDXResource() const { return m_pResource; }
  EZ_ALWAYS_INLINE D3D12MA::Allocation* GetAllocation() const { return m_pAllocation; }
  EZ_ALWAYS_INLINE D3D12_RESOURCE_STATES GetCurrentState() const { return m_CurrentState; }
  EZ_ALWAYS_INLINE void SetCurrentState(D3D12_RESOURCE_STATES state) { m_CurrentState = state; }
  EZ_ALWAYS_INLINE DXGI_FORMAT GetIndexFormat() const { return m_IndexFormat; }
  EZ_ALWAYS_INLINE D3D12_CPU_DESCRIPTOR_HANDLE GetSRVDescriptor() const { return m_SRVDescriptor; }
  EZ_ALWAYS_INLINE D3D12_CPU_DESCRIPTOR_HANDLE GetUAVDescriptor() const { return m_UAVDescriptor; }

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  ezGALBufferDX12(const ezGALBufferCreationDescription& Description);
  virtual ~ezGALBufferDX12();

  virtual ezResult InitPlatform(ezGALDevice* pDevice, ezArrayPtr<const ezUInt8> pInitialData) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;
  virtual void SetDebugNamePlatform(const char* szName) const override;

protected:
  ID3D12Resource* m_pResource = nullptr;
  D3D12MA::Allocation* m_pAllocation = nullptr;
  D3D12_RESOURCE_STATES m_CurrentState = D3D12_RESOURCE_STATE_COMMON;
  D3D12_CPU_DESCRIPTOR_HANDLE m_SRVDescriptor = {};
  D3D12_CPU_DESCRIPTOR_HANDLE m_UAVDescriptor = {};
  DXGI_FORMAT m_IndexFormat = DXGI_FORMAT_UNKNOWN;
};
