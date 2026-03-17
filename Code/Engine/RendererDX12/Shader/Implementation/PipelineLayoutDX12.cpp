#include <RendererDX12/RendererDX12PCH.h>

#include <Foundation/Logging/Log.h>
#include <RendererDX12/Device/DeviceDX12.h>
#include <RendererDX12/Shader/BindGroupLayoutDX12.h>
#include <RendererDX12/Shader/PipelineLayoutDX12.h>

#include <d3d12.h>

EZ_DEFINE_AS_POD_TYPE(D3D12_ROOT_PARAMETER);
EZ_DEFINE_AS_POD_TYPE(D3D12_DESCRIPTOR_RANGE);

ezGALPipelineLayoutDX12::ezGALPipelineLayoutDX12(const ezGALPipelineLayoutCreationDescription& Description)
  : ezGALPipelineLayout(Description)
{
}

ezGALPipelineLayoutDX12::~ezGALPipelineLayoutDX12() = default;

ezResult ezGALPipelineLayoutDX12::InitPlatform(ezGALDevice* pDevice)
{
  ezGALDeviceDX12* pDX12Device = static_cast<ezGALDeviceDX12*>(pDevice);
  ID3D12Device* pD3D12Device = pDX12Device->GetDXDevice();

  // Build root parameters from the bind group layouts.
  // Each bind group can contribute up to 2 descriptor table root parameters:
  // one for CBV/SRV/UAV and one for Samplers.
  ezHybridArray<D3D12_ROOT_PARAMETER, 16> rootParameters;
  // Keep descriptor ranges alive until serialization is complete.
  ezHybridArray<D3D12_DESCRIPTOR_RANGE, 32> allRanges;

  for (ezUInt32 bg = 0; bg < EZ_GAL_MAX_BIND_GROUPS; ++bg)
  {
    if (m_Description.m_BindGroups[bg].IsInvalidated())
      continue;

    const ezGALBindGroupLayout* pLayout = pDevice->GetBindGroupLayout(m_Description.m_BindGroups[bg]);
    if (pLayout == nullptr)
      continue;

    const ezGALBindGroupLayoutDX12* pDX12Layout = static_cast<const ezGALBindGroupLayoutDX12*>(pLayout);
    const auto& bindings = pDX12Layout->GetDescription().m_ResourceBindings;

    // Build one descriptor range per resource binding so that each register slot is exact.
    // CBV/SRV/UAV descriptor table
    if (pDX12Layout->GetSrvUavCbvCount() > 0)
    {
      const ezUInt32 rangeStartIndex = allRanges.GetCount();

      for (const auto& binding : bindings)
      {
        D3D12_DESCRIPTOR_RANGE range = {};
        range.RegisterSpace = bg;
        range.NumDescriptors = 1;
        range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        switch (binding.m_ResourceType)
        {
          case ezGALShaderResourceType::ConstantBuffer:
          case ezGALShaderResourceType::PushConstants:
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            range.BaseShaderRegister = binding.m_iSlot;
            allRanges.PushBack(range);
            break;

          case ezGALShaderResourceType::Texture:
          case ezGALShaderResourceType::TextureAndSampler:
          case ezGALShaderResourceType::TexelBuffer:
          case ezGALShaderResourceType::StructuredBuffer:
          case ezGALShaderResourceType::ByteAddressBuffer:
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            range.BaseShaderRegister = binding.m_iSlot;
            allRanges.PushBack(range);
            break;

          case ezGALShaderResourceType::TextureRW:
          case ezGALShaderResourceType::TexelBufferRW:
          case ezGALShaderResourceType::StructuredBufferRW:
          case ezGALShaderResourceType::ByteAddressBufferRW:
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            range.BaseShaderRegister = binding.m_iSlot;
            allRanges.PushBack(range);
            break;

          default:
            // Samplers are handled separately below.
            break;
        }
      }

      const ezUInt32 rangeCount = allRanges.GetCount() - rangeStartIndex;
      if (rangeCount > 0)
      {
        D3D12_ROOT_PARAMETER param = {};
        param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param.DescriptorTable.NumDescriptorRanges = rangeCount;
        // Pointer is set after all ranges are finalized to avoid pointer invalidation.
        param.DescriptorTable.pDescriptorRanges = nullptr;
        param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        m_iSrvUavCbvRootParamIndex[bg] = static_cast<ezInt8>(rootParameters.GetCount());
        rootParameters.PushBack(param);
      }
    }

    // Sampler descriptor table
    if (pDX12Layout->GetSamplerCount() > 0)
    {
      const ezUInt32 rangeStartIndex = allRanges.GetCount();

      for (const auto& binding : bindings)
      {
        if (binding.m_ResourceType == ezGALShaderResourceType::Sampler)
        {
          D3D12_DESCRIPTOR_RANGE range = {};
          range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
          range.NumDescriptors = 1;
          range.BaseShaderRegister = binding.m_iSlot;
          range.RegisterSpace = bg;
          range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
          allRanges.PushBack(range);
        }
        else if (binding.m_ResourceType == ezGALShaderResourceType::TextureAndSampler)
        {
          // Combined image samplers contribute both an SRV (above) and a sampler.
          D3D12_DESCRIPTOR_RANGE range = {};
          range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
          range.NumDescriptors = 1;
          range.BaseShaderRegister = binding.m_iSlot;
          range.RegisterSpace = bg;
          range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
          allRanges.PushBack(range);
        }
      }

      const ezUInt32 rangeCount = allRanges.GetCount() - rangeStartIndex;
      if (rangeCount > 0)
      {
        D3D12_ROOT_PARAMETER param = {};
        param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param.DescriptorTable.NumDescriptorRanges = rangeCount;
        param.DescriptorTable.pDescriptorRanges = nullptr;
        param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        m_iSamplerRootParamIndex[bg] = static_cast<ezInt8>(rootParameters.GetCount());
        rootParameters.PushBack(param);
      }
    }
  }

  // Now fix up all descriptor table pointers. The allRanges array is stable at this point.
  {
    ezUInt32 rangeOffset = 0;
    for (ezUInt32 i = 0; i < rootParameters.GetCount(); ++i)
    {
      if (rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
      {
        rootParameters[i].DescriptorTable.pDescriptorRanges = allRanges.GetData() + rangeOffset;
        rangeOffset += rootParameters[i].DescriptorTable.NumDescriptorRanges;
      }
    }
  }

  // Push constants as root constants
  if (m_Description.m_PushConstants.m_uiSize > 0)
  {
    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    param.Constants.ShaderRegister = 0;
    param.Constants.RegisterSpace = EZ_GAL_MAX_BIND_GROUPS; // Use a dedicated register space for push constants.
    param.Constants.Num32BitValues = (m_Description.m_PushConstants.m_uiSize + 3) / 4;
    param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    m_iPushConstantsRootParamIndex = static_cast<ezInt8>(rootParameters.GetCount());
    rootParameters.PushBack(param);
  }

  D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
  rootSigDesc.NumParameters = rootParameters.GetCount();
  rootSigDesc.pParameters = rootParameters.GetData();
  rootSigDesc.NumStaticSamplers = 0;
  rootSigDesc.pStaticSamplers = nullptr;
  rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

  ID3DBlob* pSignatureBlob = nullptr;
  ID3DBlob* pErrorBlob = nullptr;
  HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignatureBlob, &pErrorBlob);
  if (FAILED(hr))
  {
    if (pErrorBlob)
    {
      ezLog::Error("Failed to serialize root signature: {}", static_cast<const char*>(pErrorBlob->GetBufferPointer()));
      pErrorBlob->Release();
    }
    else
    {
      ezLog::Error("Failed to serialize root signature: {}", ezArgErrorCode(hr));
    }
    return EZ_FAILURE;
  }

  hr = pD3D12Device->CreateRootSignature(0, pSignatureBlob->GetBufferPointer(),
    pSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
  pSignatureBlob->Release();

  if (FAILED(hr))
  {
    ezLog::Error("Failed to create root signature: {}", ezArgErrorCode(hr));
    return EZ_FAILURE;
  }

  return EZ_SUCCESS;
}

ezResult ezGALPipelineLayoutDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  EZ_GAL_DX12_RELEASE(m_pRootSignature);

  for (ezUInt32 i = 0; i < EZ_GAL_MAX_BIND_GROUPS; ++i)
  {
    m_iSrvUavCbvRootParamIndex[i] = -1;
    m_iSamplerRootParamIndex[i] = -1;
  }
  m_iPushConstantsRootParamIndex = -1;

  return EZ_SUCCESS;
}
