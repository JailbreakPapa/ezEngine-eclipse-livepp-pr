#pragma once

#include <RendererDX12/RendererDX12DLL.h>
#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/Device/SwapChain.h>

#include <d3d12.h>
#include <dxgi1_6.h>

class ezGALDeviceDX12;

/// D3D12 swap chain implementation using IDXGISwapChain4 with triple-buffered FLIP_DISCARD.
///
/// Manages back buffer resources as ezGALTexture wrappers around the native swap chain buffers.
/// Each back buffer has an associated RTV descriptor allocated from the device's descriptor heap pool.
/// AcquireNextRenderTarget updates the current back buffer index before each frame, and
/// PresentRenderTarget submits the present call after rendering completes.
class EZ_RENDERERDX12_DLL ezGALSwapChainDX12 : public ezGALWindowSwapChain
{
public:
  void SetPresentMode(ezEnum<ezGALPresentMode> presentMode) { m_PresentMode = presentMode; }

  IDXGISwapChain4* GetDXSwapChain() const { return m_pSwapChain; }
  ID3D12Resource* GetCurrentBackBuffer() const;
  ezUInt32 GetCurrentBackBufferIndex() const { return m_uiCurrentBackBuffer; }

  D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRTV() const;

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  ezGALSwapChainDX12(const ezGALWindowSwapChainCreationDescription& Description);
  ~ezGALSwapChainDX12();

  virtual void AcquireNextRenderTarget(ezGALDevice* pDevice) override;
  virtual void PresentRenderTarget(ezGALDevice* pDevice) override;
  virtual ezResult UpdateSwapChain(ezGALDevice* pDevice, ezEnum<ezGALPresentMode> newPresentMode) override;

  virtual ezResult InitPlatform(ezGALDevice* pDevice) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;

private:
  void CreateBackBufferResources(ezGALDeviceDX12* pDeviceDX12);
  void DestroyBackBufferResources(ezGALDeviceDX12* pDeviceDX12);

  static constexpr ezUInt32 BACK_BUFFER_COUNT = 3;

  IDXGISwapChain4* m_pSwapChain = nullptr;
  ID3D12Resource* m_pBackBuffers[BACK_BUFFER_COUNT] = {};
  ezGALTextureHandle m_hBackBufferTextures[BACK_BUFFER_COUNT];
  D3D12_CPU_DESCRIPTOR_HANDLE m_BackBufferRTVs[BACK_BUFFER_COUNT] = {};
  ezUInt32 m_uiCurrentBackBuffer = 0;
  ezEnum<ezGALPresentMode> m_PresentMode;
};
