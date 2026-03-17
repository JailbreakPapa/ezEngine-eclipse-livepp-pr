#pragma once

#include <RendererDX12/RendererDX12DLL.h>
#include <RendererFoundation/RendererFoundationDLL.h>
#include <RendererFoundation/Shader/BindGroupLayout.h>

/// D3D12 bind group layout implementation.
///
/// Counts the number of CBV, SRV, UAV and Sampler descriptors in the layout.
/// These counts are used when building descriptor tables in the root signature
/// (see ezGALPipelineLayoutDX12) and when allocating descriptor ranges for
/// bind groups (see ezGALBindGroupDX12).
class EZ_RENDERERDX12_DLL ezGALBindGroupLayoutDX12 : public ezGALBindGroupLayout
{
public:
  /// Total number of CBV + SRV + UAV descriptors in this layout.
  EZ_ALWAYS_INLINE ezUInt32 GetSrvUavCbvCount() const { return m_uiSrvUavCbvCount; }

  /// Total number of Sampler descriptors in this layout.
  EZ_ALWAYS_INLINE ezUInt32 GetSamplerCount() const { return m_uiSamplerCount; }

  /// Number of constant buffer views.
  EZ_ALWAYS_INLINE ezUInt32 GetCbvCount() const { return m_uiCbvCount; }

  /// Number of shader resource views.
  EZ_ALWAYS_INLINE ezUInt32 GetSrvCount() const { return m_uiSrvCount; }

  /// Number of unordered access views.
  EZ_ALWAYS_INLINE ezUInt32 GetUavCount() const { return m_uiUavCount; }

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  virtual ezResult InitPlatform(ezGALDevice* pDevice) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;

  ezGALBindGroupLayoutDX12(const ezGALBindGroupLayoutCreationDescription& Description);
  virtual ~ezGALBindGroupLayoutDX12();

  ezUInt32 m_uiCbvCount = 0;
  ezUInt32 m_uiSrvCount = 0;
  ezUInt32 m_uiUavCount = 0;
  ezUInt32 m_uiSrvUavCbvCount = 0;
  ezUInt32 m_uiSamplerCount = 0;
};
