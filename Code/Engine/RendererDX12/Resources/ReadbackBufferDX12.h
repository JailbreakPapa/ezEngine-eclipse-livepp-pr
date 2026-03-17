#pragma once

#include <RendererFoundation/Resources/ReadbackBuffer.h>

#include <d3d12.h>

struct ID3D12Resource;
class ezGALDeviceDX12;

class EZ_RENDERERDX12_DLL ezGALReadbackBufferDX12 : public ezGALReadbackBuffer
{
public:
  EZ_ALWAYS_INLINE ID3D12Resource* GetDXResource() const { return m_pReadbackResource; }

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  ezGALReadbackBufferDX12(const ezGALBufferCreationDescription& Description);
  virtual ~ezGALReadbackBufferDX12();

  virtual ezResult InitPlatform(ezGALDevice* pDevice) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;
  virtual void SetDebugNamePlatform(const char* szName) const override;

protected:
  ID3D12Resource* m_pReadbackResource = nullptr;
};
