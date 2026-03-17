#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/CommandEncoder/CommandEncoderImplDX12.h>
#include <RendererDX12/Pools/CommandListPoolDX12.h>
#include <RendererDX12/Pools/DescriptorHeapPoolDX12.h>
#include <RendererDX12/Pools/FencePoolDX12.h>
#include <RendererDX12/Pools/QueryPoolDX12.h>
#include <RendererDX12/Pools/StagingBufferPoolDX12.h>
#include <RendererDX12/Pools/UniformBufferPoolDX12.h>
#include <RendererDX12/Resources/BufferDX12.h>
#include <RendererDX12/Resources/ReadbackBufferDX12.h>
#include <RendererDX12/Resources/ReadbackTextureDX12.h>
#include <RendererDX12/Resources/RenderTargetViewDX12.h>
#include <RendererDX12/Resources/TextureDX12.h>
#include <RendererDX12/Shader/BindGroupDX12.h>
#include <RendererDX12/Shader/BindGroupLayoutDX12.h>
#include <RendererDX12/Shader/PipelineLayoutDX12.h>
#include <RendererDX12/Shader/ShaderDX12.h>
#include <RendererDX12/State/ComputePipelineDX12.h>
#include <RendererDX12/State/GraphicsPipelineDX12.h>
#include <RendererDX12/State/StateDX12.h>
#include <RendererDX12/Utils/ConversionUtilsDX12.h>
#include <RendererFoundation/CommandEncoder/CommandEncoder.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Shader/BindGroup.h>

#include <Foundation/Strings/StringConversion.h>

// PIX event metadata constants for BeginEvent/EndEvent/SetMarker.
// See: https://devblogs.microsoft.com/pix/winpixeventruntime/
#define PIX_EVENT_UNICODE_VERSION 0x08000002

// The DeviceDX12 header is included here. It must expose these accessor methods:
//   ID3D12Device* GetDXDevice() const;
//   ID3D12CommandQueue* GetCommandQueue() const;
//   ezCommandListPoolDX12& GetCommandListPool();
//   ezDescriptorHeapPoolDX12& GetDescriptorHeapPool();
//   ezQueryPoolDX12& GetQueryPool();
//   ezFenceQueueDX12& GetFenceQueue();
//   ezStagingBufferPoolDX12& GetStagingBufferPool();
//   ezUniformBufferPoolDX12& GetUniformBufferPool();
//   ID3D12CommandSignature* GetDrawIndirectSignature() const;
//   ID3D12CommandSignature* GetDrawIndexedIndirectSignature() const;
//   ID3D12CommandSignature* GetDispatchIndirectSignature() const;
#include <RendererDX12/Device/DeviceDX12.h>

// Helper to compute D3D12 subresource index, mirroring D3D12CalcSubresource.
static inline UINT CalcSubresource(UINT mipSlice, UINT arraySlice, UINT mipLevels)
{
  return mipSlice + arraySlice * mipLevels;
}

ezGALCommandEncoderImplDX12::ezGALCommandEncoderImplDX12(ezGALDeviceDX12& ref_deviceDX12)
  : m_Device(ref_deviceDX12)
{
}

ezGALCommandEncoderImplDX12::~ezGALCommandEncoderImplDX12()
{
  EZ_ASSERT_DEV(m_pCommandList == nullptr, "Command list was not properly ended");
}

void ezGALCommandEncoderImplDX12::BeginCommands(const char* szName)
{
  EZ_ASSERT_DEV(m_pCommandList == nullptr, "BeginCommands called while already recording");

  m_pClosedCommandList = nullptr;

  ezCommandListDX12 cmdList = m_Device.GetCommandListPool().RequestCommandList();
  m_pCommandList = cmdList.m_pCommandList;

  m_BarrierTracker.SetCommandList(m_pCommandList);

  SetDescriptorHeaps();

  if (szName != nullptr && szName[0] != '\0')
  {
    PushMarkerPlatform(szName);
    m_bHasCommandListMarker = true;
  }
  else
  {
    m_bHasCommandListMarker = false;
  }

  m_bInsideRendering = false;
  m_bInsideCompute = false;
  m_pCurrentPipelineLayout = nullptr;
  m_bGraphicsPipeline = false;
  m_CurrentTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
  m_bIndexBufferDirty = false;
  m_bVertexBuffersDirty = false;
}

void ezGALCommandEncoderImplDX12::EndCommands()
{
  EZ_ASSERT_DEV(m_pCommandList != nullptr, "EndCommands called without BeginCommands");

  if (m_bHasCommandListMarker)
  {
    PopMarkerPlatform();
    m_bHasCommandListMarker = false;
  }

  m_BarrierTracker.Flush();

  HRESULT hr = m_pCommandList->Close();
  EZ_ASSERT_DEV(SUCCEEDED(hr), "Failed to close command list: 0x{0}", ezArgU(hr, 8, true, 16));

  m_pClosedCommandList = m_pCommandList;
  m_pCommandList = nullptr;
  m_BarrierTracker.Reset();
}

void ezGALCommandEncoderImplDX12::EndFrame()
{
  // Per-frame cleanup. Nothing to reclaim yet.
}

void ezGALCommandEncoderImplDX12::SetDescriptorHeaps()
{
  ezDescriptorHeapPoolDX12& heapPool = m_Device.GetDescriptorHeapPool();
  ID3D12DescriptorHeap* heaps[2] = {
    heapPool.GetSrvUavCbvHeap(),
    heapPool.GetSamplerHeap(),
  };

  ezUInt32 uiHeapCount = 0;
  if (heaps[0] != nullptr)
    uiHeapCount = 1;
  if (heaps[1] != nullptr)
    uiHeapCount = 2;

  if (uiHeapCount > 0)
  {
    m_pCommandList->SetDescriptorHeaps(uiHeapCount, heaps);
  }
}

// ============================================================================
// Resource Binding
// ============================================================================

void ezGALCommandEncoderImplDX12::SetBindGroupPlatform(ezUInt32 uiBindGroup, const ezGALBindGroupCreationDescription& bindGroup)
{
  if (m_pCurrentPipelineLayout == nullptr)
  {
    EZ_REPORT_FAILURE("Cannot set bind group without a pipeline bound");
    return;
  }

  const ezGALBindGroupLayoutDX12* pLayout = static_cast<const ezGALBindGroupLayoutDX12*>(m_Device.GetBindGroupLayout(bindGroup.m_hBindGroupLayout));
  EZ_ASSERT_DEV(pLayout != nullptr, "Invalid bind group layout");

  ezDescriptorHeapPoolDX12& heapPool = m_Device.GetDescriptorHeapPool();
  ID3D12Device* pDXDevice = m_Device.GetDXDevice();

  // Allocate transient descriptors for CBV/SRV/UAV
  const ezUInt32 uiSrvUavCbvCount = pLayout->GetSrvUavCbvCount();
  if (uiSrvUavCbvCount > 0)
  {
    ezDescriptorRangeDX12 range = heapPool.AllocateTransientSrvUavCbv(uiSrvUavCbvCount);

    // Build staging descriptors and copy into the transient range.
    // Walk the layout bindings in order and create descriptors.
    const auto& bindings = pLayout->GetDescription().m_ResourceBindings;
    ezUInt32 uiDescriptorIndex = 0;

    for (ezUInt32 i = 0; i < bindings.GetCount(); ++i)
    {
      const ezShaderResourceBinding& binding = bindings[i];
      const ezGALBindGroupItem& item = bindGroup.m_BindGroupItems[i];

      switch (binding.m_ResourceType)
      {
        case ezGALShaderResourceType::ConstantBuffer:
        {
          D3D12_CPU_DESCRIPTOR_HANDLE destHandle = {range.m_cpuStart.ptr + uiDescriptorIndex * heapPool.GetSrvUavCbvDescriptorSize()};

          if (!item.m_Flags.IsSet(ezGALBindGroupItemFlags::EmptyBinding))
          {
            const ezGALBufferDX12* pBuffer = static_cast<const ezGALBufferDX12*>(m_Device.GetBuffer(item.m_Buffer.m_hBuffer));
            if (pBuffer != nullptr)
            {
              D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
              cbvDesc.BufferLocation = pBuffer->GetDXResource()->GetGPUVirtualAddress();
              cbvDesc.SizeInBytes = (pBuffer->GetSize() + 255) & ~255u; // Align to 256 bytes
              pDXDevice->CreateConstantBufferView(&cbvDesc, destHandle);
            }
            else
            {
              // Create a null CBV
              D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
              cbvDesc.BufferLocation = 0;
              cbvDesc.SizeInBytes = 0;
              pDXDevice->CreateConstantBufferView(&cbvDesc, destHandle);
            }
          }
          else
          {
            // Empty binding: null CBV
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
            cbvDesc.BufferLocation = 0;
            cbvDesc.SizeInBytes = 0;
            pDXDevice->CreateConstantBufferView(&cbvDesc, destHandle);
          }
          uiDescriptorIndex++;
          break;
        }

        case ezGALShaderResourceType::Texture:
        {
          D3D12_CPU_DESCRIPTOR_HANDLE destHandle = {range.m_cpuStart.ptr + uiDescriptorIndex * heapPool.GetSrvUavCbvDescriptorSize()};

          if (!item.m_Flags.IsSet(ezGALBindGroupItemFlags::EmptyBinding))
          {
            const ezGALTextureDX12* pTexture = static_cast<const ezGALTextureDX12*>(m_Device.GetTexture(item.m_Texture.m_hTexture));
            if (pTexture != nullptr && pTexture->GetSRVDescriptor().ptr != 0)
            {
              pDXDevice->CopyDescriptorsSimple(1, destHandle, pTexture->GetSRVDescriptor(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            }
            else
            {
              // Null SRV — texture missing or has no SRV descriptor (e.g. back buffer)
              D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
              srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
              srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
              srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
              pDXDevice->CreateShaderResourceView(nullptr, &srvDesc, destHandle);
            }
          }
          else
          {
            // Empty binding: null SRV
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            pDXDevice->CreateShaderResourceView(nullptr, &srvDesc, destHandle);
          }
          uiDescriptorIndex++;
          break;
        }

        case ezGALShaderResourceType::TextureRW:
        {
          D3D12_CPU_DESCRIPTOR_HANDLE destHandle = {range.m_cpuStart.ptr + uiDescriptorIndex * heapPool.GetSrvUavCbvDescriptorSize()};

          if (!item.m_Flags.IsSet(ezGALBindGroupItemFlags::EmptyBinding))
          {
            const ezGALTextureDX12* pTexture = static_cast<const ezGALTextureDX12*>(m_Device.GetTexture(item.m_Texture.m_hTexture));
            if (pTexture != nullptr)
            {
              // Use the texture's UAV descriptor if available; otherwise create a null UAV
              // Textures may not always have a pre-created UAV descriptor, so this may need to be extended.
              D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
              uavDesc.Format = pTexture->GetDXGIFormat();
              uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
              uavDesc.Texture2D.MipSlice = item.m_Texture.m_TextureRange.m_uiBaseMipLevel;
              pDXDevice->CreateUnorderedAccessView(pTexture->GetDXResource(), nullptr, &uavDesc, destHandle);
            }
            else
            {
              D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
              uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
              uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
              pDXDevice->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, destHandle);
            }
          }
          else
          {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            pDXDevice->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, destHandle);
          }
          uiDescriptorIndex++;
          break;
        }

        case ezGALShaderResourceType::TexelBuffer:
        case ezGALShaderResourceType::StructuredBuffer:
        case ezGALShaderResourceType::ByteAddressBuffer:
        {
          D3D12_CPU_DESCRIPTOR_HANDLE destHandle = {range.m_cpuStart.ptr + uiDescriptorIndex * heapPool.GetSrvUavCbvDescriptorSize()};

          if (!item.m_Flags.IsSet(ezGALBindGroupItemFlags::EmptyBinding))
          {
            const ezGALBufferDX12* pBuffer = static_cast<const ezGALBufferDX12*>(m_Device.GetBuffer(item.m_Buffer.m_hBuffer));
            if (pBuffer != nullptr && pBuffer->GetSRVDescriptor().ptr != 0)
            {
              pDXDevice->CopyDescriptorsSimple(1, destHandle, pBuffer->GetSRVDescriptor(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            }
            else
            {
              D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
              srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
              srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
              srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
              srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
              pDXDevice->CreateShaderResourceView(nullptr, &srvDesc, destHandle);
            }
          }
          else
          {
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
            pDXDevice->CreateShaderResourceView(nullptr, &srvDesc, destHandle);
          }
          uiDescriptorIndex++;
          break;
        }

        case ezGALShaderResourceType::TexelBufferRW:
        case ezGALShaderResourceType::StructuredBufferRW:
        case ezGALShaderResourceType::ByteAddressBufferRW:
        {
          D3D12_CPU_DESCRIPTOR_HANDLE destHandle = {range.m_cpuStart.ptr + uiDescriptorIndex * heapPool.GetSrvUavCbvDescriptorSize()};

          if (!item.m_Flags.IsSet(ezGALBindGroupItemFlags::EmptyBinding))
          {
            const ezGALBufferDX12* pBuffer = static_cast<const ezGALBufferDX12*>(m_Device.GetBuffer(item.m_Buffer.m_hBuffer));
            if (pBuffer != nullptr && pBuffer->GetUAVDescriptor().ptr != 0)
            {
              pDXDevice->CopyDescriptorsSimple(1, destHandle, pBuffer->GetUAVDescriptor(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            }
            else
            {
              D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
              uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
              uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
              uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
              pDXDevice->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, destHandle);
            }
          }
          else
          {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
            pDXDevice->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, destHandle);
          }
          uiDescriptorIndex++;
          break;
        }

        case ezGALShaderResourceType::Sampler:
          // Samplers go into the separate sampler table, not counted here.
          break;

        case ezGALShaderResourceType::TextureAndSampler:
        default:
          break;
      }
    }

    // Set the descriptor table for CBV/SRV/UAV
    ezInt8 iRootParam = m_pCurrentPipelineLayout->GetSrvUavCbvRootParamIndex(uiBindGroup);
    if (iRootParam >= 0)
    {
      if (m_bGraphicsPipeline)
        m_pCommandList->SetGraphicsRootDescriptorTable(iRootParam, range.m_gpuStart);
      else
        m_pCommandList->SetComputeRootDescriptorTable(iRootParam, range.m_gpuStart);
    }
  }

  // Allocate transient descriptors for Samplers
  const ezUInt32 uiSamplerCount = pLayout->GetSamplerCount();
  if (uiSamplerCount > 0)
  {
    ezDescriptorRangeDX12 samplerRange = heapPool.AllocateTransientSampler(uiSamplerCount);
    const auto& bindings = pLayout->GetDescription().m_ResourceBindings;
    ezUInt32 uiSamplerIndex = 0;

    for (ezUInt32 i = 0; i < bindings.GetCount(); ++i)
    {
      const ezShaderResourceBinding& binding = bindings[i];
      const ezGALBindGroupItem& item = bindGroup.m_BindGroupItems[i];

      if (binding.m_ResourceType == ezGALShaderResourceType::Sampler)
      {
        D3D12_CPU_DESCRIPTOR_HANDLE destHandle = {samplerRange.m_cpuStart.ptr + uiSamplerIndex * heapPool.GetSamplerDescriptorSize()};

        const ezGALSamplerStateDX12* pSampler = static_cast<const ezGALSamplerStateDX12*>(m_Device.GetSamplerState(item.m_Sampler.m_hSampler));
        if (pSampler != nullptr)
        {
          pDXDevice->CopyDescriptorsSimple(1, destHandle, pSampler->GetSamplerDescriptor(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        }
        else
        {
          // Create a default sampler
          D3D12_SAMPLER_DESC defaultSampler = {};
          defaultSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
          defaultSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
          defaultSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
          defaultSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
          defaultSampler.MaxLOD = D3D12_FLOAT32_MAX;
          pDXDevice->CreateSampler(&defaultSampler, destHandle);
        }
        uiSamplerIndex++;
      }
    }

    ezInt8 iSamplerRootParam = m_pCurrentPipelineLayout->GetSamplerRootParamIndex(uiBindGroup);
    if (iSamplerRootParam >= 0)
    {
      if (m_bGraphicsPipeline)
        m_pCommandList->SetGraphicsRootDescriptorTable(iSamplerRootParam, samplerRange.m_gpuStart);
      else
        m_pCommandList->SetComputeRootDescriptorTable(iSamplerRootParam, samplerRange.m_gpuStart);
    }
  }
}

void ezGALCommandEncoderImplDX12::SetBindGroupPlatform(ezUInt32 uiBindGroup, const ezGALBindGroup* pBindGroup)
{
  if (m_pCurrentPipelineLayout == nullptr)
  {
    EZ_REPORT_FAILURE("Cannot set bind group without a pipeline bound");
    return;
  }

  const ezGALBindGroupDX12* pDX12BindGroup = static_cast<const ezGALBindGroupDX12*>(pBindGroup);

  // Set pre-built CBV/SRV/UAV descriptor table
  if (pDX12BindGroup->GetSrvUavCbvCount() > 0)
  {
    ezInt8 iRootParam = m_pCurrentPipelineLayout->GetSrvUavCbvRootParamIndex(uiBindGroup);
    if (iRootParam >= 0)
    {
      if (m_bGraphicsPipeline)
        m_pCommandList->SetGraphicsRootDescriptorTable(iRootParam, pDX12BindGroup->GetSrvUavCbvTableStart());
      else
        m_pCommandList->SetComputeRootDescriptorTable(iRootParam, pDX12BindGroup->GetSrvUavCbvTableStart());
    }
  }

  // Set pre-built sampler descriptor table
  if (pDX12BindGroup->GetSamplerCount() > 0)
  {
    ezInt8 iSamplerRootParam = m_pCurrentPipelineLayout->GetSamplerRootParamIndex(uiBindGroup);
    if (iSamplerRootParam >= 0)
    {
      if (m_bGraphicsPipeline)
        m_pCommandList->SetGraphicsRootDescriptorTable(iSamplerRootParam, pDX12BindGroup->GetSamplerTableStart());
      else
        m_pCommandList->SetComputeRootDescriptorTable(iSamplerRootParam, pDX12BindGroup->GetSamplerTableStart());
    }
  }
}

void ezGALCommandEncoderImplDX12::SetPushConstantsPlatform(ezArrayPtr<const ezUInt8> data)
{
  if (m_pCurrentPipelineLayout == nullptr)
  {
    EZ_REPORT_FAILURE("Cannot set push constants without a pipeline bound");
    return;
  }

  ezInt8 iRootParam = m_pCurrentPipelineLayout->GetPushConstantsRootParamIndex();
  if (iRootParam < 0)
  {
    EZ_REPORT_FAILURE("Current pipeline layout has no push constant root parameter");
    return;
  }

  // Push constants are specified in 32-bit values.
  const ezUInt32 uiNum32BitValues = (data.GetCount() + 3) / 4;
  EZ_ASSERT_DEV(data.GetCount() % 4 == 0, "Push constant data size must be a multiple of 4 bytes");

  if (m_bGraphicsPipeline)
  {
    m_pCommandList->SetGraphicsRoot32BitConstants(iRootParam, uiNum32BitValues, data.GetPtr(), 0);
  }
  else
  {
    m_pCommandList->SetComputeRoot32BitConstants(iRootParam, uiNum32BitValues, data.GetPtr(), 0);
  }
}

// ============================================================================
// Query Functions
// ============================================================================

ezGALTimestampHandle ezGALCommandEncoderImplDX12::InsertTimestampPlatform()
{
  ezQueryPoolDX12& queryPool = m_Device.GetQueryPool();
  ezUInt32 uiIndex = 0;
  queryPool.InsertTimestamp(m_pCommandList, uiIndex);

  // Construct a pool handle from the index. Generation is left at 0 as the pool manages
  // validity through frame-based resets.
  return ezGALTimestampHandle(uiIndex, 0);
}

ezGALOcclusionHandle ezGALCommandEncoderImplDX12::BeginOcclusionQueryPlatform(ezEnum<ezGALQueryType> type)
{
  ezQueryPoolDX12& queryPool = m_Device.GetQueryPool();
  ezUInt32 uiIndex = 0;
  bool bBinary = (type == ezGALQueryType::AnySamplesPassed);
  queryPool.BeginOcclusionQuery(m_pCommandList, uiIndex, bBinary);

  // Store the binary flag in the generation bits for retrieval in EndOcclusionQuery.
  return ezGALOcclusionHandle(uiIndex, bBinary ? 1 : 0);
}

void ezGALCommandEncoderImplDX12::EndOcclusionQueryPlatform(ezGALOcclusionHandle hOcclusion)
{
  ezQueryPoolDX12& queryPool = m_Device.GetQueryPool();
  bool bBinary = (hOcclusion.m_Generation != 0);
  queryPool.EndOcclusionQuery(m_pCommandList, static_cast<ezUInt32>(hOcclusion.m_InstanceIndex), bBinary);
}

ezGALFenceHandle ezGALCommandEncoderImplDX12::InsertFencePlatform()
{
  return m_Device.GetFenceQueue().GetCurrentFenceHandle();
}

// ============================================================================
// Resource Copy/Update
// ============================================================================

void ezGALCommandEncoderImplDX12::CopyBufferPlatform(const ezGALBuffer* pDestination, const ezGALBuffer* pSource)
{
  const ezGALBufferDX12* pDstBuffer = static_cast<const ezGALBufferDX12*>(pDestination);
  const ezGALBufferDX12* pSrcBuffer = static_cast<const ezGALBufferDX12*>(pSource);

  // Transition source to copy source and destination to copy dest
  m_BarrierTracker.BufferBarrier(pSrcBuffer->GetDXResource(), pSrcBuffer->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_SOURCE);
  m_BarrierTracker.BufferBarrier(pDstBuffer->GetDXResource(), pDstBuffer->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_DEST);
  m_BarrierTracker.Flush();

  m_pCommandList->CopyResource(pDstBuffer->GetDXResource(), pSrcBuffer->GetDXResource());

  // Transition back to previous states
  m_BarrierTracker.BufferBarrier(pSrcBuffer->GetDXResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, pSrcBuffer->GetCurrentState());
  m_BarrierTracker.BufferBarrier(pDstBuffer->GetDXResource(), D3D12_RESOURCE_STATE_COPY_DEST, pDstBuffer->GetCurrentState());
}

void ezGALCommandEncoderImplDX12::CopyBufferRegionPlatform(const ezGALBuffer* pDestination, ezUInt32 uiDestOffset, const ezGALBuffer* pSource, ezUInt32 uiSourceOffset, ezUInt32 uiByteCount)
{
  const ezGALBufferDX12* pDstBuffer = static_cast<const ezGALBufferDX12*>(pDestination);
  const ezGALBufferDX12* pSrcBuffer = static_cast<const ezGALBufferDX12*>(pSource);

  m_BarrierTracker.BufferBarrier(pSrcBuffer->GetDXResource(), pSrcBuffer->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_SOURCE);
  m_BarrierTracker.BufferBarrier(pDstBuffer->GetDXResource(), pDstBuffer->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_DEST);
  m_BarrierTracker.Flush();

  m_pCommandList->CopyBufferRegion(pDstBuffer->GetDXResource(), uiDestOffset, pSrcBuffer->GetDXResource(), uiSourceOffset, uiByteCount);

  m_BarrierTracker.BufferBarrier(pSrcBuffer->GetDXResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, pSrcBuffer->GetCurrentState());
  m_BarrierTracker.BufferBarrier(pDstBuffer->GetDXResource(), D3D12_RESOURCE_STATE_COPY_DEST, pDstBuffer->GetCurrentState());
}

void ezGALCommandEncoderImplDX12::UpdateBufferPlatform(const ezGALBuffer* pDestination, ezUInt32 uiDestOffset, ezArrayPtr<const ezUInt8> sourceData, ezGALUpdateMode::Enum updateMode)
{
  EZ_CHECK_ALIGNMENT(sourceData.GetPtr(), 16);

  const ezGALBufferDX12* pDstBuffer = static_cast<const ezGALBufferDX12*>(pDestination);

  // Upload-heap buffers (constant buffers) cannot be the destination of a GPU copy
  // and cannot be transitioned — they must remain in GENERIC_READ. Map and copy directly.
  if (pDstBuffer->GetCurrentState() == D3D12_RESOURCE_STATE_GENERIC_READ)
  {
    void* pMappedData = nullptr;
    D3D12_RANGE readRange = {0, 0};
    HRESULT hr = pDstBuffer->GetDXResource()->Map(0, &readRange, &pMappedData);
    if (SUCCEEDED(hr))
    {
      ezMemoryUtils::RawByteCopy(static_cast<ezUInt8*>(pMappedData) + uiDestOffset, sourceData.GetPtr(), sourceData.GetCount());
      D3D12_RANGE writeRange = {uiDestOffset, uiDestOffset + sourceData.GetCount()};
      pDstBuffer->GetDXResource()->Unmap(0, &writeRange);
    }
    return;
  }

  if (updateMode == ezGALUpdateMode::TransientConstantBuffer)
  {
    // For transient constant buffers on default heap, allocate from the uniform buffer pool and copy.
    ezUniformBufferPoolDX12& uniformPool = m_Device.GetUniformBufferPool();
    auto allocation = uniformPool.Allocate(sourceData.GetCount());

    memcpy(allocation.m_pMappedData, sourceData.GetPtr(), sourceData.GetCount());

    m_BarrierTracker.BufferBarrier(pDstBuffer->GetDXResource(), pDstBuffer->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_DEST);
    m_BarrierTracker.Flush();

    m_pCommandList->CopyBufferRegion(pDstBuffer->GetDXResource(), uiDestOffset, allocation.m_pBuffer, allocation.m_uiOffset, sourceData.GetCount());

    m_BarrierTracker.BufferBarrier(pDstBuffer->GetDXResource(), D3D12_RESOURCE_STATE_COPY_DEST, pDstBuffer->GetCurrentState());
  }
  else
  {
    // AheadOfTime: Use a staging buffer from the upload heap.
    ezStagingBufferPoolDX12& stagingPool = m_Device.GetStagingBufferPool();
    ezStagingBufferDX12 staging = stagingPool.AllocateBuffer(sourceData.GetCount(), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

    memcpy(staging.m_pMappedData, sourceData.GetPtr(), sourceData.GetCount());

    m_BarrierTracker.BufferBarrier(pDstBuffer->GetDXResource(), pDstBuffer->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_DEST);
    m_BarrierTracker.Flush();

    m_pCommandList->CopyBufferRegion(pDstBuffer->GetDXResource(), uiDestOffset, staging.m_pBuffer, staging.m_uiOffset, sourceData.GetCount());

    m_BarrierTracker.BufferBarrier(pDstBuffer->GetDXResource(), D3D12_RESOURCE_STATE_COPY_DEST, pDstBuffer->GetCurrentState());
  }
}

void ezGALCommandEncoderImplDX12::CopyTexturePlatform(const ezGALTexture* pDestination, const ezGALTexture* pSource)
{
  const ezGALTextureDX12* pDstTexture = static_cast<const ezGALTextureDX12*>(pDestination);
  const ezGALTextureDX12* pSrcTexture = static_cast<const ezGALTextureDX12*>(pSource);

  m_BarrierTracker.TextureBarrier(pSrcTexture->GetDXResource(), pSrcTexture->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_SOURCE);
  m_BarrierTracker.TextureBarrier(pDstTexture->GetDXResource(), pDstTexture->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_DEST);
  m_BarrierTracker.Flush();

  m_pCommandList->CopyResource(pDstTexture->GetDXResource(), pSrcTexture->GetDXResource());

  m_BarrierTracker.TextureBarrier(pSrcTexture->GetDXResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, pSrcTexture->GetCurrentState());
  m_BarrierTracker.TextureBarrier(pDstTexture->GetDXResource(), D3D12_RESOURCE_STATE_COPY_DEST, pDstTexture->GetCurrentState());
}

void ezGALCommandEncoderImplDX12::CopyTextureRegionPlatform(const ezGALTexture* pDestination, const ezGALTextureSubresource& destinationSubResource,
  const ezVec3U32& vDestinationPoint, const ezGALTexture* pSource, const ezGALTextureSubresource& sourceSubResource, const ezBoundingBoxu32& box)
{
  const ezGALTextureDX12* pDstTexture = static_cast<const ezGALTextureDX12*>(pDestination);
  const ezGALTextureDX12* pSrcTexture = static_cast<const ezGALTextureDX12*>(pSource);

  m_BarrierTracker.TextureBarrier(pSrcTexture->GetDXResource(), pSrcTexture->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_SOURCE);
  m_BarrierTracker.TextureBarrier(pDstTexture->GetDXResource(), pDstTexture->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_DEST);
  m_BarrierTracker.Flush();

  UINT dstSubresource = CalcSubresource(destinationSubResource.m_uiMipLevel, destinationSubResource.m_uiArraySlice, pDestination->GetDescription().m_uiMipLevelCount);
  UINT srcSubresource = CalcSubresource(sourceSubResource.m_uiMipLevel, sourceSubResource.m_uiArraySlice, pSource->GetDescription().m_uiMipLevelCount);

  D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
  dstLocation.pResource = pDstTexture->GetDXResource();
  dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  dstLocation.SubresourceIndex = dstSubresource;

  D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
  srcLocation.pResource = pSrcTexture->GetDXResource();
  srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  srcLocation.SubresourceIndex = srcSubresource;

  D3D12_BOX srcBox = {};
  srcBox.left = box.m_vMin.x;
  srcBox.top = box.m_vMin.y;
  srcBox.front = box.m_vMin.z;
  srcBox.right = box.m_vMax.x;
  srcBox.bottom = box.m_vMax.y;
  srcBox.back = box.m_vMax.z;

  m_pCommandList->CopyTextureRegion(&dstLocation, vDestinationPoint.x, vDestinationPoint.y, vDestinationPoint.z, &srcLocation, &srcBox);

  m_BarrierTracker.TextureBarrier(pSrcTexture->GetDXResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, pSrcTexture->GetCurrentState());
  m_BarrierTracker.TextureBarrier(pDstTexture->GetDXResource(), D3D12_RESOURCE_STATE_COPY_DEST, pDstTexture->GetCurrentState());
}

void ezGALCommandEncoderImplDX12::UpdateTexturePlatform(const ezGALTexture* pDestination, const ezGALTextureSubresource& destinationSubResource,
  const ezBoundingBoxu32& destinationBox, const ezGALSystemMemoryDescription& sourceData)
{
  const ezGALTextureDX12* pDstTexture = static_cast<const ezGALTextureDX12*>(pDestination);

  ezUInt32 uiWidth = ezMath::Max(destinationBox.m_vMax.x - destinationBox.m_vMin.x, 1u);
  ezUInt32 uiHeight = ezMath::Max(destinationBox.m_vMax.y - destinationBox.m_vMin.y, 1u);
  ezUInt32 uiDepth = ezMath::Max(destinationBox.m_vMax.z - destinationBox.m_vMin.z, 1u);

  // Calculate the required staging buffer size.
  // Row pitch must be aligned to D3D12_TEXTURE_DATA_PITCH_ALIGNMENT (256 bytes).
  const ezUInt32 uiSrcRowPitch = sourceData.m_uiRowPitch;
  const ezUInt32 uiAlignedRowPitch = (uiSrcRowPitch + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
  const ezUInt64 uiStagingSize = (ezUInt64)uiAlignedRowPitch * uiHeight * uiDepth;

  ezStagingBufferPoolDX12& stagingPool = m_Device.GetStagingBufferPool();
  ezStagingBufferDX12 staging = stagingPool.AllocateBuffer(uiStagingSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

  // Copy data row by row to respect the aligned row pitch.
  const ezUInt8* pSrcData = sourceData.m_pData.GetPtr();
  ezUInt8* pDstData = static_cast<ezUInt8*>(staging.m_pMappedData);

  for (ezUInt32 z = 0; z < uiDepth; ++z)
  {
    for (ezUInt32 y = 0; y < uiHeight; ++y)
    {
      memcpy(pDstData + (z * uiHeight + y) * uiAlignedRowPitch,
        pSrcData + z * sourceData.m_uiSlicePitch + y * uiSrcRowPitch,
        uiSrcRowPitch);
    }
  }

  m_BarrierTracker.TextureBarrier(pDstTexture->GetDXResource(), pDstTexture->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_DEST);
  m_BarrierTracker.Flush();

  UINT dstSubresource = CalcSubresource(destinationSubResource.m_uiMipLevel, destinationSubResource.m_uiArraySlice, pDestination->GetDescription().m_uiMipLevelCount);

  D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
  dstLocation.pResource = pDstTexture->GetDXResource();
  dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  dstLocation.SubresourceIndex = dstSubresource;

  D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
  srcLocation.pResource = staging.m_pBuffer;
  srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  srcLocation.PlacedFootprint.Offset = staging.m_uiOffset;
  srcLocation.PlacedFootprint.Footprint.Format = pDstTexture->GetDXGIFormat();
  srcLocation.PlacedFootprint.Footprint.Width = uiWidth;
  srcLocation.PlacedFootprint.Footprint.Height = uiHeight;
  srcLocation.PlacedFootprint.Footprint.Depth = uiDepth;
  srcLocation.PlacedFootprint.Footprint.RowPitch = uiAlignedRowPitch;

  D3D12_BOX srcBox = {};
  srcBox.left = 0;
  srcBox.top = 0;
  srcBox.front = 0;
  srcBox.right = uiWidth;
  srcBox.bottom = uiHeight;
  srcBox.back = uiDepth;

  m_pCommandList->CopyTextureRegion(&dstLocation, destinationBox.m_vMin.x, destinationBox.m_vMin.y, destinationBox.m_vMin.z, &srcLocation, &srcBox);

  m_BarrierTracker.TextureBarrier(pDstTexture->GetDXResource(), D3D12_RESOURCE_STATE_COPY_DEST, pDstTexture->GetCurrentState());
}

void ezGALCommandEncoderImplDX12::ResolveTexturePlatform(const ezGALTexture* pDestination, const ezGALTextureSubresource& destinationSubResource,
  const ezGALTexture* pSource, const ezGALTextureSubresource& sourceSubResource)
{
  const ezGALTextureDX12* pDstTexture = static_cast<const ezGALTextureDX12*>(pDestination);
  const ezGALTextureDX12* pSrcTexture = static_cast<const ezGALTextureDX12*>(pSource);

  UINT dstSubresource = CalcSubresource(destinationSubResource.m_uiMipLevel, destinationSubResource.m_uiArraySlice, pDestination->GetDescription().m_uiMipLevelCount);
  UINT srcSubresource = CalcSubresource(sourceSubResource.m_uiMipLevel, sourceSubResource.m_uiArraySlice, pSource->GetDescription().m_uiMipLevelCount);

  m_BarrierTracker.SubresourceBarrier(pSrcTexture->GetDXResource(), srcSubresource, pSrcTexture->GetCurrentState(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
  m_BarrierTracker.SubresourceBarrier(pDstTexture->GetDXResource(), dstSubresource, pDstTexture->GetCurrentState(), D3D12_RESOURCE_STATE_RESOLVE_DEST);
  m_BarrierTracker.Flush();

  DXGI_FORMAT resolveFormat = pDstTexture->GetDXGIFormat();
  m_pCommandList->ResolveSubresource(pDstTexture->GetDXResource(), dstSubresource, pSrcTexture->GetDXResource(), srcSubresource, resolveFormat);

  m_BarrierTracker.SubresourceBarrier(pSrcTexture->GetDXResource(), srcSubresource, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, pSrcTexture->GetCurrentState());
  m_BarrierTracker.SubresourceBarrier(pDstTexture->GetDXResource(), dstSubresource, D3D12_RESOURCE_STATE_RESOLVE_DEST, pDstTexture->GetCurrentState());
}

void ezGALCommandEncoderImplDX12::ReadbackTexturePlatform(const ezGALReadbackTexture* pDestination, const ezGALTexture* pSource)
{
  const ezGALReadbackTextureDX12* pDstReadback = static_cast<const ezGALReadbackTextureDX12*>(pDestination);
  const ezGALTextureDX12* pSrcTexture = static_cast<const ezGALTextureDX12*>(pSource);

  EZ_ASSERT_DEV(pSrcTexture->GetDescription().m_SampleCount == ezGALMSAASampleCount::None, "MSAA readback is not supported");

  m_BarrierTracker.TextureBarrier(pSrcTexture->GetDXResource(), pSrcTexture->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_SOURCE);
  m_BarrierTracker.Flush();

  // Copy from the GPU texture into the readback buffer.
  // The readback resource is a buffer in a readback heap, so we use placed footprint layout.
  D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
  srcLocation.pResource = pSrcTexture->GetDXResource();
  srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  srcLocation.SubresourceIndex = 0;

  D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
  dstLocation.pResource = pDstReadback->GetDXResource();
  dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

  // Query the device for the footprint of the source texture.
  D3D12_RESOURCE_DESC srcDesc = pSrcTexture->GetDXResource()->GetDesc();
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
  UINT64 uiTotalBytes = 0;
  m_Device.GetDXDevice()->GetCopyableFootprints(&srcDesc, 0, 1, 0, &footprint, nullptr, nullptr, &uiTotalBytes);

  dstLocation.PlacedFootprint = footprint;

  m_pCommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

  m_BarrierTracker.TextureBarrier(pSrcTexture->GetDXResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, pSrcTexture->GetCurrentState());
}

void ezGALCommandEncoderImplDX12::ReadbackBufferPlatform(const ezGALReadbackBuffer* pDestination, const ezGALBuffer* pSource)
{
  const ezGALReadbackBufferDX12* pDstReadback = static_cast<const ezGALReadbackBufferDX12*>(pDestination);
  const ezGALBufferDX12* pSrcBuffer = static_cast<const ezGALBufferDX12*>(pSource);

  m_BarrierTracker.BufferBarrier(pSrcBuffer->GetDXResource(), pSrcBuffer->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_SOURCE);
  m_BarrierTracker.Flush();

  m_pCommandList->CopyResource(pDstReadback->GetDXResource(), pSrcBuffer->GetDXResource());

  m_BarrierTracker.BufferBarrier(pSrcBuffer->GetDXResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, pSrcBuffer->GetCurrentState());
}

void ezGALCommandEncoderImplDX12::GenerateMipMapsPlatform(const ezGALTexture* pTexture, ezGALTextureRange range)
{
  EZ_IGNORE_UNUSED(pTexture);
  EZ_IGNORE_UNUSED(range);

  // D3D12 does not have a built-in GenerateMips like D3D11. This requires a compute shader pass
  // that reads from mip N and writes to mip N+1 iteratively.
  EZ_ASSERT_NOT_IMPLEMENTED;
}

// ============================================================================
// Misc
// ============================================================================

void ezGALCommandEncoderImplDX12::FlushPlatform()
{
  m_BarrierTracker.Flush();
}

// ============================================================================
// Debug Markers
// ============================================================================

void ezGALCommandEncoderImplDX12::PushMarkerPlatform(const char* szMarker)
{
  if (m_pCommandList != nullptr)
  {
    ezStringWChar wsMarker(szMarker);
    m_pCommandList->BeginEvent(PIX_EVENT_UNICODE_VERSION, wsMarker.GetData(), static_cast<UINT>((wsMarker.GetElementCount() + 1) * sizeof(wchar_t)));
  }
}

void ezGALCommandEncoderImplDX12::PopMarkerPlatform()
{
  if (m_pCommandList != nullptr)
  {
    m_pCommandList->EndEvent();
  }
}

void ezGALCommandEncoderImplDX12::InsertEventMarkerPlatform(const char* szMarker)
{
  if (m_pCommandList != nullptr)
  {
    ezStringWChar wsMarker(szMarker);
    m_pCommandList->SetMarker(PIX_EVENT_UNICODE_VERSION, wsMarker.GetData(), static_cast<UINT>((wsMarker.GetElementCount() + 1) * sizeof(wchar_t)));
  }
}

// ============================================================================
// Compute
// ============================================================================

void ezGALCommandEncoderImplDX12::BeginComputePlatform()
{
  m_bInsideCompute = true;
  m_bInsideRendering = false;
}

void ezGALCommandEncoderImplDX12::EndComputePlatform()
{
  m_bInsideCompute = false;
}

ezResult ezGALCommandEncoderImplDX12::DispatchPlatform(ezUInt32 uiThreadGroupCountX, ezUInt32 uiThreadGroupCountY, ezUInt32 uiThreadGroupCountZ)
{
  if (m_pCurrentPipelineLayout == nullptr)
    return EZ_FAILURE;

  FlushDeferredStateChanges();
  m_BarrierTracker.Flush();

  m_pCommandList->Dispatch(uiThreadGroupCountX, uiThreadGroupCountY, uiThreadGroupCountZ);
  return EZ_SUCCESS;
}

ezResult ezGALCommandEncoderImplDX12::DispatchIndirectPlatform(const ezGALBuffer* pIndirectArgumentBuffer, ezUInt32 uiArgumentOffsetInBytes)
{
  if (m_pCurrentPipelineLayout == nullptr)
    return EZ_FAILURE;

  FlushDeferredStateChanges();

  const ezGALBufferDX12* pBuffer = static_cast<const ezGALBufferDX12*>(pIndirectArgumentBuffer);

  m_BarrierTracker.BufferBarrier(pBuffer->GetDXResource(), pBuffer->GetCurrentState(), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
  m_BarrierTracker.Flush();

  // ExecuteIndirect requires a command signature. For a single Dispatch, the device should provide one.
  // This uses the device's indirect dispatch command signature.
  m_pCommandList->ExecuteIndirect(m_Device.GetDispatchIndirectSignature(), 1, pBuffer->GetDXResource(), uiArgumentOffsetInBytes, nullptr, 0);

  m_BarrierTracker.BufferBarrier(pBuffer->GetDXResource(), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, pBuffer->GetCurrentState());

  return EZ_SUCCESS;
}

// ============================================================================
// Rendering
// ============================================================================

void ezGALCommandEncoderImplDX12::BeginRenderingPlatform(const ezGALRenderingSetup& renderingSetup)
{
  m_RenderTargetSetup = renderingSetup;
  m_bInsideRendering = true;
  m_bInsideCompute = false;
  m_CurrentRTVs.Clear();
  m_bHasDSV = false;

  const ezUInt32 uiRenderTargetCount = renderingSetup.GetColorTargetCount();

  for (ezUInt8 i = 0; i < uiRenderTargetCount; ++i)
  {
    const ezGALRenderTargetView* pRTV = m_Device.GetRenderTargetView(renderingSetup.GetFrameBuffer().m_hColorTarget[i]);
    if (pRTV != nullptr)
    {
      const ezGALRenderTargetViewDX12* pRTVDX12 = static_cast<const ezGALRenderTargetViewDX12*>(pRTV);

      // Transition the render target texture to render target state and update tracked state.
      ezGALTextureDX12* pTexture = const_cast<ezGALTextureDX12*>(static_cast<const ezGALTextureDX12*>(pRTV->GetTexture()));
      m_BarrierTracker.TextureBarrier(pTexture->GetDXResource(), pTexture->GetCurrentState(), D3D12_RESOURCE_STATE_RENDER_TARGET);
      pTexture->SetCurrentState(D3D12_RESOURCE_STATE_RENDER_TARGET);

      m_CurrentRTVs.PushBack(pRTVDX12->GetDescriptor());
    }
    else
    {
      // Skip null render target slots — don't push zero descriptor handles.
      continue;
    }
  }

  if (renderingSetup.HasDepthStencilTarget())
  {
    const ezGALRenderTargetView* pDSV = m_Device.GetRenderTargetView(renderingSetup.GetFrameBuffer().m_hDepthTarget);
    if (pDSV != nullptr)
    {
      const ezGALRenderTargetViewDX12* pDSVDX12 = static_cast<const ezGALRenderTargetViewDX12*>(pDSV);

      ezGALTextureDX12* pTexture = const_cast<ezGALTextureDX12*>(static_cast<const ezGALTextureDX12*>(pDSV->GetTexture()));
      m_BarrierTracker.TextureBarrier(pTexture->GetDXResource(), pTexture->GetCurrentState(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
      pTexture->SetCurrentState(D3D12_RESOURCE_STATE_DEPTH_WRITE);

      m_CurrentDSV = pDSVDX12->GetDescriptor();
      m_bHasDSV = true;
    }
  }

  m_BarrierTracker.Flush();

  // Set the render targets on the command list.
  m_pCommandList->OMSetRenderTargets(
    m_CurrentRTVs.GetCount(),
    m_CurrentRTVs.IsEmpty() ? nullptr : m_CurrentRTVs.GetData(),
    FALSE,
    m_bHasDSV ? &m_CurrentDSV : nullptr);

  // Handle load ops (clear on begin).
  const ezGALRenderPassDescriptor& renderPass = renderingSetup.GetRenderPass();

  for (ezUInt8 i = 0; i < m_CurrentRTVs.GetCount(); ++i)
  {
    if (i < EZ_GAL_MAX_RENDERTARGET_COUNT && renderPass.m_ColorLoadOp[i] == ezGALRenderTargetLoadOp::Clear && m_CurrentRTVs[i].ptr != 0)
    {
      const ezColor& clearColor = renderingSetup.GetClearColor(i);
      m_pCommandList->ClearRenderTargetView(m_CurrentRTVs[i], clearColor.GetData(), 0, nullptr);
    }
  }

  bool bClearDepth = renderPass.m_DepthLoadOp == ezGALRenderTargetLoadOp::Clear;
  bool bClearStencil = renderPass.m_StencilLoadOp == ezGALRenderTargetLoadOp::Clear;
  if ((bClearDepth || bClearStencil) && m_bHasDSV)
  {
    D3D12_CLEAR_FLAGS clearFlags = static_cast<D3D12_CLEAR_FLAGS>(0);
    if (bClearDepth)
      clearFlags = static_cast<D3D12_CLEAR_FLAGS>(clearFlags | D3D12_CLEAR_FLAG_DEPTH);
    if (bClearStencil)
      clearFlags = static_cast<D3D12_CLEAR_FLAGS>(clearFlags | D3D12_CLEAR_FLAG_STENCIL);

    m_pCommandList->ClearDepthStencilView(m_CurrentDSV, clearFlags, renderingSetup.GetClearDepth(), renderingSetup.GetClearStencil(), 0, nullptr);
  }
}

void ezGALCommandEncoderImplDX12::EndRenderingPlatform()
{
  // Transition render targets back from their render target states.
  // This will be handled automatically by the next BeginRendering or by the barrier tracker
  // when the textures are used for something else. For now, just clear state.
  m_bInsideRendering = false;
  m_CurrentRTVs.Clear();
  m_bHasDSV = false;
}

void ezGALCommandEncoderImplDX12::ClearPlatform(const ezColor& clearColor, ezUInt32 uiRenderTargetClearMask, bool bClearDepth, bool bClearStencil, float fDepthClear, ezUInt8 uiStencilClear)
{
  for (ezUInt32 i = 0; i < m_CurrentRTVs.GetCount(); ++i)
  {
    if ((uiRenderTargetClearMask & (1u << i)) && m_CurrentRTVs[i].ptr != 0)
    {
      m_pCommandList->ClearRenderTargetView(m_CurrentRTVs[i], clearColor.GetData(), 0, nullptr);
    }
  }

  if ((bClearDepth || bClearStencil) && m_bHasDSV)
  {
    D3D12_CLEAR_FLAGS clearFlags = static_cast<D3D12_CLEAR_FLAGS>(0);
    if (bClearDepth)
      clearFlags = static_cast<D3D12_CLEAR_FLAGS>(clearFlags | D3D12_CLEAR_FLAG_DEPTH);
    if (bClearStencil)
      clearFlags = static_cast<D3D12_CLEAR_FLAGS>(clearFlags | D3D12_CLEAR_FLAG_STENCIL);

    m_pCommandList->ClearDepthStencilView(m_CurrentDSV, clearFlags, fDepthClear, uiStencilClear, 0, nullptr);
  }
}

// ============================================================================
// Draw Functions
// ============================================================================

ezResult ezGALCommandEncoderImplDX12::DrawPlatform(ezUInt32 uiVertexCount, ezUInt32 uiStartVertex)
{
  if (m_pCurrentPipelineLayout == nullptr)
    return EZ_FAILURE;

  FlushDeferredStateChanges();
  m_BarrierTracker.Flush();

  m_pCommandList->DrawInstanced(uiVertexCount, 1, uiStartVertex, 0);
  return EZ_SUCCESS;
}

ezResult ezGALCommandEncoderImplDX12::DrawIndexedPlatform(ezUInt32 uiIndexCount, ezUInt32 uiStartIndex)
{
  if (m_pCurrentPipelineLayout == nullptr)
    return EZ_FAILURE;

  FlushDeferredStateChanges();
  m_BarrierTracker.Flush();

  m_pCommandList->DrawIndexedInstanced(uiIndexCount, 1, uiStartIndex, 0, 0);
  return EZ_SUCCESS;
}

ezResult ezGALCommandEncoderImplDX12::DrawIndexedInstancedPlatform(ezUInt32 uiIndexCountPerInstance, ezUInt32 uiInstanceCount, ezUInt32 uiStartIndex)
{
  if (m_pCurrentPipelineLayout == nullptr)
    return EZ_FAILURE;

  FlushDeferredStateChanges();
  m_BarrierTracker.Flush();

  m_pCommandList->DrawIndexedInstanced(uiIndexCountPerInstance, uiInstanceCount, uiStartIndex, 0, 0);
  return EZ_SUCCESS;
}

ezResult ezGALCommandEncoderImplDX12::DrawIndexedInstancedIndirectPlatform(const ezGALBuffer* pIndirectArgumentBuffer, ezUInt32 uiArgumentOffsetInBytes)
{
  if (m_pCurrentPipelineLayout == nullptr)
    return EZ_FAILURE;

  FlushDeferredStateChanges();

  const ezGALBufferDX12* pBuffer = static_cast<const ezGALBufferDX12*>(pIndirectArgumentBuffer);

  m_BarrierTracker.BufferBarrier(pBuffer->GetDXResource(), pBuffer->GetCurrentState(), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
  m_BarrierTracker.Flush();

  m_pCommandList->ExecuteIndirect(m_Device.GetDrawIndexedIndirectSignature(), 1, pBuffer->GetDXResource(), uiArgumentOffsetInBytes, nullptr, 0);

  m_BarrierTracker.BufferBarrier(pBuffer->GetDXResource(), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, pBuffer->GetCurrentState());

  return EZ_SUCCESS;
}

ezResult ezGALCommandEncoderImplDX12::DrawInstancedPlatform(ezUInt32 uiVertexCountPerInstance, ezUInt32 uiInstanceCount, ezUInt32 uiStartVertex)
{
  if (m_pCurrentPipelineLayout == nullptr)
    return EZ_FAILURE;

  FlushDeferredStateChanges();
  m_BarrierTracker.Flush();

  m_pCommandList->DrawInstanced(uiVertexCountPerInstance, uiInstanceCount, uiStartVertex, 0);
  return EZ_SUCCESS;
}

ezResult ezGALCommandEncoderImplDX12::DrawInstancedIndirectPlatform(const ezGALBuffer* pIndirectArgumentBuffer, ezUInt32 uiArgumentOffsetInBytes)
{
  if (m_pCurrentPipelineLayout == nullptr)
    return EZ_FAILURE;

  FlushDeferredStateChanges();

  const ezGALBufferDX12* pBuffer = static_cast<const ezGALBufferDX12*>(pIndirectArgumentBuffer);

  m_BarrierTracker.BufferBarrier(pBuffer->GetDXResource(), pBuffer->GetCurrentState(), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
  m_BarrierTracker.Flush();

  m_pCommandList->ExecuteIndirect(m_Device.GetDrawIndirectSignature(), 1, pBuffer->GetDXResource(), uiArgumentOffsetInBytes, nullptr, 0);

  m_BarrierTracker.BufferBarrier(pBuffer->GetDXResource(), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, pBuffer->GetCurrentState());

  return EZ_SUCCESS;
}

// ============================================================================
// State Functions
// ============================================================================

void ezGALCommandEncoderImplDX12::SetIndexBufferPlatform(const ezGALBuffer* pIndexBuffer)
{
  if (pIndexBuffer != nullptr)
  {
    const ezGALBufferDX12* pBuffer = static_cast<const ezGALBufferDX12*>(pIndexBuffer);

    m_CurrentIndexBufferView.BufferLocation = pBuffer->GetDXResource()->GetGPUVirtualAddress();
    m_CurrentIndexBufferView.SizeInBytes = static_cast<UINT>(pBuffer->GetSize());
    m_CurrentIndexBufferView.Format = pBuffer->GetIndexFormat();
  }
  else
  {
    m_CurrentIndexBufferView = {};
  }
  m_bIndexBufferDirty = true;
}

void ezGALCommandEncoderImplDX12::SetVertexBufferPlatform(ezUInt32 uiSlot, const ezGALBuffer* pVertexBuffer, ezUInt32 uiOffset)
{
  EZ_ASSERT_DEV(uiSlot < EZ_GAL_MAX_VERTEX_BUFFER_COUNT, "Invalid slot index");

  if (pVertexBuffer != nullptr)
  {
    const ezGALBufferDX12* pBuffer = static_cast<const ezGALBufferDX12*>(pVertexBuffer);

    m_CurrentVertexBufferViews[uiSlot].BufferLocation = pBuffer->GetDXResource()->GetGPUVirtualAddress() + uiOffset;
    m_CurrentVertexBufferViews[uiSlot].SizeInBytes = static_cast<UINT>(pBuffer->GetSize() - uiOffset);
    m_CurrentVertexBufferViews[uiSlot].StrideInBytes = pBuffer->GetDescription().m_uiStructSize;
  }
  else
  {
    m_CurrentVertexBufferViews[uiSlot] = {};
  }
  m_bVertexBuffersDirty = true;
}

void ezGALCommandEncoderImplDX12::SetGraphicsPipelinePlatform(const ezGALGraphicsPipeline* pGraphicsPipeline)
{
  if (pGraphicsPipeline == nullptr)
  {
    m_pCurrentPipelineLayout = nullptr;
    return;
  }

  const ezGALGraphicsPipelineDX12* pPipelineDX12 = static_cast<const ezGALGraphicsPipelineDX12*>(pGraphicsPipeline);
  const ezGALGraphicsPipelineCreationDescription& desc = pGraphicsPipeline->GetDescription();

  // Set the PSO
  m_pCommandList->SetPipelineState(pPipelineDX12->GetPipelineState());

  // Set the root signature from the pipeline layout
  const ezGALShader* pShader = m_Device.GetShader(desc.m_hShader);
  EZ_ASSERT_DEV(pShader != nullptr, "Invalid shader in graphics pipeline");

  const ezGALPipelineLayoutDX12* pLayout = static_cast<const ezGALPipelineLayoutDX12*>(m_Device.GetPipelineLayout(pShader->GetPipelineLayout()));
  EZ_ASSERT_DEV(pLayout != nullptr, "Invalid pipeline layout");

  m_pCommandList->SetGraphicsRootSignature(pLayout->GetRootSignature());
  m_pCurrentPipelineLayout = pLayout;
  m_bGraphicsPipeline = true;

  // Set the primitive topology
  m_CurrentTopology = ezConversionUtilsDX12::ToD3DPrimitiveTopology(desc.m_Topology);
  m_pCommandList->IASetPrimitiveTopology(m_CurrentTopology);

  // Re-bind the descriptor heaps as the root signature change may invalidate them.
  SetDescriptorHeaps();
}

void ezGALCommandEncoderImplDX12::SetComputePipelinePlatform(const ezGALComputePipeline* pComputePipeline)
{
  if (pComputePipeline == nullptr)
  {
    m_pCurrentPipelineLayout = nullptr;
    return;
  }

  const ezGALComputePipelineDX12* pPipelineDX12 = static_cast<const ezGALComputePipelineDX12*>(pComputePipeline);
  const ezGALComputePipelineCreationDescription& desc = pComputePipeline->GetDescription();

  // Set the PSO
  m_pCommandList->SetPipelineState(pPipelineDX12->GetPipelineState());

  // Set the root signature from the pipeline layout
  const ezGALShader* pShader = m_Device.GetShader(desc.m_hShader);
  EZ_ASSERT_DEV(pShader != nullptr, "Invalid shader in compute pipeline");

  const ezGALPipelineLayoutDX12* pLayout = static_cast<const ezGALPipelineLayoutDX12*>(m_Device.GetPipelineLayout(pShader->GetPipelineLayout()));
  EZ_ASSERT_DEV(pLayout != nullptr, "Invalid pipeline layout");

  m_pCommandList->SetComputeRootSignature(pLayout->GetRootSignature());
  m_pCurrentPipelineLayout = pLayout;
  m_bGraphicsPipeline = false;

  SetDescriptorHeaps();
}

void ezGALCommandEncoderImplDX12::SetViewportPlatform(const ezRectFloat& rect, float fMinDepth, float fMaxDepth)
{
  D3D12_VIEWPORT viewport;
  viewport.TopLeftX = rect.x;
  viewport.TopLeftY = rect.y;
  viewport.Width = rect.width;
  viewport.Height = rect.height;
  viewport.MinDepth = fMinDepth;
  viewport.MaxDepth = fMaxDepth;

  m_pCommandList->RSSetViewports(1, &viewport);
}

void ezGALCommandEncoderImplDX12::SetScissorRectPlatform(const ezRectU32& rect)
{
  D3D12_RECT scissorRect;
  scissorRect.left = static_cast<LONG>(rect.x);
  scissorRect.top = static_cast<LONG>(rect.y);
  scissorRect.right = static_cast<LONG>(rect.x + rect.width);
  scissorRect.bottom = static_cast<LONG>(rect.y + rect.height);

  m_pCommandList->RSSetScissorRects(1, &scissorRect);
}

void ezGALCommandEncoderImplDX12::SetStencilReferencePlatform(ezUInt8 uiStencilRefValue)
{
  m_pCommandList->OMSetStencilRef(uiStencilRefValue);
}

// ============================================================================
// Deferred State Flush
// ============================================================================

void ezGALCommandEncoderImplDX12::FlushDeferredStateChanges()
{
  if (m_bIndexBufferDirty)
  {
    m_pCommandList->IASetIndexBuffer(m_CurrentIndexBufferView.SizeInBytes > 0 ? &m_CurrentIndexBufferView : nullptr);
    m_bIndexBufferDirty = false;
  }

  if (m_bVertexBuffersDirty)
  {
    // Find the range of non-null vertex buffers to set.
    UINT uiStartSlot = 0;
    UINT uiCount = 0;
    for (UINT i = 0; i < EZ_GAL_MAX_VERTEX_BUFFER_COUNT; ++i)
    {
      if (m_CurrentVertexBufferViews[i].BufferLocation != 0 || m_CurrentVertexBufferViews[i].SizeInBytes != 0)
      {
        uiCount = i + 1;
      }
    }

    if (uiCount > 0)
    {
      m_pCommandList->IASetVertexBuffers(uiStartSlot, uiCount, m_CurrentVertexBufferViews);
    }
    m_bVertexBuffersDirty = false;
  }
}
