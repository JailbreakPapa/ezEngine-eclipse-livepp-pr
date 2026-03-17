#pragma once

#include <ShaderCompilerDXC/ShaderCompilerDXC.h>
#include <ShaderCompilerDX12/ShaderCompilerDX12DLL.h>

struct ID3D12ShaderReflection;
struct ID3D12ShaderReflectionConstantBuffer;

class EZ_SHADERCOMPILERDX12_DLL ezShaderCompilerDX12 : public ezShaderCompilerDXC
{
  EZ_ADD_DYNAMIC_REFLECTION(ezShaderCompilerDX12, ezShaderCompilerDXC);

public:
  virtual void GetSupportedPlatforms(ezHybridArray<ezString, 4>& out_platforms) override
  {
    out_platforms.PushBack("DX12");
  }

  virtual ezEnum<ezGALBufferLayout> GetMaterialBufferLayout(ezStringView sPlatform) const override
  {
    return ezGALBufferLayout::DirectX_ConstantButter;
  }

  virtual ezResult Compile(ezShaderProgramData& inout_Data, ezLogInterface* pLog) override;

protected:
  virtual void ConfigureDxcArgs(ezDynamicArray<ezStringWChar>& inout_Args) override;
  virtual bool AllowCombinedImageSamplers() const override { return false; }

private:
  ezResult ReflectShaderStage(ezShaderProgramData& inout_Data, ezGALShaderStage::Enum Stage, ID3D12ShaderReflection* pReflector);
  ezSharedPtr<ezShaderConstantBufferLayout> ReflectConstantBufferLayout(ID3D12ShaderReflectionConstantBuffer* pCB);
};
