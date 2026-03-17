#pragma once

#include <RendererFoundation/Resources/Texture.h>

#include <d3d12.h>

struct ID3D12Resource;
class ezGALDeviceDX12;

namespace D3D12MA
{
  class Allocation;
} // namespace D3D12MA

class EZ_RENDERERDX12_DLL ezGALTextureDX12 : public ezGALTexture
{
public:
  EZ_ALWAYS_INLINE ID3D12Resource* GetDXResource() const { return m_pResource; }
  EZ_ALWAYS_INLINE D3D12MA::Allocation* GetAllocation() const { return m_pAllocation; }
  EZ_ALWAYS_INLINE D3D12_RESOURCE_STATES GetCurrentState() const { return m_CurrentState; }
  EZ_ALWAYS_INLINE void SetCurrentState(D3D12_RESOURCE_STATES state) { m_CurrentState = state; }
  EZ_ALWAYS_INLINE D3D12_CPU_DESCRIPTOR_HANDLE GetSRVDescriptor() const { return m_SRVDescriptor; }
  EZ_ALWAYS_INLINE DXGI_FORMAT GetDXGIFormat() const { return m_DXGIFormat; }

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;
  friend class ezGALSharedTextureDX12;

  ezGALTextureDX12(const ezGALTextureCreationDescription& Description);
  virtual ~ezGALTextureDX12();

  virtual ezResult InitPlatform(ezGALDevice* pDevice, ezArrayPtr<ezGALSystemMemoryDescription> pInitialData) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;
  virtual void SetDebugNamePlatform(const char* szName) const override;

  ezResult InitFromNativeObject(ezGALDeviceDX12* pDXDevice);

protected:
  ID3D12Resource* m_pResource = nullptr;
  D3D12MA::Allocation* m_pAllocation = nullptr;
  D3D12_RESOURCE_STATES m_CurrentState = D3D12_RESOURCE_STATE_COMMON;
  D3D12_CPU_DESCRIPTOR_HANDLE m_SRVDescriptor = {};
  DXGI_FORMAT m_DXGIFormat = DXGI_FORMAT_UNKNOWN;
};
