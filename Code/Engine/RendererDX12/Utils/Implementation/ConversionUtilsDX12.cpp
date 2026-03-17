#include <RendererDX12/RendererDX12PCH.h>

#include <RendererDX12/Utils/ConversionUtilsDX12.h>

// clang-format off

DXGI_FORMAT ezConversionUtilsDX12::ToDXGIFormat(ezGALResourceFormat::Enum format)
{
  switch (format)
  {
    case ezGALResourceFormat::RGBAFloat:            return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case ezGALResourceFormat::RGBAUInt:             return DXGI_FORMAT_R32G32B32A32_UINT;
    case ezGALResourceFormat::RGBAInt:              return DXGI_FORMAT_R32G32B32A32_SINT;

    case ezGALResourceFormat::RGBFloat:             return DXGI_FORMAT_R32G32B32_FLOAT;
    case ezGALResourceFormat::RGBUInt:              return DXGI_FORMAT_R32G32B32_UINT;
    case ezGALResourceFormat::RGBInt:               return DXGI_FORMAT_R32G32B32_SINT;

    case ezGALResourceFormat::B5G6R5UNormalized:    return DXGI_FORMAT_B5G6R5_UNORM;
    case ezGALResourceFormat::BGRAUByteNormalized:  return DXGI_FORMAT_B8G8R8A8_UNORM;
    case ezGALResourceFormat::BGRAUByteNormalizedsRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

    case ezGALResourceFormat::RGBAHalf:             return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case ezGALResourceFormat::RGBAUShort:           return DXGI_FORMAT_R16G16B16A16_UINT;
    case ezGALResourceFormat::RGBAUShortNormalized: return DXGI_FORMAT_R16G16B16A16_UNORM;
    case ezGALResourceFormat::RGBAShort:            return DXGI_FORMAT_R16G16B16A16_SINT;
    case ezGALResourceFormat::RGBAShortNormalized:  return DXGI_FORMAT_R16G16B16A16_SNORM;

    case ezGALResourceFormat::RGFloat:              return DXGI_FORMAT_R32G32_FLOAT;
    case ezGALResourceFormat::RGUInt:               return DXGI_FORMAT_R32G32_UINT;
    case ezGALResourceFormat::RGInt:                return DXGI_FORMAT_R32G32_SINT;

    case ezGALResourceFormat::RGB10A2UInt:          return DXGI_FORMAT_R10G10B10A2_UINT;
    case ezGALResourceFormat::RGB10A2UIntNormalized: return DXGI_FORMAT_R10G10B10A2_UNORM;
    case ezGALResourceFormat::RG11B10Float:         return DXGI_FORMAT_R11G11B10_FLOAT;

    case ezGALResourceFormat::RGBAUByteNormalized:  return DXGI_FORMAT_R8G8B8A8_UNORM;
    case ezGALResourceFormat::RGBAUByteNormalizedsRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case ezGALResourceFormat::RGBAUByte:            return DXGI_FORMAT_R8G8B8A8_UINT;
    case ezGALResourceFormat::RGBAByteNormalized:   return DXGI_FORMAT_R8G8B8A8_SNORM;
    case ezGALResourceFormat::RGBAByte:             return DXGI_FORMAT_R8G8B8A8_SINT;

    case ezGALResourceFormat::RGHalf:               return DXGI_FORMAT_R16G16_FLOAT;
    case ezGALResourceFormat::RGUShort:             return DXGI_FORMAT_R16G16_UINT;
    case ezGALResourceFormat::RGUShortNormalized:   return DXGI_FORMAT_R16G16_UNORM;
    case ezGALResourceFormat::RGShort:              return DXGI_FORMAT_R16G16_SINT;
    case ezGALResourceFormat::RGShortNormalized:    return DXGI_FORMAT_R16G16_SNORM;
    case ezGALResourceFormat::RGUByte:              return DXGI_FORMAT_R8G8_UINT;
    case ezGALResourceFormat::RGUByteNormalized:    return DXGI_FORMAT_R8G8_UNORM;
    case ezGALResourceFormat::RGByte:               return DXGI_FORMAT_R8G8_SINT;
    case ezGALResourceFormat::RGByteNormalized:     return DXGI_FORMAT_R8G8_SNORM;

    case ezGALResourceFormat::DFloat:               return DXGI_FORMAT_D32_FLOAT;

    case ezGALResourceFormat::RFloat:               return DXGI_FORMAT_R32_FLOAT;
    case ezGALResourceFormat::RUInt:                return DXGI_FORMAT_R32_UINT;
    case ezGALResourceFormat::RInt:                 return DXGI_FORMAT_R32_SINT;
    case ezGALResourceFormat::RHalf:                return DXGI_FORMAT_R16_FLOAT;
    case ezGALResourceFormat::RUShort:              return DXGI_FORMAT_R16_UINT;
    case ezGALResourceFormat::RUShortNormalized:    return DXGI_FORMAT_R16_UNORM;
    case ezGALResourceFormat::RShort:               return DXGI_FORMAT_R16_SINT;
    case ezGALResourceFormat::RShortNormalized:     return DXGI_FORMAT_R16_SNORM;
    case ezGALResourceFormat::RUByte:               return DXGI_FORMAT_R8_UINT;
    case ezGALResourceFormat::RUByteNormalized:     return DXGI_FORMAT_R8_UNORM;
    case ezGALResourceFormat::RByte:                return DXGI_FORMAT_R8_SINT;
    case ezGALResourceFormat::RByteNormalized:      return DXGI_FORMAT_R8_SNORM;

    case ezGALResourceFormat::AUByteNormalized:     return DXGI_FORMAT_A8_UNORM;

    case ezGALResourceFormat::D16:                  return DXGI_FORMAT_D16_UNORM;
    case ezGALResourceFormat::D24S8:                return DXGI_FORMAT_D24_UNORM_S8_UINT;

    case ezGALResourceFormat::BC1:                  return DXGI_FORMAT_BC1_UNORM;
    case ezGALResourceFormat::BC1sRGB:              return DXGI_FORMAT_BC1_UNORM_SRGB;
    case ezGALResourceFormat::BC2:                  return DXGI_FORMAT_BC2_UNORM;
    case ezGALResourceFormat::BC2sRGB:              return DXGI_FORMAT_BC2_UNORM_SRGB;
    case ezGALResourceFormat::BC3:                  return DXGI_FORMAT_BC3_UNORM;
    case ezGALResourceFormat::BC3sRGB:              return DXGI_FORMAT_BC3_UNORM_SRGB;
    case ezGALResourceFormat::BC4UNormalized:       return DXGI_FORMAT_BC4_UNORM;
    case ezGALResourceFormat::BC4Normalized:        return DXGI_FORMAT_BC4_SNORM;
    case ezGALResourceFormat::BC5UNormalized:       return DXGI_FORMAT_BC5_UNORM;
    case ezGALResourceFormat::BC5Normalized:        return DXGI_FORMAT_BC5_SNORM;
    case ezGALResourceFormat::BC6UFloat:            return DXGI_FORMAT_BC6H_UF16;
    case ezGALResourceFormat::BC6Float:             return DXGI_FORMAT_BC6H_SF16;
    case ezGALResourceFormat::BC7UNormalized:       return DXGI_FORMAT_BC7_UNORM;
    case ezGALResourceFormat::BC7UNormalizedsRGB:   return DXGI_FORMAT_BC7_UNORM_SRGB;

    default:
      EZ_REPORT_FAILURE("Invalid ezGALResourceFormat");
      return DXGI_FORMAT_UNKNOWN;
  }
}

D3D12_BLEND ezConversionUtilsDX12::ToD3D12Blend(ezGALBlend::Enum blend)
{
  switch (blend)
  {
    case ezGALBlend::Zero:             return D3D12_BLEND_ZERO;
    case ezGALBlend::One:              return D3D12_BLEND_ONE;
    case ezGALBlend::SrcColor:         return D3D12_BLEND_SRC_COLOR;
    case ezGALBlend::InvSrcColor:      return D3D12_BLEND_INV_SRC_COLOR;
    case ezGALBlend::SrcAlpha:         return D3D12_BLEND_SRC_ALPHA;
    case ezGALBlend::InvSrcAlpha:      return D3D12_BLEND_INV_SRC_ALPHA;
    case ezGALBlend::DestAlpha:        return D3D12_BLEND_DEST_ALPHA;
    case ezGALBlend::InvDestAlpha:     return D3D12_BLEND_INV_DEST_ALPHA;
    case ezGALBlend::DestColor:        return D3D12_BLEND_DEST_COLOR;
    case ezGALBlend::InvDestColor:     return D3D12_BLEND_INV_DEST_COLOR;
    case ezGALBlend::SrcAlphaSaturated: return D3D12_BLEND_SRC_ALPHA_SAT;
    case ezGALBlend::BlendFactor:      return D3D12_BLEND_BLEND_FACTOR;
    case ezGALBlend::InvBlendFactor:   return D3D12_BLEND_INV_BLEND_FACTOR;

    default:
      EZ_REPORT_FAILURE("Invalid ezGALBlend value");
      return D3D12_BLEND_ONE;
  }
}

D3D12_BLEND_OP ezConversionUtilsDX12::ToD3D12BlendOp(ezGALBlendOp::Enum blendOp)
{
  switch (blendOp)
  {
    case ezGALBlendOp::Add:         return D3D12_BLEND_OP_ADD;
    case ezGALBlendOp::Subtract:    return D3D12_BLEND_OP_SUBTRACT;
    case ezGALBlendOp::RevSubtract: return D3D12_BLEND_OP_REV_SUBTRACT;
    case ezGALBlendOp::Min:         return D3D12_BLEND_OP_MIN;
    case ezGALBlendOp::Max:         return D3D12_BLEND_OP_MAX;

    default:
      EZ_REPORT_FAILURE("Invalid ezGALBlendOp value");
      return D3D12_BLEND_OP_ADD;
  }
}

D3D12_COMPARISON_FUNC ezConversionUtilsDX12::ToD3D12ComparisonFunc(ezGALCompareFunc::Enum compareFunc)
{
  switch (compareFunc)
  {
    case ezGALCompareFunc::Never:        return D3D12_COMPARISON_FUNC_NEVER;
    case ezGALCompareFunc::Less:         return D3D12_COMPARISON_FUNC_LESS;
    case ezGALCompareFunc::Equal:        return D3D12_COMPARISON_FUNC_EQUAL;
    case ezGALCompareFunc::LessEqual:    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
    case ezGALCompareFunc::Greater:      return D3D12_COMPARISON_FUNC_GREATER;
    case ezGALCompareFunc::NotEqual:     return D3D12_COMPARISON_FUNC_NOT_EQUAL;
    case ezGALCompareFunc::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    case ezGALCompareFunc::Always:       return D3D12_COMPARISON_FUNC_ALWAYS;

    default:
      EZ_REPORT_FAILURE("Invalid ezGALCompareFunc value");
      return D3D12_COMPARISON_FUNC_NEVER;
  }
}

D3D12_STENCIL_OP ezConversionUtilsDX12::ToD3D12StencilOp(ezGALStencilOp::Enum stencilOp)
{
  switch (stencilOp)
  {
    case ezGALStencilOp::Keep:               return D3D12_STENCIL_OP_KEEP;
    case ezGALStencilOp::Zero:               return D3D12_STENCIL_OP_ZERO;
    case ezGALStencilOp::Replace:            return D3D12_STENCIL_OP_REPLACE;
    case ezGALStencilOp::IncrementSaturated: return D3D12_STENCIL_OP_INCR_SAT;
    case ezGALStencilOp::DecrementSaturated: return D3D12_STENCIL_OP_DECR_SAT;
    case ezGALStencilOp::Invert:             return D3D12_STENCIL_OP_INVERT;
    case ezGALStencilOp::Increment:          return D3D12_STENCIL_OP_INCR;
    case ezGALStencilOp::Decrement:          return D3D12_STENCIL_OP_DECR;

    default:
      EZ_REPORT_FAILURE("Invalid ezGALStencilOp value");
      return D3D12_STENCIL_OP_KEEP;
  }
}

D3D12_CULL_MODE ezConversionUtilsDX12::ToD3D12CullMode(ezGALCullMode::Enum cullMode)
{
  switch (cullMode)
  {
    case ezGALCullMode::None:  return D3D12_CULL_MODE_NONE;
    case ezGALCullMode::Front: return D3D12_CULL_MODE_FRONT;
    case ezGALCullMode::Back:  return D3D12_CULL_MODE_BACK;

    default:
      EZ_REPORT_FAILURE("Invalid ezGALCullMode value");
      return D3D12_CULL_MODE_BACK;
  }
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE ezConversionUtilsDX12::ToD3D12PrimitiveTopologyType(ezGALPrimitiveTopology::Enum topology)
{
  switch (topology)
  {
    case ezGALPrimitiveTopology::Points:        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    case ezGALPrimitiveTopology::Lines:         return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    case ezGALPrimitiveTopology::Triangles:     return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case ezGALPrimitiveTopology::TriangleStrip: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    default:
      EZ_REPORT_FAILURE("Invalid ezGALPrimitiveTopology value");
      return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  }
}

D3D_PRIMITIVE_TOPOLOGY ezConversionUtilsDX12::ToD3DPrimitiveTopology(ezGALPrimitiveTopology::Enum topology)
{
  switch (topology)
  {
    case ezGALPrimitiveTopology::Points:        return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    case ezGALPrimitiveTopology::Lines:         return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    case ezGALPrimitiveTopology::Triangles:     return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case ezGALPrimitiveTopology::TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

    default:
      EZ_REPORT_FAILURE("Invalid ezGALPrimitiveTopology value");
      return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
  }
}

D3D12_TEXTURE_ADDRESS_MODE ezConversionUtilsDX12::ToD3D12TextureAddressMode(ezImageAddressMode::Enum addressMode)
{
  switch (addressMode)
  {
    case ezImageAddressMode::Repeat:      return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    case ezImageAddressMode::Clamp:       return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    case ezImageAddressMode::ClampBorder: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    case ezImageAddressMode::Mirror:      return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;

    default:
      EZ_REPORT_FAILURE("Invalid ezImageAddressMode value");
      return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  }
}

// clang-format on

D3D12_FILTER ezConversionUtilsDX12::ToD3D12Filter(ezGALTextureFilterMode::Enum filterMode)
{
  switch (filterMode)
  {
    case ezGALTextureFilterMode::Point:
      return D3D12_FILTER_MIN_MAG_MIP_POINT;

    case ezGALTextureFilterMode::Linear:
      return D3D12_FILTER_MIN_MAG_MIP_LINEAR;

    case ezGALTextureFilterMode::Anisotropic:
      return D3D12_FILTER_ANISOTROPIC;

    default:
      EZ_REPORT_FAILURE("Invalid ezGALTextureFilterMode value");
      return D3D12_FILTER_MIN_MAG_MIP_POINT;
  }
}

DXGI_SAMPLE_DESC ezConversionUtilsDX12::ToD3D12SampleDesc(ezGALMSAASampleCount::Enum sampleCount)
{
  switch (sampleCount)
  {
    case ezGALMSAASampleCount::None:
      return {1, 0};

    case ezGALMSAASampleCount::TwoSamples:
      return {2, 0};

    case ezGALMSAASampleCount::FourSamples:
      return {4, 0};

    case ezGALMSAASampleCount::EightSamples:
      return {8, 0};

    default:
      EZ_REPORT_FAILURE("Invalid ezGALMSAASampleCount value");
      return {1, 0};
  }
}

UINT ezConversionUtilsDX12::ToD3D12ResourceFlags(const ezGALBufferCreationDescription& desc)
{
  UINT flags = D3D12_RESOURCE_FLAG_NONE;

  if (desc.m_BufferFlags.IsSet(ezGALBufferUsageFlags::UnorderedAccess))
  {
    flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  }

  return flags;
}

D3D12_RESOURCE_FLAGS ezConversionUtilsDX12::ToD3D12TextureResourceFlags(const ezGALTextureCreationDescription& desc)
{
  D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

  if (desc.m_TextureFlags.IsSet(ezGALTextureUsageFlags::RenderTarget))
  {
    if (ezGALResourceFormat::IsDepthFormat(desc.m_Format))
    {
      flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    }
    else
    {
      flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }
  }

  if (desc.m_TextureFlags.IsSet(ezGALTextureUsageFlags::UnorderedAccess))
  {
    flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  }

  return flags;
}
