#pragma once

#include <RendererDX12/RendererDX12DLL.h>
#include <RendererFoundation/RendererFoundationDLL.h>
#include <RendererFoundation/Shader/Shader.h>

#include <d3d12.h>

/// Stores DXIL bytecode for a single shader stage.
struct ezDXILByteCode
{
  ezDynamicArray<ezUInt8> m_ByteCode;

  D3D12_SHADER_BYTECODE GetByteCode() const { return {m_ByteCode.GetData(), m_ByteCode.GetCount()}; }
  bool IsValid() const { return !m_ByteCode.IsEmpty(); }
};

/// D3D12 shader implementation.
///
/// Unlike D3D11 which creates per-stage shader objects, D3D12 only needs the DXIL bytecode
/// to embed into pipeline state objects. This class stores a copy of the bytecode for each
/// active stage.
class EZ_RENDERERDX12_DLL ezGALShaderDX12 : public ezGALShader
{
public:
  void SetDebugName(ezStringView sName) const override;

  EZ_ALWAYS_INLINE const ezDXILByteCode& GetByteCode(ezGALShaderStage::Enum stage) const { return m_ByteCodes[stage]; }
  EZ_ALWAYS_INLINE bool HasStage(ezGALShaderStage::Enum stage) const { return m_ByteCodes[stage].IsValid(); }

protected:
  friend class ezGALDeviceDX12;
  friend class ezMemoryUtils;

  ezGALShaderDX12(const ezGALShaderCreationDescription& description);
  virtual ~ezGALShaderDX12();

  virtual ezResult InitPlatform(ezGALDevice* pDevice) override;
  virtual ezResult DeInitPlatform(ezGALDevice* pDevice) override;

  ezDXILByteCode m_ByteCodes[ezGALShaderStage::ENUM_COUNT];
};
