#pragma once

#include "Platforms.h"

#if EZ_ENABLED(PLATFORM_SHADER)

// GBuffer layout:
// RT0 (RGBA8):    Albedo.rgb + Metallic
// RT1 (RGB10A2):  Octahedral Normal (10+10 bits) + Roughness (10 bits) + Material Flags (2 bits)
// RT2 (RGBA8):    Emission.rgb + AO

// Material flag bits (in RT1.a, 2 bits)
// 0 = default (non-SSS), 1-3 = SSS with profile index (value - 1)
#define GBUFFER_MATFLAG_DEFAULT     0
#define GBUFFER_MATFLAG_SSS         1  // SSS profile 0 (skin)
#define GBUFFER_MATFLAG_SSS_PROFILE1 2 // SSS profile 1 (wax)
#define GBUFFER_MATFLAG_SSS_PROFILE2 3 // SSS profile 2 (jade)

// --- Octahedral normal encoding ---
// Maps a unit normal to a 2D coordinate in [0, 1]^2
// Reference: "A Survey of Efficient Representations for Independent Unit Vectors" (Cigolle et al. 2014)

float2 OctahedralWrap(float2 v)
{
  return (1.0 - abs(v.yx)) * select(v.xy >= 0.0, float2(1.0, 1.0), float2(-1.0, -1.0));
}

float2 EncodeNormalOctahedral(float3 n)
{
  n /= (abs(n.x) + abs(n.y) + abs(n.z));
  n.xy = n.z >= 0.0 ? n.xy : OctahedralWrap(n.xy);
  n.xy = n.xy * 0.5 + 0.5;
  return n.xy;
}

float3 DecodeNormalOctahedral(float2 encodedNormal)
{
  encodedNormal = encodedNormal * 2.0 - 1.0;
  float3 n = float3(encodedNormal.xy, 1.0 - abs(encodedNormal.x) - abs(encodedNormal.y));
  float t = saturate(-n.z);
  n.xy += select(n.xy >= 0.0, float2(-t, -t), float2(t, t));
  return normalize(n);
}

// --- GBuffer encode/decode functions ---

struct GBufferData
{
  float3 albedo;
  float metallic;
  float3 normal;
  float roughness;
  float3 emission;
  float occlusion;
  uint materialFlags;
};

struct GBufferOutput
{
  float4 RT0 : SV_Target0; // Albedo.rgb + Metallic
  float4 RT1 : SV_Target1; // Octahedral Normal.xy + Roughness + Material Flags (packed as RGB10A2)
  float4 RT2 : SV_Target2; // Emission.rgb + AO
};

GBufferOutput EncodeGBuffer(GBufferData data)
{
  GBufferOutput output;

  // RT0: albedo + metallic
  output.RT0 = float4(data.albedo, data.metallic);

  // RT1: octahedral normal + roughness + material flags
  // For RGB10A2 format: xy = normal (10 bits each), z = roughness (10 bits), w = flags (2 bits)
  float2 encNormal = EncodeNormalOctahedral(data.normal);
  output.RT1 = float4(encNormal, data.roughness, float(data.materialFlags) / 3.0);

  // RT2: emission + AO
  output.RT2 = float4(data.emission, data.occlusion);

  return output;
}

GBufferData DecodeGBuffer(float4 rt0, float4 rt1, float4 rt2)
{
  GBufferData data;

  data.albedo = rt0.rgb;
  data.metallic = rt0.a;

  data.normal = DecodeNormalOctahedral(rt1.xy);
  data.roughness = rt1.z;
  data.materialFlags = uint(rt1.w * 3.0 + 0.5);

  data.emission = rt2.rgb;
  data.occlusion = rt2.a;

  return data;
}

#endif // PLATFORM_SHADER
