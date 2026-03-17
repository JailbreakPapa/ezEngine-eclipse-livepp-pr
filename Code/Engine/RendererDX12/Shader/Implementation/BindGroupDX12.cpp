#include <RendererDX12/RendererDX12PCH.h>

#include <Foundation/Logging/Log.h>
#include <RendererDX12/Device/DeviceDX12.h>
#include <RendererDX12/Pools/DescriptorHeapPoolDX12.h>
#include <RendererDX12/Resources/BufferDX12.h>
#include <RendererDX12/Resources/TextureDX12.h>
#include <RendererDX12/Shader/BindGroupDX12.h>
#include <RendererDX12/Shader/BindGroupLayoutDX12.h>
#include <RendererDX12/State/StateDX12.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Resources/Buffer.h>
#include <RendererFoundation/Resources/Texture.h>

ezGALBindGroupDX12::ezGALBindGroupDX12(const ezGALBindGroupCreationDescription& Description)
  : ezGALBindGroup(Description)
{
}

ezGALBindGroupDX12::~ezGALBindGroupDX12() = default;

ezResult ezGALBindGroupDX12::InitPlatform(ezGALDevice* pDevice)
{
  ezGALDeviceDX12* pDX12Device = static_cast<ezGALDeviceDX12*>(pDevice);
  ID3D12Device* pD3D12Device = pDX12Device->GetDXDevice();
  ezDescriptorHeapPoolDX12& heapPool = pDX12Device->GetDescriptorHeapPool();

  // Get the layout to find out how many descriptors we need.
  const ezGALBindGroupLayout* pLayoutBase = pDevice->GetBindGroupLayout(m_Description.m_hBindGroupLayout);
  if (pLayoutBase == nullptr)
  {
    ezLog::Error("Bind group layout is invalid.");
    return EZ_FAILURE;
  }

  const ezGALBindGroupLayoutDX12* pLayout = static_cast<const ezGALBindGroupLayoutDX12*>(pLayoutBase);
  const auto& bindings = pLayout->GetDescription().m_ResourceBindings;

  m_uiSrvUavCbvCount = pLayout->GetSrvUavCbvCount();
  m_uiSamplerCount = pLayout->GetSamplerCount();

  // Allocate from the transient (shader-visible) descriptor heaps.
  ezDescriptorRangeDX12 srvUavCbvRange = {};
  if (m_uiSrvUavCbvCount > 0)
  {
    srvUavCbvRange = heapPool.AllocateTransientSrvUavCbv(m_uiSrvUavCbvCount);
    m_SrvUavCbvTableStart = srvUavCbvRange.m_gpuStart;
  }

  ezDescriptorRangeDX12 samplerRange = {};
  if (m_uiSamplerCount > 0)
  {
    samplerRange = heapPool.AllocateTransientSampler(m_uiSamplerCount);
    m_SamplerTableStart = samplerRange.m_gpuStart;
  }

  // Build staging descriptors and copy them into the shader-visible heap.
  ezHybridArray<D3D12_CPU_DESCRIPTOR_HANDLE, 16> stagingSrvUavCbvHandles;
  ezHybridArray<D3D12_CPU_DESCRIPTOR_HANDLE, 8> stagingSamplerHandles;

  EZ_ASSERT_DEV(m_Description.m_BindGroupItems.GetCount() == bindings.GetCount(),
    "Bind group item count must match binding count.");

  for (ezUInt32 i = 0; i < bindings.GetCount(); ++i)
  {
    const auto& binding = bindings[i];
    const auto& item = m_Description.m_BindGroupItems[i];

    switch (binding.m_ResourceType)
    {
      case ezGALShaderResourceType::ConstantBuffer:
      case ezGALShaderResourceType::PushConstants:
      {
        D3D12_CPU_DESCRIPTOR_HANDLE stagingHandle = heapPool.AllocateStagingSrvUavCbv();
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};

        if (item.m_Flags.IsSet(ezGALBindGroupItemFlags::Buffer))
        {
          const ezGALBufferDX12* pBuffer = static_cast<const ezGALBufferDX12*>(pDevice->GetBuffer(item.m_Buffer.m_hBuffer));
          if (pBuffer != nullptr && pBuffer->GetDXResource() != nullptr)
          {
            cbvDesc.BufferLocation = pBuffer->GetDXResource()->GetGPUVirtualAddress();
            cbvDesc.SizeInBytes = static_cast<UINT>((pBuffer->GetSize() + 255) & ~255u);
          }
        }

        pD3D12Device->CreateConstantBufferView(&cbvDesc, stagingHandle);
        stagingSrvUavCbvHandles.PushBack(stagingHandle);
        break;
      }

      case ezGALShaderResourceType::Texture:
      case ezGALShaderResourceType::TextureAndSampler:
      {
        // SRV for the texture: use pre-built descriptor from the texture object.
        if (item.m_Flags.IsSet(ezGALBindGroupItemFlags::Texture))
        {
          const ezGALTextureDX12* pTexture = static_cast<const ezGALTextureDX12*>(pDevice->GetTexture(item.m_Texture.m_hTexture));
          if (pTexture != nullptr && pTexture->GetSRVDescriptor().ptr != 0)
          {
            stagingSrvUavCbvHandles.PushBack(pTexture->GetSRVDescriptor());
          }
          else
          {
            // Null SRV
            D3D12_CPU_DESCRIPTOR_HANDLE nullHandle = heapPool.AllocateStagingSrvUavCbv();
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            pD3D12Device->CreateShaderResourceView(nullptr, &srvDesc, nullHandle);
            stagingSrvUavCbvHandles.PushBack(nullHandle);
          }
        }
        else
        {
          D3D12_CPU_DESCRIPTOR_HANDLE nullHandle = heapPool.AllocateStagingSrvUavCbv();
          D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
          srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
          srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
          srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
          pD3D12Device->CreateShaderResourceView(nullptr, &srvDesc, nullHandle);
          stagingSrvUavCbvHandles.PushBack(nullHandle);
        }

        // Sampler for TextureAndSampler types: use pre-built descriptor from sampler state.
        if (binding.m_ResourceType == ezGALShaderResourceType::TextureAndSampler)
        {
          const ezGALSamplerStateDX12* pSampler = nullptr;
          if (item.m_Flags.IsSet(ezGALBindGroupItemFlags::Texture))
          {
            pSampler = static_cast<const ezGALSamplerStateDX12*>(pDevice->GetSamplerState(item.m_Texture.m_hSampler));
          }

          if (pSampler != nullptr && pSampler->GetSamplerDescriptor().ptr != 0)
          {
            stagingSamplerHandles.PushBack(pSampler->GetSamplerDescriptor());
          }
          else
          {
            D3D12_CPU_DESCRIPTOR_HANDLE defaultHandle = heapPool.AllocateStagingSampler();
            D3D12_SAMPLER_DESC defaultSampler = {};
            defaultSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            defaultSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            defaultSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            defaultSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            defaultSampler.MaxLOD = D3D12_FLOAT32_MAX;
            pD3D12Device->CreateSampler(&defaultSampler, defaultHandle);
            stagingSamplerHandles.PushBack(defaultHandle);
          }
        }
        break;
      }

      case ezGALShaderResourceType::TexelBuffer:
      case ezGALShaderResourceType::StructuredBuffer:
      case ezGALShaderResourceType::ByteAddressBuffer:
      {
        // Use pre-built SRV descriptor from the buffer object.
        if (item.m_Flags.IsSet(ezGALBindGroupItemFlags::Buffer))
        {
          const ezGALBufferDX12* pBuffer = static_cast<const ezGALBufferDX12*>(pDevice->GetBuffer(item.m_Buffer.m_hBuffer));
          if (pBuffer != nullptr && pBuffer->GetSRVDescriptor().ptr != 0)
          {
            stagingSrvUavCbvHandles.PushBack(pBuffer->GetSRVDescriptor());
          }
          else
          {
            D3D12_CPU_DESCRIPTOR_HANDLE nullHandle = heapPool.AllocateStagingSrvUavCbv();
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
            pD3D12Device->CreateShaderResourceView(nullptr, &srvDesc, nullHandle);
            stagingSrvUavCbvHandles.PushBack(nullHandle);
          }
        }
        else
        {
          D3D12_CPU_DESCRIPTOR_HANDLE nullHandle = heapPool.AllocateStagingSrvUavCbv();
          D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
          srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
          srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
          srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
          srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
          pD3D12Device->CreateShaderResourceView(nullptr, &srvDesc, nullHandle);
          stagingSrvUavCbvHandles.PushBack(nullHandle);
        }
        break;
      }

      case ezGALShaderResourceType::TextureRW:
      {
        if (item.m_Flags.IsSet(ezGALBindGroupItemFlags::Texture))
        {
          const ezGALTextureDX12* pTexture = static_cast<const ezGALTextureDX12*>(pDevice->GetTexture(item.m_Texture.m_hTexture));
          if (pTexture != nullptr)
          {
            D3D12_CPU_DESCRIPTOR_HANDLE uavHandle = heapPool.AllocateStagingSrvUavCbv();
            pD3D12Device->CreateUnorderedAccessView(pTexture->GetDXResource(), nullptr, nullptr, uavHandle);
            stagingSrvUavCbvHandles.PushBack(uavHandle);
          }
          else
          {
            D3D12_CPU_DESCRIPTOR_HANDLE nullHandle = heapPool.AllocateStagingSrvUavCbv();
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            pD3D12Device->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, nullHandle);
            stagingSrvUavCbvHandles.PushBack(nullHandle);
          }
        }
        else
        {
          D3D12_CPU_DESCRIPTOR_HANDLE nullHandle = heapPool.AllocateStagingSrvUavCbv();
          D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
          uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
          uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
          pD3D12Device->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, nullHandle);
          stagingSrvUavCbvHandles.PushBack(nullHandle);
        }
        break;
      }

      case ezGALShaderResourceType::TexelBufferRW:
      case ezGALShaderResourceType::StructuredBufferRW:
      case ezGALShaderResourceType::ByteAddressBufferRW:
      {
        // Use pre-built UAV descriptor from the buffer object.
        if (item.m_Flags.IsSet(ezGALBindGroupItemFlags::Buffer))
        {
          const ezGALBufferDX12* pBuffer = static_cast<const ezGALBufferDX12*>(pDevice->GetBuffer(item.m_Buffer.m_hBuffer));
          if (pBuffer != nullptr && pBuffer->GetUAVDescriptor().ptr != 0)
          {
            stagingSrvUavCbvHandles.PushBack(pBuffer->GetUAVDescriptor());
          }
          else
          {
            D3D12_CPU_DESCRIPTOR_HANDLE nullHandle = heapPool.AllocateStagingSrvUavCbv();
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
            pD3D12Device->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, nullHandle);
            stagingSrvUavCbvHandles.PushBack(nullHandle);
          }
        }
        else
        {
          D3D12_CPU_DESCRIPTOR_HANDLE nullHandle = heapPool.AllocateStagingSrvUavCbv();
          D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
          uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
          uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
          uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
          pD3D12Device->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, nullHandle);
          stagingSrvUavCbvHandles.PushBack(nullHandle);
        }
        break;
      }

      case ezGALShaderResourceType::Sampler:
      {
        const ezGALSamplerStateDX12* pSampler = nullptr;
        if (item.m_Flags.IsSet(ezGALBindGroupItemFlags::Sampler))
        {
          pSampler = static_cast<const ezGALSamplerStateDX12*>(pDevice->GetSamplerState(item.m_Sampler.m_hSampler));
        }

        if (pSampler != nullptr && pSampler->GetSamplerDescriptor().ptr != 0)
        {
          stagingSamplerHandles.PushBack(pSampler->GetSamplerDescriptor());
        }
        else
        {
          D3D12_CPU_DESCRIPTOR_HANDLE defaultHandle = heapPool.AllocateStagingSampler();
          D3D12_SAMPLER_DESC defaultSampler = {};
          defaultSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
          defaultSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
          defaultSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
          defaultSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
          defaultSampler.MaxLOD = D3D12_FLOAT32_MAX;
          pD3D12Device->CreateSampler(&defaultSampler, defaultHandle);
          stagingSamplerHandles.PushBack(defaultHandle);
        }
        break;
      }

      default:
        break;
    }
  }

  // Copy staging descriptors into the shader-visible heap.
  if (!stagingSrvUavCbvHandles.IsEmpty())
  {
    heapPool.CopyToShaderVisible(stagingSrvUavCbvHandles.GetCount(), stagingSrvUavCbvHandles.GetData(), srvUavCbvRange);
  }

  if (!stagingSamplerHandles.IsEmpty())
  {
    heapPool.CopyToShaderVisible(stagingSamplerHandles.GetCount(), stagingSamplerHandles.GetData(), samplerRange);
  }

  m_bInvalidated = false;
  return EZ_SUCCESS;
}

ezResult ezGALBindGroupDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  m_SrvUavCbvTableStart = {};
  m_SamplerTableStart = {};
  m_uiSrvUavCbvCount = 0;
  m_uiSamplerCount = 0;
  m_bInvalidated = false;
  return EZ_SUCCESS;
}

void ezGALBindGroupDX12::Invalidate(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);
  m_bInvalidated = true;
}

bool ezGALBindGroupDX12::IsInvalidated() const
{
  return m_bInvalidated;
}

void ezGALBindGroupDX12::SetDebugNamePlatform(const char* szName) const
{
  EZ_IGNORE_UNUSED(szName);
}
