#pragma once

#include <RendererFoundation/Resources/ReadbackTexture.h>

#include <d3d12.h>

struct ID3D12Resource;
class ezGALDeviceDX12;

class EZ_RENDERERDX12_DLL ezGALReadbackTextureDX12 : public ezGALReadbackTexture
{
public:
  EZ_ALWAYS_INLINE ID3D12Resource* GetDXResource() const { return m_pReadbackResource; }
  EZ_ALWAYS_INLINE ezUInt64 GetRowPitch() const { return m_uiRowPitch; }

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  ezGALReadbackTextureDX12(const ezGALTextureCreationDescription& Description);
  virtual ~ezGALReadbackTextureDX12();

  virtual ezResult InitPlatform(ezGALDevice* pDevice) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;
  virtual void SetDebugNamePlatform(const char* szName) const override;

protected:
  ID3D12Resource* m_pReadbackResource = nullptr;
  ezUInt64 m_uiRowPitch = 0;
};
