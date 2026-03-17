#pragma once

#include <RendererDX12/Resources/TextureDX12.h>

struct ID3D12Fence;

class EZ_RENDERERDX12_DLL ezGALSharedTextureDX12 : public ezGALTextureDX12, public ezGALSharedTexture
{
  using SUPER = ezGALTextureDX12;

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  ezGALSharedTextureDX12(const ezGALTextureCreationDescription& Description, ezEnum<ezGALSharedTextureType> sharedType, ezGALPlatformSharedHandle hSharedHandle);
  ~ezGALSharedTextureDX12();

  virtual ezResult InitPlatform(ezGALDevice* pDevice, ezArrayPtr<ezGALSystemMemoryDescription> pInitialData) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;

  virtual ezGALPlatformSharedHandle GetSharedHandle() const override;
  virtual void WaitSemaphoreGPU(ezUInt64 uiValue) const override;
  virtual void SignalSemaphoreGPU(ezUInt64 uiValue) const override;

protected:
  ezEnum<ezGALSharedTextureType> m_SharedType = ezGALSharedTextureType::None;
  ezGALPlatformSharedHandle m_hSharedHandle;
  ID3D12Fence* m_pFence = nullptr;
  HANDLE m_hSharedFenceHandle = nullptr;
};
