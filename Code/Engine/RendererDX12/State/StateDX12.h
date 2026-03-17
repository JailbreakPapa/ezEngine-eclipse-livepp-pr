#pragma once

#include <RendererDX12/RendererDX12DLL.h>
#include <RendererFoundation/State/State.h>

#include <d3d12.h>

class ezGALDeviceDX12;

class EZ_RENDERERDX12_DLL ezGALBlendStateDX12 : public ezGALBlendState
{
public:
  const D3D12_BLEND_DESC& GetBlendDesc() const { return m_BlendDesc; }

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  ezGALBlendStateDX12(const ezGALBlendStateCreationDescription& Description);
  ~ezGALBlendStateDX12();

  virtual ezResult InitPlatform(ezGALDevice* pDevice) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;

  D3D12_BLEND_DESC m_BlendDesc = {};
};

class EZ_RENDERERDX12_DLL ezGALDepthStencilStateDX12 : public ezGALDepthStencilState
{
public:
  const D3D12_DEPTH_STENCIL_DESC& GetDepthStencilDesc() const { return m_DepthStencilDesc; }

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  ezGALDepthStencilStateDX12(const ezGALDepthStencilStateCreationDescription& Description);
  ~ezGALDepthStencilStateDX12();

  virtual ezResult InitPlatform(ezGALDevice* pDevice) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;

  D3D12_DEPTH_STENCIL_DESC m_DepthStencilDesc = {};
};

class EZ_RENDERERDX12_DLL ezGALRasterizerStateDX12 : public ezGALRasterizerState
{
public:
  const D3D12_RASTERIZER_DESC& GetRasterizerDesc() const { return m_RasterizerDesc; }

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  ezGALRasterizerStateDX12(const ezGALRasterizerStateCreationDescription& Description);
  ~ezGALRasterizerStateDX12();

  virtual ezResult InitPlatform(ezGALDevice* pDevice) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;

  D3D12_RASTERIZER_DESC m_RasterizerDesc = {};
};

class EZ_RENDERERDX12_DLL ezGALSamplerStateDX12 : public ezGALSamplerState
{
public:
  D3D12_CPU_DESCRIPTOR_HANDLE GetSamplerDescriptor() const { return m_SamplerDescriptor; }
  const D3D12_SAMPLER_DESC& GetSamplerDesc() const { return m_SamplerDesc; }

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  ezGALSamplerStateDX12(const ezGALSamplerStateCreationDescription& Description);
  ~ezGALSamplerStateDX12();

  virtual ezResult InitPlatform(ezGALDevice* pDevice) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;
  virtual void SetDebugNamePlatform(const char* szName) const override;

  D3D12_SAMPLER_DESC m_SamplerDesc = {};
  D3D12_CPU_DESCRIPTOR_HANDLE m_SamplerDescriptor = {};
};
