#include <ShaderCompilerDX12/ShaderCompilerDX12.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Strings/StringConversion.h>

#include <d3d12shader.h>
#include <d3dcompiler.h>
#include <dxcapi.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezShaderCompilerDX12, 1, ezRTTIDefaultAllocator<ezShaderCompilerDX12>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void ezShaderCompilerDX12::ConfigureDxcArgs(ezDynamicArray<ezStringWChar>& inout_Args)
{
  // DXC outputs DXIL natively when the -spirv flag is absent.
  // Remove -spirv if the base class added it.
  for (ezUInt32 i = inout_Args.GetCount(); i > 0; --i)
  {
    if (inout_Args[i - 1] == L"-spirv")
    {
      inout_Args.RemoveAtAndCopy(i - 1);
    }
  }

  // Also remove Vulkan-specific args
  for (ezUInt32 i = inout_Args.GetCount(); i > 0; --i)
  {
    const wchar_t* sz = inout_Args[i - 1].GetData();
    if (wcsstr(sz, L"-fvk-") != nullptr || wcsstr(sz, L"-fspv-") != nullptr)
    {
      inout_Args.RemoveAtAndCopy(i - 1);
    }
  }
}

template <typename T>
struct DxComPtr
{
  DxComPtr() = default;
  ~DxComPtr()
  {
    if (m_ptr)
    {
      m_ptr->Release();
      m_ptr = nullptr;
    }
  }
  DxComPtr(const DxComPtr&) = delete;
  DxComPtr& operator=(const DxComPtr&) = delete;

  T* operator->() const { return m_ptr; }
  T** put()
  {
    EZ_ASSERT_DEV(m_ptr == nullptr, "");
    return &m_ptr;
  }
  bool operator==(nullptr_t) const { return m_ptr == nullptr; }
  bool operator!=(nullptr_t) const { return m_ptr != nullptr; }
  T* Get() const { return m_ptr; }

private:
  T* m_ptr = nullptr;
};

ezResult ezShaderCompilerDX12::Compile(ezShaderProgramData& inout_Data, ezLogInterface* pLog)
{
  if (m_VertexInputMapping.IsEmpty())
  {
    m_VertexInputMapping["POSITION"] = ezGALVertexAttributeSemantic::Position;
    m_VertexInputMapping["NORMAL"] = ezGALVertexAttributeSemantic::Normal;
    m_VertexInputMapping["TANGENT"] = ezGALVertexAttributeSemantic::Tangent;
    m_VertexInputMapping["BITANGENT"] = ezGALVertexAttributeSemantic::BiTangent;
    m_VertexInputMapping["COLOR"] = ezGALVertexAttributeSemantic::Color0;
    m_VertexInputMapping["TEXCOORD"] = ezGALVertexAttributeSemantic::TexCoord0;
    m_VertexInputMapping["BONEINDICES"] = ezGALVertexAttributeSemantic::BoneIndices0;
    m_VertexInputMapping["BONEWEIGHTS"] = ezGALVertexAttributeSemantic::BoneWeights0;
    m_VertexInputMapping["DATAOFFSETS"] = ezGALVertexAttributeSemantic::DataOffsets;
  }

  // Create DXC instances
  DxComPtr<IDxcUtils> pUtils;
  DxComPtr<IDxcCompiler3> pCompiler;
  DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.put()));
  DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(pCompiler.put()));
  if (pUtils == nullptr || pCompiler == nullptr)
  {
    ezLog::Error("Failed to create DXC instances.");
    return EZ_FAILURE;
  }

  for (ezUInt32 stage = 0; stage < ezGALShaderStage::ENUM_COUNT; ++stage)
  {
    if (inout_Data.m_uiSourceHash[stage] == 0)
      continue;
    if (inout_Data.m_bWriteToDisk[stage] == false)
    {
      ezLog::Debug("Shader for stage '{}' is already compiled.", ezGALShaderStage::Names[stage]);
      continue;
    }

    const ezStringBuilder sShaderSource = inout_Data.m_sShaderSource[stage];
    if (sShaderSource.IsEmpty() || sShaderSource.FindSubString("main") == nullptr)
      continue;

    // Determine profile
    const char* szProfile = nullptr;
    switch ((ezGALShaderStage::Enum)stage)
    {
      case ezGALShaderStage::VertexShader:
        szProfile = "vs_6_0";
        break;
      case ezGALShaderStage::HullShader:
        szProfile = "hs_6_0";
        break;
      case ezGALShaderStage::DomainShader:
        szProfile = "ds_6_0";
        break;
      case ezGALShaderStage::GeometryShader:
        szProfile = "gs_6_0";
        break;
      case ezGALShaderStage::PixelShader:
        szProfile = "ps_6_0";
        break;
      case ezGALShaderStage::ComputeShader:
        szProfile = "cs_6_0";
        break;
      default:
        continue;
    }

    const ezStringBuilder sSourceFile = inout_Data.m_sSourceFile;
    ezStringView sCompileSource = sShaderSource;
    ezStringBuilder sDebugSource;

    // Set up DXC arguments
    ezDynamicArray<ezStringWChar> args;
    args.PushBack(ezStringWChar(sSourceFile.GetView()));
    args.PushBack(L"-E");
    args.PushBack(L"main");
    args.PushBack(L"-T");
    args.PushBack(ezStringWChar(szProfile));

    ConfigureDxcArgs(args);

    if (inout_Data.m_Flags.IsSet(ezShaderCompilerFlags::Debug))
    {
      sDebugSource = sShaderSource;
      sDebugSource.ReplaceAll("#line ", "//ine ");
      sCompileSource = sDebugSource;
      args.PushBack(L"-Zi");
    }

    // Create source blob
    DxComPtr<IDxcBlobEncoding> pSource;
    pUtils->CreateBlob(sCompileSource.GetStartPointer(), sCompileSource.GetElementCount(), DXC_CP_UTF8, pSource.put());

    DxcBuffer Source;
    Source.Ptr = pSource->GetBufferPointer();
    Source.Size = pSource->GetBufferSize();
    Source.Encoding = DXC_CP_UTF8;

    ezHybridArray<LPCWSTR, 16> pszArgs;
    pszArgs.SetCount(args.GetCount());
    for (ezUInt32 i = 0; i < args.GetCount(); ++i)
    {
      pszArgs[i] = args[i].GetData();
    }

    // Compile
    DxComPtr<IDxcResult> pResults;
    pCompiler->Compile(&Source, pszArgs.GetData(), pszArgs.GetCount(), nullptr, IID_PPV_ARGS(pResults.put()));

    DxComPtr<IDxcBlobUtf8> pErrors;
    pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(pErrors.put()), nullptr);

    HRESULT hrStatus;
    pResults->GetStatus(&hrStatus);
    if (FAILED(hrStatus))
    {
      ezLog::Error("DXIL shader compilation failed for stage '{}'.", ezGALShaderStage::Names[stage]);
      if (pErrors != nullptr && pErrors->GetStringLength() != 0)
      {
        ezLog::Error("{}", ezStringUtf8(pErrors->GetStringPointer()).GetData());
      }
      return EZ_FAILURE;
    }
    else
    {
      if (pErrors != nullptr && pErrors->GetStringLength() != 0)
      {
        ezLog::Warning("{}", ezStringUtf8(pErrors->GetStringPointer()).GetData());
      }
    }

    // Extract DXIL bytecode
    DxComPtr<IDxcBlob> pShader;
    DxComPtr<IDxcBlobWide> pShaderName;
    pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(pShader.put()), pShaderName.put());

    if (pShader == nullptr)
    {
      ezLog::Error("No DXIL bytecode was generated for stage '{}'.", ezGALShaderStage::Names[stage]);
      return EZ_FAILURE;
    }

    auto& byteCode = inout_Data.m_ByteCode[stage]->m_ByteCode;
    byteCode.SetCountUninitialized(static_cast<ezUInt32>(pShader->GetBufferSize()));
    ezMemoryUtils::Copy(byteCode.GetData(), reinterpret_cast<ezUInt8*>(pShader->GetBufferPointer()), byteCode.GetCount());

    // Extract reflection data from DXC result
    DxComPtr<IDxcBlob> pReflectionData;
    pResults->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(pReflectionData.put()), nullptr);

    if (pReflectionData != nullptr)
    {
      DxcBuffer reflectionBuffer;
      reflectionBuffer.Ptr = pReflectionData->GetBufferPointer();
      reflectionBuffer.Size = pReflectionData->GetBufferSize();
      reflectionBuffer.Encoding = 0;

      DxComPtr<ID3D12ShaderReflection> pReflector;
      if (SUCCEEDED(pUtils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(pReflector.put()))))
      {
        EZ_SUCCEED_OR_RETURN(ReflectShaderStage(inout_Data, (ezGALShaderStage::Enum)stage, pReflector.Get()));
      }
      else
      {
        ezLog::Error("Failed to create shader reflection for stage '{}'.", ezGALShaderStage::Names[stage]);
        return EZ_FAILURE;
      }
    }
    else
    {
      ezLog::Error("No reflection data available for stage '{}'.", ezGALShaderStage::Names[stage]);
      return EZ_FAILURE;
    }
  }

  return EZ_SUCCESS;
}

static ezGALResourceFormat::Enum GetEZFormatDX12(const D3D12_SIGNATURE_PARAMETER_DESC& paramDesc)
{
  ezUInt32 uiComponents = ezMath::Log2i(paramDesc.Mask + 1);
  switch (paramDesc.ComponentType)
  {
    case D3D_REGISTER_COMPONENT_UINT32:
      switch (uiComponents)
      {
        case 1:
          return ezGALResourceFormat::RUInt;
        case 2:
          return ezGALResourceFormat::RGUInt;
        case 3:
          return ezGALResourceFormat::RGBUInt;
        case 4:
          return ezGALResourceFormat::RGBAUInt;
        default:
          break;
      }
      break;
    case D3D_REGISTER_COMPONENT_SINT32:
      switch (uiComponents)
      {
        case 1:
          return ezGALResourceFormat::RInt;
        case 2:
          return ezGALResourceFormat::RGInt;
        case 3:
          return ezGALResourceFormat::RGBInt;
        case 4:
          return ezGALResourceFormat::RGBAInt;
        default:
          break;
      }
      break;
    case D3D_REGISTER_COMPONENT_FLOAT32:
      switch (uiComponents)
      {
        case 1:
          return ezGALResourceFormat::RFloat;
        case 2:
          return ezGALResourceFormat::RGFloat;
        case 3:
          return ezGALResourceFormat::RGBFloat;
        case 4:
          return ezGALResourceFormat::RGBAFloat;
        default:
          break;
      }
      break;
    default:
      break;
  }
  return ezGALResourceFormat::Invalid;
}

ezResult ezShaderCompilerDX12::ReflectShaderStage(ezShaderProgramData& inout_Data, ezGALShaderStage::Enum Stage, ID3D12ShaderReflection* pReflector)
{
  D3D12_SHADER_DESC shaderDesc;
  pReflector->GetDesc(&shaderDesc);

  ezGALShaderByteCode* pShader = inout_Data.m_ByteCode[Stage];

  if (Stage == ezGALShaderStage::VertexShader)
  {
    auto& vertexInputAttributes = pShader->m_ShaderVertexInput;
    vertexInputAttributes.Reserve(shaderDesc.InputParameters);
    for (ezUInt32 i = 0; i < shaderDesc.InputParameters; ++i)
    {
      D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
      pReflector->GetInputParameterDesc(i, &paramDesc);

      ezGALVertexAttributeSemantic::Enum semantic;
      if (!m_VertexInputMapping.TryGetValue(paramDesc.SemanticName, semantic))
      {
        if (ezStringUtils::StartsWith_NoCase(paramDesc.SemanticName, "SV_"))
          continue;

        ezLog::Warning("Unknown vertex input semantic: '{}'", paramDesc.SemanticName);
        continue;
      }

      switch (semantic)
      {
        case ezGALVertexAttributeSemantic::Color0:
          EZ_ASSERT_DEBUG(paramDesc.SemanticIndex <= 7, "Color out of range");
          semantic = static_cast<ezGALVertexAttributeSemantic::Enum>((ezUInt32)semantic + paramDesc.SemanticIndex);
          break;
        case ezGALVertexAttributeSemantic::TexCoord0:
          EZ_ASSERT_DEBUG(paramDesc.SemanticIndex <= 9, "TexCoord out of range");
          semantic = static_cast<ezGALVertexAttributeSemantic::Enum>((ezUInt32)semantic + paramDesc.SemanticIndex);
          break;
        case ezGALVertexAttributeSemantic::BoneIndices0:
          EZ_ASSERT_DEBUG(paramDesc.SemanticIndex <= 1, "BoneIndices out of range");
          semantic = static_cast<ezGALVertexAttributeSemantic::Enum>((ezUInt32)semantic + paramDesc.SemanticIndex);
          break;
        case ezGALVertexAttributeSemantic::BoneWeights0:
          EZ_ASSERT_DEBUG(paramDesc.SemanticIndex <= 1, "BoneWeights out of range");
          semantic = static_cast<ezGALVertexAttributeSemantic::Enum>((ezUInt32)semantic + paramDesc.SemanticIndex);
          break;
        default:
          break;
      }

      ezShaderVertexInputAttribute& attr = vertexInputAttributes.ExpandAndGetRef();
      attr.m_eSemantic = semantic;
      attr.m_eFormat = GetEZFormatDX12(paramDesc);
      attr.m_uiLocation = paramDesc.Register;
    }
  }
  else if (Stage == ezGALShaderStage::HullShader)
  {
    pShader->m_uiTessellationPatchControlPoints = shaderDesc.cControlPoints;
  }

  for (ezUInt32 r = 0; r < shaderDesc.BoundResources; ++r)
  {
    D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc;
    pReflector->GetResourceBindingDesc(r, &shaderInputBindDesc);

    ezShaderResourceBinding shaderResourceBinding;
    shaderResourceBinding.m_iBindGroup = static_cast<ezInt16>(shaderInputBindDesc.Space);
    shaderResourceBinding.m_iSlot = static_cast<ezInt16>(shaderInputBindDesc.BindPoint);
    shaderResourceBinding.m_uiArraySize = shaderInputBindDesc.BindCount;
    shaderResourceBinding.m_sName.Assign(shaderInputBindDesc.Name);
    shaderResourceBinding.m_Stages = ezGALShaderStageFlags::MakeFromShaderStage(Stage);

    if (shaderInputBindDesc.Type == D3D_SIT_TEXTURE || shaderInputBindDesc.Type == D3D_SIT_UAV_RWTYPED)
    {
      shaderResourceBinding.m_ResourceType = shaderInputBindDesc.Type == D3D_SIT_TEXTURE ? ezGALShaderResourceType::Texture : ezGALShaderResourceType::TextureRW;
      switch (shaderInputBindDesc.Dimension)
      {
        case D3D_SRV_DIMENSION_TEXTURE1D:
          shaderResourceBinding.m_TextureType = ezGALShaderTextureType::Texture1D;
          break;
        case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
          shaderResourceBinding.m_TextureType = ezGALShaderTextureType::Texture1DArray;
          break;
        case D3D_SRV_DIMENSION_TEXTURE2D:
          shaderResourceBinding.m_TextureType = ezGALShaderTextureType::Texture2D;
          break;
        case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
          shaderResourceBinding.m_TextureType = ezGALShaderTextureType::Texture2DArray;
          break;
        case D3D_SRV_DIMENSION_TEXTURE2DMS:
          shaderResourceBinding.m_TextureType = ezGALShaderTextureType::Texture2DMS;
          break;
        case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
          shaderResourceBinding.m_TextureType = ezGALShaderTextureType::Texture2DMSArray;
          break;
        case D3D_SRV_DIMENSION_TEXTURE3D:
          shaderResourceBinding.m_TextureType = ezGALShaderTextureType::Texture3D;
          break;
        case D3D_SRV_DIMENSION_TEXTURECUBE:
          shaderResourceBinding.m_TextureType = ezGALShaderTextureType::TextureCube;
          break;
        case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
          shaderResourceBinding.m_TextureType = ezGALShaderTextureType::TextureCubeArray;
          break;
        case D3D_SRV_DIMENSION_BUFFER:
          shaderResourceBinding.m_ResourceType = shaderInputBindDesc.Type == D3D_SIT_TEXTURE ? ezGALShaderResourceType::TexelBuffer : ezGALShaderResourceType::TexelBufferRW;
          shaderResourceBinding.m_TextureType = ezGALShaderTextureType::Unknown;
          break;
        default:
          EZ_ASSERT_NOT_IMPLEMENTED;
          break;
      }
    }
    else if (shaderInputBindDesc.Type == D3D_SIT_STRUCTURED)
      shaderResourceBinding.m_ResourceType = ezGALShaderResourceType::StructuredBuffer;
    else if (shaderInputBindDesc.Type == D3D_SIT_BYTEADDRESS)
      shaderResourceBinding.m_ResourceType = ezGALShaderResourceType::ByteAddressBuffer;
    else if (shaderInputBindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED)
      shaderResourceBinding.m_ResourceType = ezGALShaderResourceType::StructuredBufferRW;
    else if (shaderInputBindDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS)
      shaderResourceBinding.m_ResourceType = ezGALShaderResourceType::ByteAddressBufferRW;
    else if (shaderInputBindDesc.Type == D3D_SIT_UAV_APPEND_STRUCTURED)
      shaderResourceBinding.m_ResourceType = ezGALShaderResourceType::StructuredBufferRW;
    else if (shaderInputBindDesc.Type == D3D_SIT_UAV_CONSUME_STRUCTURED)
      shaderResourceBinding.m_ResourceType = ezGALShaderResourceType::StructuredBufferRW;
    else if (shaderInputBindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER)
      shaderResourceBinding.m_ResourceType = ezGALShaderResourceType::StructuredBufferRW;
    else if (shaderInputBindDesc.Type == D3D_SIT_CBUFFER)
    {
      shaderResourceBinding.m_ResourceType = ezGALShaderResourceType::ConstantBuffer;
      shaderResourceBinding.m_pLayout = ReflectConstantBufferLayout(pReflector->GetConstantBufferByName(shaderInputBindDesc.Name));
    }
    else if (shaderInputBindDesc.Type == D3D_SIT_SAMPLER)
    {
      shaderResourceBinding.m_ResourceType = ezGALShaderResourceType::Sampler;
      if (ezStringUtils::EndsWith(shaderInputBindDesc.Name, "_AutoSampler"))
      {
        ezStringBuilder sb = shaderInputBindDesc.Name;
        sb.Shrink(0, ezStringUtils::GetStringElementCount("_AutoSampler"));
        shaderResourceBinding.m_sName.Assign(sb.GetData());
      }
    }
    else
    {
      shaderResourceBinding.m_ResourceType = ezGALShaderResourceType::Unknown;
    }

    if (shaderResourceBinding.m_ResourceType != ezGALShaderResourceType::Unknown)
    {
      pShader->m_ShaderResourceBindings.PushBack(shaderResourceBinding);
    }
  }

  return EZ_SUCCESS;
}

ezSharedPtr<ezShaderConstantBufferLayout> ezShaderCompilerDX12::ReflectConstantBufferLayout(ID3D12ShaderReflectionConstantBuffer* pCB)
{
  D3D12_SHADER_BUFFER_DESC shaderBufferDesc;
  if (FAILED(pCB->GetDesc(&shaderBufferDesc)))
  {
    return nullptr;
  }

  EZ_LOG_BLOCK("Constant Buffer Layout", shaderBufferDesc.Name);
  ezLog::Debug("Constant Buffer has {} variables, Size is {}", shaderBufferDesc.Variables, shaderBufferDesc.Size);

  ezSharedPtr<ezShaderConstantBufferLayout> pLayout = EZ_DEFAULT_NEW(ezShaderConstantBufferLayout);
  pLayout->m_uiTotalSize = shaderBufferDesc.Size;

  for (ezUInt32 var = 0; var < shaderBufferDesc.Variables; ++var)
  {
    ID3D12ShaderReflectionVariable* pVar = pCB->GetVariableByIndex(var);

    D3D12_SHADER_VARIABLE_DESC svd;
    pVar->GetDesc(&svd);

    EZ_LOG_BLOCK("Constant", svd.Name);

    D3D12_SHADER_TYPE_DESC std;
    pVar->GetType()->GetDesc(&std);

    ezShaderConstant constant;
    constant.m_uiArrayElements = static_cast<ezUInt8>(ezMath::Max(std.Elements, 1u));
    constant.m_uiOffset = static_cast<ezUInt16>(svd.StartOffset);
    constant.m_sName.Assign(svd.Name);

    if (std.Class == D3D_SVC_SCALAR || std.Class == D3D_SVC_VECTOR)
    {
      switch (std.Type)
      {
        case D3D_SVT_FLOAT:
          constant.m_Type = (ezShaderConstant::Type::Enum)((ezInt32)ezShaderConstant::Type::Float1 + std.Columns - 1);
          break;
        case D3D_SVT_INT:
          constant.m_Type = (ezShaderConstant::Type::Enum)((ezInt32)ezShaderConstant::Type::Int1 + std.Columns - 1);
          break;
        case D3D_SVT_UINT:
          constant.m_Type = (ezShaderConstant::Type::Enum)((ezInt32)ezShaderConstant::Type::UInt1 + std.Columns - 1);
          break;
        case D3D_SVT_BOOL:
          if (std.Columns == 1)
            constant.m_Type = ezShaderConstant::Type::Bool;
          break;
        default:
          break;
      }
    }
    else if (std.Class == D3D_SVC_MATRIX_COLUMNS)
    {
      if (std.Type != D3D_SVT_FLOAT)
      {
        ezLog::Error("Variable '{}': Only float matrices are supported", svd.Name);
        continue;
      }

      if (std.Columns == 3 && std.Rows == 3)
        constant.m_Type = ezShaderConstant::Type::Mat3x3;
      else if (std.Columns == 4 && std.Rows == 4)
        constant.m_Type = ezShaderConstant::Type::Mat4x4;
      else
      {
        ezLog::Error("Variable '{}': {}x{} matrices are not supported", svd.Name, std.Rows, std.Columns);
        continue;
      }
    }
    else if (std.Class == D3D_SVC_MATRIX_ROWS)
    {
      ezLog::Error("Variable '{}': Row-Major matrices are not supported", svd.Name);
      continue;
    }
    else if (std.Class == D3D_SVC_STRUCT)
    {
      constant.m_Type = ezShaderConstant::Type::Struct;
      if (svd.Size == 48 && std.Members == 3)
      {
        ezStringView sMember0 = pVar->GetType()->GetMemberTypeName(0);
        ezStringView sMember1 = pVar->GetType()->GetMemberTypeName(1);
        ezStringView sMember2 = pVar->GetType()->GetMemberTypeName(2);
        if (sMember0 == "r0" && sMember1 == "r1" && sMember2 == "r2")
        {
          constant.m_Type = ezShaderConstant::Type::Transform;
        }
      }
    }

    if (constant.m_Type == ezShaderConstant::Type::Default)
    {
      ezLog::Error("Variable '{}': Variable type '{}' is unknown / not supported", svd.Name, std.Class);
      continue;
    }

    pLayout->m_Constants.PushBack(constant);
  }

  return pLayout;
}
