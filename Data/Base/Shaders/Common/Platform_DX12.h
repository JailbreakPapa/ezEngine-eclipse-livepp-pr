#pragma once

#define PLATFORM_DX12 EZ_OFF

#if defined(DX12)

#  undef PLATFORM_SHADER
#  define PLATFORM_SHADER EZ_ON

#  undef PLATFORM_DX12
#  define PLATFORM_DX12 EZ_ON

// DX12 uses root constants for push constants, but from the shader's perspective
// they look like a normal constant buffer bound to the draw call bind group.
#  define BEGIN_PUSH_CONSTANTS(Name) cbuffer Name BIND_GROUP(BG_DRAW_CALL)
#  define END_PUSH_CONSTANTS(Name) ;
#  define GET_PUSH_CONSTANT(Name, Constant) Constant

// Material data uses a structured buffer (same layout as Vulkan backend).
#  define BEGIN_MATERIAL_CONSTANTS struct ezMaterialConstants
#  define END_MATERIAL_CONSTANTS \
    ;                            \
    StructuredBuffer<ezMaterialConstants> materialData BIND_RESOURCE(0, BG_MATERIAL);
#  define GetMaterialData(x) materialData[0].x

#  define SUPPORTS_TEXEL_BUFFER EZ_ON
#  define SUPPORTS_MSAA_ARRAYS EZ_ON

float ezEvaluateAttributeAtSample(float Attribute, uint SampleIndex, uint NumMsaaSamples)
{
  return EvaluateAttributeAtSample(Attribute, SampleIndex);
}
float2 ezEvaluateAttributeAtSample(float2 Attribute, uint SampleIndex, uint NumMsaaSamples)
{
  return EvaluateAttributeAtSample(Attribute, SampleIndex);
}
float3 ezEvaluateAttributeAtSample(float3 Attribute, uint SampleIndex, uint NumMsaaSamples)
{
  return EvaluateAttributeAtSample(Attribute, SampleIndex);
}
float4 ezEvaluateAttributeAtSample(float4 Attribute, uint SampleIndex, uint NumMsaaSamples)
{
  return EvaluateAttributeAtSample(Attribute, SampleIndex);
}

// select() polyfill for SM < 6.6 compatibility.
float2 select(bool2 condition, float2 yes, float2 no)
{
  return float2(condition.x ? yes.x : no.x, condition.y ? yes.y : no.y);
}

float3 select(bool3 condition, float3 yes, float3 no)
{
  return float3(condition.x ? yes.x : no.x, condition.y ? yes.y : no.y, condition.z ? yes.z : no.z);
}

float4 select(bool4 condition, float4 yes, float4 no)
{
  return float4(condition.x ? yes.x : no.x, condition.y ? yes.y : no.y, condition.z ? yes.z : no.z, condition.w ? yes.w : no.w);
}

float4 ezSampleLevel_PointClampBorder(Texture2DArray DepthTexture, SamplerState DepthSampler, float2 SamplePos, int ArrayIndex, int MipLevel, float4 BorderColor)
{
  return DepthTexture.SampleLevel(DepthSampler, float3(SamplePos, ArrayIndex), MipLevel);
}
#endif
