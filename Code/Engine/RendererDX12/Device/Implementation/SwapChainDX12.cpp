#include <RendererDX12/RendererDX12PCH.h>

#include <Core/System/Window.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Platform/Win/Utils/HResultUtils.h>
#include <Foundation/Platform/Win/Utils/IncludeWindows.h>
#include <RendererDX12/Device/DeviceDX12.h>
#include <RendererDX12/Device/SwapChainDX12.h>
#include <RendererDX12/Pools/DescriptorHeapPoolDX12.h>
#include <RendererFoundation/CommandEncoder/CommandEncoder.h>

ezGALSwapChainDX12::ezGALSwapChainDX12(const ezGALWindowSwapChainCreationDescription& Description)
  : ezGALWindowSwapChain(Description)
{
}

ezGALSwapChainDX12::~ezGALSwapChainDX12() = default;

ID3D12Resource* ezGALSwapChainDX12::GetCurrentBackBuffer() const
{
  return m_pBackBuffers[m_uiCurrentBackBuffer];
}

D3D12_CPU_DESCRIPTOR_HANDLE ezGALSwapChainDX12::GetCurrentBackBufferRTV() const
{
  return m_BackBufferRTVs[m_uiCurrentBackBuffer];
}

ezResult ezGALSwapChainDX12::InitPlatform(ezGALDevice* pDevice)
{
  ezGALDeviceDX12* pDXDevice = static_cast<ezGALDeviceDX12*>(pDevice);
  m_PresentMode = m_WindowDesc.m_InitialPresentMode;

  IDXGIFactory2* pFactory2 = nullptr;
  HRESULT hr = pDXDevice->GetDXGIFactory()->QueryInterface(IID_PPV_ARGS(&pFactory2));
  if (FAILED(hr))
  {
    ezLog::Error("Failed to query IDXGIFactory2 interface: {}", ezArgErrorCode(hr));
    return EZ_FAILURE;
  }
  EZ_SCOPE_EXIT(EZ_GAL_DX12_RELEASE(pFactory2));

  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
  swapChainDesc.Width = m_WindowDesc.m_pWindow->GetClientAreaSize().width;
  swapChainDesc.Height = m_WindowDesc.m_pWindow->GetClientAreaSize().height;
  swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.Stereo = FALSE;
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.SampleDesc.Quality = 0;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.BufferCount = BACK_BUFFER_COUNT;
  swapChainDesc.Scaling = DXGI_SCALING_NONE;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
  swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

  HWND hWnd = ezMinWindows::ToNative(m_WindowDesc.m_pWindow->GetNativeWindowHandle());

  IDXGISwapChain1* pSwapChain1 = nullptr;
  hr = pFactory2->CreateSwapChainForHwnd(
    pDXDevice->GetGraphicsQueue(),
    hWnd,
    &swapChainDesc,
    nullptr,
    nullptr,
    &pSwapChain1);

  if (FAILED(hr))
  {
    ezLog::Error("Failed to create D3D12 swap chain: {}", ezArgErrorCode(hr));
    return EZ_FAILURE;
  }

  hr = pSwapChain1->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
  pSwapChain1->Release();

  if (FAILED(hr))
  {
    ezLog::Error("Failed to query IDXGISwapChain4 interface: {}", ezArgErrorCode(hr));
    return EZ_FAILURE;
  }

  // Disable Alt+Enter fullscreen toggle, the engine manages fullscreen state itself.
  pFactory2->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

  // We have created a surface on a window, the window must not be destroyed while the surface is still alive.
  m_WindowDesc.m_pWindow->AddReference();

  CreateBackBufferResources(pDXDevice);

  return EZ_SUCCESS;
}

void ezGALSwapChainDX12::CreateBackBufferResources(ezGALDeviceDX12* pDXDevice)
{
  for (ezUInt32 i = 0; i < BACK_BUFFER_COUNT; ++i)
  {
    HRESULT hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pBackBuffers[i]));
    if (FAILED(hr))
    {
      ezLog::Error("Failed to get swap chain back buffer {}: {}", i, ezArgErrorCode(hr));
      continue;
    }

    // Create an ezGALTexture wrapper around the native back buffer resource.
    ezGALTextureCreationDescription texDesc;
    texDesc.m_uiWidth = m_WindowDesc.m_pWindow->GetClientAreaSize().width;
    texDesc.m_uiHeight = m_WindowDesc.m_pWindow->GetClientAreaSize().height;
    texDesc.m_SampleCount = ezGALMSAASampleCount::None;
    texDesc.m_pExisitingNativeObject = m_pBackBuffers[i];
    texDesc.m_TextureFlags = ezGALTextureUsageFlags::RenderTarget;
    texDesc.m_Format = m_WindowDesc.m_BackBufferFormat;
    texDesc.m_ResourceAccess.m_bImmutable = true;

    m_hBackBufferTextures[i] = pDXDevice->CreateTexture(texDesc);
    EZ_ASSERT_RELEASE(!m_hBackBufferTextures[i].IsInvalidated(), "Couldn't create native backbuffer texture object for buffer {}!", i);

    // Create an RTV descriptor for this back buffer.
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;

    m_BackBufferRTVs[i] = pDXDevice->GetDescriptorHeapPool().AllocateStagingRTV();
    pDXDevice->GetDXDevice()->CreateRenderTargetView(m_pBackBuffers[i], &rtvDesc, m_BackBufferRTVs[i]);
  }

  m_uiCurrentBackBuffer = m_pSwapChain->GetCurrentBackBufferIndex();

  // Set up render targets - use the first back buffer initially.
  m_RenderTargets.m_hRTs[0] = m_hBackBufferTextures[m_uiCurrentBackBuffer];
  m_CurrentSize = ezSizeU32(
    m_WindowDesc.m_pWindow->GetClientAreaSize().width,
    m_WindowDesc.m_pWindow->GetClientAreaSize().height);
}

void ezGALSwapChainDX12::DestroyBackBufferResources(ezGALDeviceDX12* pDXDevice)
{
  for (ezUInt32 i = 0; i < BACK_BUFFER_COUNT; ++i)
  {
    if (!m_hBackBufferTextures[i].IsInvalidated())
    {
      pDXDevice->DestroyTexture(m_hBackBufferTextures[i]);
      m_hBackBufferTextures[i].Invalidate();
    }

    // The back buffer resources are owned by the swap chain, but we obtained references via GetBuffer.
    EZ_GAL_DX12_RELEASE(m_pBackBuffers[i]);
    m_BackBufferRTVs[i] = {};
  }

  m_RenderTargets.m_hRTs[0].Invalidate();
}

void ezGALSwapChainDX12::AcquireNextRenderTarget(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  m_uiCurrentBackBuffer = m_pSwapChain->GetCurrentBackBufferIndex();
  m_RenderTargets.m_hRTs[0] = m_hBackBufferTextures[m_uiCurrentBackBuffer];
}

void ezGALSwapChainDX12::PresentRenderTarget(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  UINT syncInterval = (m_PresentMode == ezGALPresentMode::VSync) ? 1 : 0;
  UINT presentFlags = 0;

  // Allow tearing is required when using variable refresh rate displays in windowed mode.
  if (m_PresentMode == ezGALPresentMode::Immediate)
  {
    presentFlags |= DXGI_PRESENT_ALLOW_TEARING;
  }

  HRESULT hr = m_pSwapChain->Present(syncInterval, presentFlags);
  if (FAILED(hr))
  {
    ezLog::Error("Swap chain Present failed: {}", ezArgErrorCode(hr));
  }
}

ezResult ezGALSwapChainDX12::UpdateSwapChain(ezGALDevice* pDevice, ezEnum<ezGALPresentMode> newPresentMode)
{
  ezGALDeviceDX12* pDXDevice = static_cast<ezGALDeviceDX12*>(pDevice);
  m_PresentMode = newPresentMode;

  DestroyBackBufferResources(pDXDevice);

  // Need to flush dead objects or ResizeBuffers will fail as the back buffers are still referenced.
  pDXDevice->FlushDeadObjects();

  HRESULT hr = m_pSwapChain->ResizeBuffers(
    BACK_BUFFER_COUNT,
    m_WindowDesc.m_pWindow->GetClientAreaSize().width,
    m_WindowDesc.m_pWindow->GetClientAreaSize().height,
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);

  if (FAILED(hr))
  {
    ezLog::Error("UpdateSwapChain: ResizeBuffers failed: {}", ezArgErrorCode(hr));
    return EZ_FAILURE;
  }

  CreateBackBufferResources(pDXDevice);

  return EZ_SUCCESS;
}

ezResult ezGALSwapChainDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  ezGALDeviceDX12* pDXDevice = static_cast<ezGALDeviceDX12*>(pDevice);

  DestroyBackBufferResources(pDXDevice);

  if (m_pSwapChain)
  {
    // Full screen swap chains must be switched to windowed mode before destruction.
    m_pSwapChain->SetFullscreenState(FALSE, nullptr);

    EZ_GAL_DX12_RELEASE(m_pSwapChain);

    m_WindowDesc.m_pWindow->RemoveReference();
  }

  return EZ_SUCCESS;
}

EZ_STATICLINK_FILE(RendererDX12, RendererDX12_Device_Implementation_SwapChainDX12);
