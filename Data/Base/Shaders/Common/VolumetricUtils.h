#pragma once

#include "Platforms.h"

#if EZ_ENABLED(PLATFORM_SHADER)

// Henyey-Greenstein phase function
// g: asymmetry parameter in [-1, 1], 0 = isotropic, positive = forward scattering
float HenyeyGreenstein(float cosTheta, float g)
{
  float g2 = g * g;
  float denom = 1.0 + g2 - 2.0 * g * cosTheta;
  return (1.0 - g2) / (4.0 * PI * denom * sqrt(denom));
}

// Dual-lobe version for more realistic scattering
float HenyeyGreensteinDualLobe(float cosTheta, float g)
{
  return lerp(HenyeyGreenstein(cosTheta, g), HenyeyGreenstein(cosTheta, -g * 0.3), 0.2);
}

// Convert screen-space froxel coordinates to world-space position
float3 FroxelToWorldPosition(uint3 froxelCoord, uint3 gridSize, float nearPlane, float farPlane, float depthSliceScale, float depthSliceBias)
{
  // Exponential depth distribution
  float sliceNorm = (float(froxelCoord.z) + 0.5) / float(gridSize.z);
  float linearDepth = nearPlane * exp(sliceNorm * log(farPlane / nearPlane));

  // XY: normalized screen space [0, 1]
  float2 uv = (float2(froxelCoord.xy) + 0.5) / float2(gridSize.xy);

  // Convert to clip space [-1, 1]
  float2 clipXY = uv * float2(2.0, -2.0) + float2(-1.0, 1.0);

  // Reconstruct world position
  float4 clipPos = float4(clipXY * linearDepth, 1.0, linearDepth);
  float4 worldPos = mul(GetScreenToWorldMatrix(), float4(clipXY, 0.5, 1.0));
  worldPos.xyz /= worldPos.w;

  // Ray from camera
  float3 camPos = GetCameraPosition();
  float3 rayDir = normalize(worldPos.xyz - camPos);
  return camPos + rayDir * linearDepth;
}

// Convert world position to froxel UVW coordinates for texture sampling
float3 WorldToFroxelUVW(float3 worldPosition, uint3 gridSize, float nearPlane, float farPlane)
{
  float4 clipPos = mul(GetWorldToScreenMatrix(), float4(worldPosition, 1.0));
  clipPos.xyz /= clipPos.w;

  float2 uv = clipPos.xy * float2(0.5, -0.5) + 0.5;
  float linearDepth = length(worldPosition - GetCameraPosition());

  // Inverse of exponential depth
  float w = saturate(log(linearDepth / nearPlane) / log(farPlane / nearPlane));

  return float3(uv, w);
}

// Beer-Lambert extinction
float BeerLambert(float density, float stepSize)
{
  return exp(-density * stepSize);
}

// Height-based density falloff
float HeightDensity(float worldHeight, float baseHeight, float falloff)
{
  if (falloff > 0.0)
  {
    return exp(-max(0.0, worldHeight - baseHeight) * falloff);
  }
  return 1.0;
}

// Cornette-Shanks phase function (improved Henyey-Greenstein)
float CornetteShanks(float cosTheta, float g)
{
  float g2 = g * g;
  float num = 3.0 * (1.0 - g2) * (1.0 + cosTheta * cosTheta);
  float denom = 2.0 * (2.0 + g2) * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5);
  return num / (4.0 * PI * denom);
}

#endif // PLATFORM_SHADER
