#pragma once

#include <RendererDX12/RendererDX12DLL.h>
#include <RendererFoundation/RendererFoundationDLL.h>
#include <RendererFoundation/Shader/BindGroup.h>

#include <d3d12.h>

/// D3D12 bind group implementation.
///
/// A bind group holds GPU descriptor handles into the shader-visible descriptor heaps.
/// On InitPlatform, descriptors are allocated from the transient descriptor heap pool
/// and written using the device. The resulting GPU handles are stored here so they can
/// be bound to the command list as descriptor tables via SetGraphicsRootDescriptorTable
/// or SetComputeRootDescriptorTable.
class EZ_RENDERERDX12_DLL ezGALBindGroupDX12 : public ezGALBindGroup
{
public:
  EZ_ALWAYS_INLINE D3D12_GPU_DESCRIPTOR_HANDLE GetSrvUavCbvTableStart() const { return m_SrvUavCbvTableStart; }
  EZ_ALWAYS_INLINE D3D12_GPU_DESCRIPTOR_HANDLE GetSamplerTableStart() const { return m_SamplerTableStart; }
  EZ_ALWAYS_INLINE ezUInt32 GetSrvUavCbvCount() const { return m_uiSrvUavCbvCount; }
  EZ_ALWAYS_INLINE ezUInt32 GetSamplerCount() const { return m_uiSamplerCount; }

  virtual bool IsInvalidated() const override;

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  virtual ezResult InitPlatform(ezGALDevice* pDevice) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;
  virtual void Invalidate(ezGALDevice* pDevice) override;
  virtual void SetDebugNamePlatform(const char* szName) const override;

  ezGALBindGroupDX12(const ezGALBindGroupCreationDescription& Description);
  virtual ~ezGALBindGroupDX12();

private:
  D3D12_GPU_DESCRIPTOR_HANDLE m_SrvUavCbvTableStart = {};
  D3D12_GPU_DESCRIPTOR_HANDLE m_SamplerTableStart = {};
  ezUInt32 m_uiSrvUavCbvCount = 0;
  ezUInt32 m_uiSamplerCount = 0;
  bool m_bInvalidated = false;
};
