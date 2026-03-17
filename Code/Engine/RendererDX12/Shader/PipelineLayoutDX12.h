#pragma once

#include <RendererDX12/RendererDX12DLL.h>
#include <RendererFoundation/RendererFoundationDLL.h>
#include <RendererFoundation/Shader/PipelineLayout.h>

#include <d3d12.h>

/// D3D12 pipeline layout implementation.
///
/// Builds and owns the ID3D12RootSignature that maps bind group layouts to root parameters.
/// Each bind group can produce up to two descriptor table root parameters:
/// one for CBV/SRV/UAV and one for Samplers. Push constants are mapped to root constants.
class EZ_RENDERERDX12_DLL ezGALPipelineLayoutDX12 : public ezGALPipelineLayout
{
public:
  EZ_ALWAYS_INLINE ID3D12RootSignature* GetRootSignature() const { return m_pRootSignature; }

  /// Returns the root parameter index for the CBV/SRV/UAV descriptor table of the given bind group.
  /// Returns -1 if that bind group has no such descriptors.
  EZ_ALWAYS_INLINE ezInt8 GetSrvUavCbvRootParamIndex(ezUInt32 uiBindGroup) const { return m_iSrvUavCbvRootParamIndex[uiBindGroup]; }

  /// Returns the root parameter index for the Sampler descriptor table of the given bind group.
  /// Returns -1 if that bind group has no samplers.
  EZ_ALWAYS_INLINE ezInt8 GetSamplerRootParamIndex(ezUInt32 uiBindGroup) const { return m_iSamplerRootParamIndex[uiBindGroup]; }

  /// Returns the root parameter index for push constants, or -1 if none.
  EZ_ALWAYS_INLINE ezInt8 GetPushConstantsRootParamIndex() const { return m_iPushConstantsRootParamIndex; }

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  virtual ezResult InitPlatform(ezGALDevice* pDevice) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;

  ezGALPipelineLayoutDX12(const ezGALPipelineLayoutCreationDescription& Description);
  virtual ~ezGALPipelineLayoutDX12();

  ID3D12RootSignature* m_pRootSignature = nullptr;

  ezInt8 m_iSrvUavCbvRootParamIndex[EZ_GAL_MAX_BIND_GROUPS] = {-1, -1, -1, -1};
  ezInt8 m_iSamplerRootParamIndex[EZ_GAL_MAX_BIND_GROUPS] = {-1, -1, -1, -1};
  ezInt8 m_iPushConstantsRootParamIndex = -1;
};
