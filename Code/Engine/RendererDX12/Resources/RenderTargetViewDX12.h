#pragma once

#include <RendererFoundation/Resources/RenderTargetView.h>

#include <d3d12.h>

class ezGALDeviceDX12;

class EZ_RENDERERDX12_DLL ezGALRenderTargetViewDX12 : public ezGALRenderTargetView
{
public:
  EZ_ALWAYS_INLINE D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor() const { return m_Descriptor; }
  EZ_ALWAYS_INLINE bool IsDepthStencil() const { return m_bIsDepthStencil; }

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  ezGALRenderTargetViewDX12(ezGALTexture* pTexture, const ezGALRenderTargetViewCreationDescription& Description);
  virtual ~ezGALRenderTargetViewDX12();

  virtual ezResult InitPlatform(ezGALDevice* pDevice) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;

  D3D12_CPU_DESCRIPTOR_HANDLE m_Descriptor = {};
  bool m_bIsDepthStencil = false;
};
