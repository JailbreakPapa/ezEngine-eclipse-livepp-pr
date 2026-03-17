#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/Shader/ShaderDX12.h>

ezGALShaderDX12::ezGALShaderDX12(const ezGALShaderCreationDescription& description)
  : ezGALShader(description)
{
}

ezGALShaderDX12::~ezGALShaderDX12() = default;

void ezGALShaderDX12::SetDebugName(ezStringView sName) const
{
  EZ_IGNORE_UNUSED(sName);
  // DXIL bytecode blobs are plain data, not COM objects. There is nothing to tag.
}

ezResult ezGALShaderDX12::InitPlatform(ezGALDevice* pDevice)
{
  m_pDevice = pDevice;
  EZ_SUCCEED_OR_RETURN(CreateBindingMapping(true));
  EZ_SUCCEED_OR_RETURN(CreateLayouts(pDevice, false));

  // Copy DXIL bytecode for each active stage. In D3D12 there are no per-stage shader COM
  // objects; the raw bytecode is embedded directly into the pipeline state descriptor.
  for (ezUInt32 stage = 0; stage < ezGALShaderStage::ENUM_COUNT; ++stage)
  {
    if (m_Description.HasByteCodeForStage(static_cast<ezGALShaderStage::Enum>(stage)))
    {
      const auto& pByteCode = m_Description.m_ByteCodes[stage];
      m_ByteCodes[stage].m_ByteCode.SetCountUninitialized(pByteCode->GetSize());
      ezMemoryUtils::Copy(m_ByteCodes[stage].m_ByteCode.GetData(),
        static_cast<const ezUInt8*>(pByteCode->GetByteCode()), pByteCode->GetSize());
    }
  }

  return EZ_SUCCESS;
}

ezResult ezGALShaderDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  DestroyBindingMapping();
  DestroyLayouts(pDevice);

  for (ezUInt32 stage = 0; stage < ezGALShaderStage::ENUM_COUNT; ++stage)
  {
    m_ByteCodes[stage].m_ByteCode.Clear();
  }

  return EZ_SUCCESS;
}
