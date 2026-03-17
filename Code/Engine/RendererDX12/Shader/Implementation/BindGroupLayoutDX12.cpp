#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/Shader/BindGroupLayoutDX12.h>
#include <RendererFoundation/Shader/ShaderByteCode.h>

ezGALBindGroupLayoutDX12::ezGALBindGroupLayoutDX12(const ezGALBindGroupLayoutCreationDescription& Description)
  : ezGALBindGroupLayout(Description)
{
}

ezGALBindGroupLayoutDX12::~ezGALBindGroupLayoutDX12() = default;

ezResult ezGALBindGroupLayoutDX12::InitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  m_uiCbvCount = 0;
  m_uiSrvCount = 0;
  m_uiUavCount = 0;
  m_uiSamplerCount = 0;

  for (const auto& binding : m_Description.m_ResourceBindings)
  {
    switch (binding.m_ResourceType)
    {
      case ezGALShaderResourceType::Sampler:
        m_uiSamplerCount++;
        break;

      case ezGALShaderResourceType::ConstantBuffer:
      case ezGALShaderResourceType::PushConstants:
        m_uiCbvCount++;
        break;

      case ezGALShaderResourceType::Texture:
      case ezGALShaderResourceType::TextureAndSampler:
      case ezGALShaderResourceType::TexelBuffer:
      case ezGALShaderResourceType::StructuredBuffer:
      case ezGALShaderResourceType::ByteAddressBuffer:
        m_uiSrvCount++;
        break;

      case ezGALShaderResourceType::TextureRW:
      case ezGALShaderResourceType::TexelBufferRW:
      case ezGALShaderResourceType::StructuredBufferRW:
      case ezGALShaderResourceType::ByteAddressBufferRW:
        m_uiUavCount++;
        break;

      default:
        break;
    }

    // TextureAndSampler contributes both an SRV and a Sampler.
    if (binding.m_ResourceType == ezGALShaderResourceType::TextureAndSampler)
    {
      m_uiSamplerCount++;
    }
  }

  m_uiSrvUavCbvCount = m_uiCbvCount + m_uiSrvCount + m_uiUavCount;
  return EZ_SUCCESS;
}

ezResult ezGALBindGroupLayoutDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  m_uiCbvCount = 0;
  m_uiSrvCount = 0;
  m_uiUavCount = 0;
  m_uiSrvUavCbvCount = 0;
  m_uiSamplerCount = 0;
  return EZ_SUCCESS;
}
