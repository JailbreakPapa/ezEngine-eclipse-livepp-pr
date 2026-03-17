#pragma once

#include <RendererDX12/RendererDX12DLL.h>
#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/Descriptors/Enumerations.h>
#include <RendererFoundation/Resources/ResourceFormats.h>

#include <d3d12.h>

/// Converts ezGAL types to their D3D12 equivalents.
///
/// Most mappings are straightforward 1:1 conversions. DXGI formats are shared between DX11 and
/// DX12, so the format table is identical. Functions that take creation descriptions (buffer,
/// texture) derive D3D12 resource flags from the usage flags set in the description.
class EZ_RENDERERDX12_DLL ezConversionUtilsDX12
{
public:
  static DXGI_FORMAT ToDXGIFormat(ezGALResourceFormat::Enum format);
  static D3D12_BLEND ToD3D12Blend(ezGALBlend::Enum blend);
  static D3D12_BLEND_OP ToD3D12BlendOp(ezGALBlendOp::Enum blendOp);
  static D3D12_COMPARISON_FUNC ToD3D12ComparisonFunc(ezGALCompareFunc::Enum compareFunc);
  static D3D12_STENCIL_OP ToD3D12StencilOp(ezGALStencilOp::Enum stencilOp);
  static D3D12_CULL_MODE ToD3D12CullMode(ezGALCullMode::Enum cullMode);
  static D3D12_PRIMITIVE_TOPOLOGY_TYPE ToD3D12PrimitiveTopologyType(ezGALPrimitiveTopology::Enum topology);
  static D3D_PRIMITIVE_TOPOLOGY ToD3DPrimitiveTopology(ezGALPrimitiveTopology::Enum topology);
  static D3D12_TEXTURE_ADDRESS_MODE ToD3D12TextureAddressMode(ezImageAddressMode::Enum addressMode);
  static D3D12_FILTER ToD3D12Filter(ezGALTextureFilterMode::Enum filterMode);
  static DXGI_SAMPLE_DESC ToD3D12SampleDesc(ezGALMSAASampleCount::Enum sampleCount);
  static UINT ToD3D12ResourceFlags(const ezGALBufferCreationDescription& desc);
  static D3D12_RESOURCE_FLAGS ToD3D12TextureResourceFlags(const ezGALTextureCreationDescription& desc);
};
