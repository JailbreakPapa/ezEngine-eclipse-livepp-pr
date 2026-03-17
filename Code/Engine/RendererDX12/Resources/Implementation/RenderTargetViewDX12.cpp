#include <RendererDX12/RendererDX12PCH.h>

#include <Foundation/Logging/Log.h>
#include <RendererDX12/Device/DeviceDX12.h>
#include <RendererDX12/Pools/DescriptorHeapPoolDX12.h>
#include <RendererDX12/Resources/RenderTargetViewDX12.h>
#include <RendererDX12/Resources/TextureDX12.h>
#include <RendererDX12/Utils/ConversionUtilsDX12.h>

#include <d3d12.h>

ezGALRenderTargetViewDX12::ezGALRenderTargetViewDX12(ezGALTexture* pTexture, const ezGALRenderTargetViewCreationDescription& Description)
  : ezGALRenderTargetView(pTexture, Description)
{
}

ezGALRenderTargetViewDX12::~ezGALRenderTargetViewDX12() = default;

ezResult ezGALRenderTargetViewDX12::InitPlatform(ezGALDevice* pDevice)
{
  const ezGALTexture* pTexture = nullptr;
  if (!m_Description.m_hTexture.IsInvalidated())
    pTexture = pDevice->GetTexture(m_Description.m_hTexture);

  if (pTexture == nullptr)
  {
    ezLog::Error("No valid texture handle given for render target view creation!");
    return EZ_FAILURE;
  }

  const ezGALTextureCreationDescription& texDesc = pTexture->GetDescription();
  ezGALResourceFormat::Enum viewFormat = texDesc.m_Format;

  if (m_Description.m_OverrideViewFormat != ezGALResourceFormat::Invalid)
    viewFormat = m_Description.m_OverrideViewFormat;

  DXGI_FORMAT dxgiFormat = ezConversionUtilsDX12::ToDXGIFormat(viewFormat);
  if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
  {
    ezLog::Error("Couldn't get DXGI format for render target view!");
    return EZ_FAILURE;
  }

  const ezGALTextureDX12* pDX12Texture = static_cast<const ezGALTextureDX12*>(pTexture->GetParentResource());
  ID3D12Resource* pDXResource = pDX12Texture->GetDXResource();

  ezGALDeviceDX12* pDXDevice = static_cast<ezGALDeviceDX12*>(pDevice);
  ID3D12Device* pD3D12Device = pDXDevice->GetDXDevice();

  m_bIsDepthStencil = ezGALResourceFormat::IsDepthFormat(viewFormat);

  if (m_bIsDepthStencil)
  {
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = dxgiFormat;

    const ezEnum<ezGALTextureType> type = m_Description.m_OverrideViewType != ezGALTextureType::Invalid ? m_Description.m_OverrideViewType : texDesc.m_Type;
    if (texDesc.m_SampleCount == ezGALMSAASampleCount::None)
    {
      switch (type)
      {
        case ezGALTextureType::Texture2D:
        case ezGALTextureType::Texture2DShared:
          dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
          dsvDesc.Texture2D.MipSlice = m_Description.m_uiMipLevel;
          break;

        case ezGALTextureType::Texture2DProxy:
        case ezGALTextureType::Texture2DArray:
          dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
          dsvDesc.Texture2DArray.MipSlice = m_Description.m_uiMipLevel;
          dsvDesc.Texture2DArray.FirstArraySlice = m_Description.m_uiFirstSlice;
          dsvDesc.Texture2DArray.ArraySize = m_Description.m_uiSliceCount;
          break;

          EZ_DEFAULT_CASE_NOT_IMPLEMENTED;
      }
    }
    else
    {
      switch (type)
      {
        case ezGALTextureType::Texture2D:
        case ezGALTextureType::Texture2DShared:
          dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
          break;

        case ezGALTextureType::Texture2DProxy:
        case ezGALTextureType::Texture2DArray:
          dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
          dsvDesc.Texture2DMSArray.FirstArraySlice = m_Description.m_uiFirstSlice;
          dsvDesc.Texture2DMSArray.ArraySize = m_Description.m_uiSliceCount;
          break;

          EZ_DEFAULT_CASE_NOT_IMPLEMENTED;
      }
    }

    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    if (m_Description.m_bReadOnly)
      dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;

    m_Descriptor = pDXDevice->GetDescriptorHeapPool().AllocateStagingDSV();
    pD3D12Device->CreateDepthStencilView(pDXResource, &dsvDesc, m_Descriptor);
  }
  else
  {
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = dxgiFormat;

    if (texDesc.m_SampleCount == ezGALMSAASampleCount::None)
    {
      switch (texDesc.m_Type)
      {
        case ezGALTextureType::Texture2D:
        case ezGALTextureType::Texture2DShared:
          rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
          rtvDesc.Texture2D.MipSlice = m_Description.m_uiMipLevel;
          rtvDesc.Texture2D.PlaneSlice = 0;
          break;

        case ezGALTextureType::Texture2DProxy:
        case ezGALTextureType::Texture2DArray:
        case ezGALTextureType::TextureCube:
        case ezGALTextureType::TextureCubeArray:
          rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
          rtvDesc.Texture2DArray.MipSlice = m_Description.m_uiMipLevel;
          rtvDesc.Texture2DArray.FirstArraySlice = m_Description.m_uiFirstSlice;
          rtvDesc.Texture2DArray.ArraySize = m_Description.m_uiSliceCount;
          rtvDesc.Texture2DArray.PlaneSlice = 0;
          break;

        case ezGALTextureType::Texture3D:
          rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
          rtvDesc.Texture3D.MipSlice = m_Description.m_uiMipLevel;
          rtvDesc.Texture3D.FirstWSlice = m_Description.m_uiFirstSlice;
          rtvDesc.Texture3D.WSize = m_Description.m_uiSliceCount;
          break;

          EZ_DEFAULT_CASE_NOT_IMPLEMENTED;
      }
    }
    else
    {
      switch (texDesc.m_Type)
      {
        case ezGALTextureType::Texture2D:
        case ezGALTextureType::Texture2DShared:
          rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
          break;

        case ezGALTextureType::Texture2DProxy:
        case ezGALTextureType::Texture2DArray:
        case ezGALTextureType::TextureCube:
        case ezGALTextureType::TextureCubeArray:
          rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
          rtvDesc.Texture2DMSArray.FirstArraySlice = m_Description.m_uiFirstSlice;
          rtvDesc.Texture2DMSArray.ArraySize = m_Description.m_uiSliceCount;
          break;

          EZ_DEFAULT_CASE_NOT_IMPLEMENTED;
      }
    }

    m_Descriptor = pDXDevice->GetDescriptorHeapPool().AllocateStagingRTV();
    pD3D12Device->CreateRenderTargetView(pDXResource, &rtvDesc, m_Descriptor);
  }

  return EZ_SUCCESS;
}

ezResult ezGALRenderTargetViewDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  // Descriptor handles are managed by the staging descriptor heap pool.
  // They will be reclaimed when the pool is reset or destroyed.
  m_Descriptor = {};
  m_bIsDepthStencil = false;

  return EZ_SUCCESS;
}
