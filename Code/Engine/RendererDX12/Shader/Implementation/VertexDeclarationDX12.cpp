#include <RendererDX12/RendererDX12PCH.h>

#include <Foundation/Logging/Log.h>
#include <RendererDX12/Shader/VertexDeclarationDX12.h>
#include <RendererDX12/Utils/ConversionUtilsDX12.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Shader/Shader.h>

#include <d3d12.h>

// clang-format off
static const char* GALSemanticToDX12[] = {
  "POSITION",
  "NORMAL",
  "TANGENT",
  "COLOR",
  "COLOR",
  "COLOR",
  "COLOR",
  "COLOR",
  "COLOR",
  "COLOR",
  "COLOR",
  "TEXCOORD",
  "TEXCOORD",
  "TEXCOORD",
  "TEXCOORD",
  "TEXCOORD",
  "TEXCOORD",
  "TEXCOORD",
  "TEXCOORD",
  "TEXCOORD",
  "TEXCOORD",
  "BITANGENT",
  "BONEINDICES",
  "BONEINDICES",
  "BONEWEIGHTS",
  "BONEWEIGHTS",
  "DATAOFFSETS",
};

static UINT GALSemanticToIndexDX12[] = {
  0,                            // Position
  0,                            // Normal
  0,                            // Tangent
  0, 1, 2, 3, 4, 5, 6, 7,       // Color
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, // TexCoord
  0,                            // BiTangent
  0, 1,                         // BoneIndices
  0, 1,                         // BoneWeights
  0,                            // DataOffsets
};
// clang-format on

static_assert(EZ_ARRAY_SIZE(GALSemanticToDX12) == ezGALVertexAttributeSemantic::ENUM_COUNT,
  "GALSemanticToDX12 array size does not match vertex attribute semantic count");
static_assert(EZ_ARRAY_SIZE(GALSemanticToIndexDX12) == ezGALVertexAttributeSemantic::ENUM_COUNT,
  "GALSemanticToIndexDX12 array size does not match vertex attribute semantic count");

ezGALVertexDeclarationDX12::ezGALVertexDeclarationDX12(const ezGALVertexDeclarationCreationDescription& Description)
  : ezGALVertexDeclaration(Description)
{
}

ezGALVertexDeclarationDX12::~ezGALVertexDeclarationDX12() = default;

ezResult ezGALVertexDeclarationDX12::InitPlatform(ezGALDevice* pDevice)
{
  const ezGALShader* pShader = pDevice->GetShader(m_Description.m_hShader);

  if (pShader == nullptr || !pShader->GetDescription().HasByteCodeForStage(ezGALShaderStage::VertexShader))
  {
    return EZ_FAILURE;
  }

  auto usedVertexAttributes = pShader->GetVertexInputAttributes();
  auto IsAttributeUsed = [&](ezGALVertexAttributeSemantic::Enum semantic)
  {
    for (auto attrib : usedVertexAttributes)
    {
      if (attrib.m_eSemantic == semantic)
        return true;
    }
    return false;
  };

  for (ezUInt32 i = 0; i < m_Description.m_VertexAttributes.GetCount(); i++)
  {
    const ezGALVertexAttribute& Current = m_Description.m_VertexAttributes[i];
    if (!IsAttributeUsed(Current.m_eSemantic))
      continue;

    D3D12_INPUT_ELEMENT_DESC DXDesc = {};
    DXDesc.SemanticName = GALSemanticToDX12[Current.m_eSemantic];
    DXDesc.SemanticIndex = GALSemanticToIndexDX12[Current.m_eSemantic];
    DXDesc.Format = ezConversionUtilsDX12::ToDXGIFormat(Current.m_eFormat);

    if (DXDesc.Format == DXGI_FORMAT_UNKNOWN)
    {
      ezLog::Error("Vertex attribute format {0} of attribute at index {1} is unknown!", Current.m_eFormat, i);
      return EZ_FAILURE;
    }

    const ezGALVertexBinding& binding = m_Description.m_VertexBindings[Current.m_uiVertexBufferSlot];

    DXDesc.InputSlot = Current.m_uiVertexBufferSlot;
    DXDesc.AlignedByteOffset = Current.m_uiOffset;
    DXDesc.InputSlotClass = (binding.m_Rate == ezGALVertexBindingRate::Vertex)
                              ? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA
                              : D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    DXDesc.InstanceDataStepRate = (binding.m_Rate == ezGALVertexBindingRate::Vertex) ? 0 : 1;

    m_InputElements.PushBack(DXDesc);
  }

  if (m_InputElements.IsEmpty())
  {
    return EZ_FAILURE;
  }

  m_VertexBufferStrides.SetCount(m_Description.m_VertexBindings.GetCount());
  for (ezUInt32 i = 0; i < m_Description.m_VertexBindings.GetCount(); i++)
  {
    m_VertexBufferStrides[i] = m_Description.m_VertexBindings[i].m_uiStride;
  }

  return EZ_SUCCESS;
}

ezResult ezGALVertexDeclarationDX12::DeInitPlatform(ezGALDevice* pDevice)
{
  EZ_IGNORE_UNUSED(pDevice);

  m_InputElements.Clear();
  m_VertexBufferStrides.Clear();
  return EZ_SUCCESS;
}
