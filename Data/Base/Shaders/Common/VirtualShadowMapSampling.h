#pragma once

#include "VirtualShadowMapData.h"

// Default shadow quality if not set by shader permutation (0=Low, 1=Medium, 2=High)
#if !defined(SHADOW_QUALITY)
#  define SHADOW_QUALITY 1
#endif

#if EZ_ENABLED(PLATFORM_SHADER)

// Page table textures - one per clipmap level, accessed as a Texture2DArray
Texture2DArray<uint> VSMPageTable;

// Physical shadow atlas (same format as the existing ShadowAtlasTexture)
Texture2D VSMPhysicalAtlas;

/// Samples the virtual shadow map for a directional light using the clipmap approach.
///
/// Clipmap level is selected via log2-based NDC magnitude from level 0.
/// Virtual UVs are toroidally wrapped with frac() to handle positions near clipmap boundaries.
/// Returns -1.0 when no clipmap level contains the position or the page is not allocated
/// (caller falls back to CSM).
float SampleVirtualShadowMap(float3 worldPosition, float3 vertexNormal, float3 lightVector,
  float noise, float2x2 randomRotation)
{
  // Normal offset bias: push the sample position along the vertex normal to reduce shadow acne.
  // The offset is proportional to VSMNormalBias and scales with how perpendicular the surface
  // is to the light direction (same principle as CSM's normalOffsetScale).
  float NdotL = dot(vertexNormal, lightVector);
  float normalOffsetScale = VSMNormalBias * saturate(1.0 - NdotL);
  worldPosition += vertexNormal * normalOffsetScale;

  // Project into the finest clipmap level (level 0) to determine the appropriate level.
  float4 uv0 = mul(ClipmapWorldToUV[0], float4(worldPosition, 1.0));

  // Recover NDC magnitude from the UV (ClipmapWorldToUV includes *0.5+0.5 remap).
  // For X: ndc = (uv - 0.5) / 0.5.  For Y (flipped): we only need abs, so same formula.
  float2 ndcAbs = abs(uv0.xy - 0.5) * 2.0;
  float maxNDC = max(ndcAbs.x, ndcAbs.y);

  // Each clipmap level covers 2x the area, so level = ceil(log2(max(|ndc|, 1))).
  uint bestLevel = (uint)ceil(log2(max(maxNDC, 1.0)));

  if (bestLevel >= NumClipmapLevels)
    return -1.0;

  // Use the selected level's ClipmapWorldToUV
  float4 clipmapUV4 = mul(ClipmapWorldToUV[bestLevel], float4(worldPosition, 1.0));

  // Toroidal wrapping: the sample matrix is origin-centered so UVs can exceed [0,1]
  float2 virtualUV = frac(clipmapUV4.xy);
  float shadowDepth = clipmapUV4.z;

  // Compute virtual page coordinate
  uint2 pageCoord = uint2(virtualUV * float(VSM_PAGES_PER_LEVEL));
  pageCoord = min(pageCoord, VSM_PAGES_PER_LEVEL - 1);

  // Look up the page table
  uint pageEntry = VSMPageTable.Load(int4(pageCoord, bestLevel, 0));

  uint2 physicalPageCoord;
  if (!VSMDecodePage(pageEntry, physicalPageCoord))
  {
    // Page not allocated - return sentinel so caller falls back to CSM
    return -1.0;
  }

  // Compute physical atlas UV
  float2 atlasUV = VSMVirtualToPhysicalUV(virtualUV, physicalPageCoord);

  // Apply depth bias
  shadowDepth -= VSMDepthBias;

  // PCSS sampling with page-boundary-aware clamping
  // Convert world-space light size to atlas UV space for this clipmap level
  float levelWorldSize = ClipmapLevelWorldSize[bestLevel].x;
  float lightSizeUV = VSMLightSize * float(VSM_PAGES_PER_LEVEL * VSM_PAGE_SIZE)
                    / (levelWorldSize * float(VSM_PHYSICAL_ATLAS_SIZE));
  float minPenumbra = 1.0 / float(VSM_PHYSICAL_ATLAS_SIZE);

  // Page bounds for clamping to prevent cross-page artifacts
  float2 pageMinUV = float2(physicalPageCoord) / float(VSM_PHYSICAL_ATLAS_SIZE);
  float2 pageMaxUV = float2(physicalPageCoord + uint2(VSM_PAGE_SIZE, VSM_PAGE_SIZE))
                   / float(VSM_PHYSICAL_ATLAS_SIZE);

#if SHADOW_QUALITY == 0 // LOW
  // Low: 5-tap PCF with fixed kernel, no blocker search
  const float2 lowOffsets[] = {
    {0.0f, 0.0f},
    {1.0f, 0.0f},
    {-1.0f, 0.0f},
    {0.0f, 1.0f},
    {0.0f, -1.0f},
  };

  float filterSize = max(lightSizeUV, minPenumbra) * 0.5;
  float shadowTerm = 0.0;
  [unroll] for (int i = 0; i < 5; ++i)
  {
    float2 offset = mul(randomRotation, lowOffsets[i]) * filterSize;
    float2 sampleUV = clamp(atlasUV + offset, pageMinUV, pageMaxUV);
    shadowTerm += VSMPhysicalAtlas.SampleCmpLevelZero(ShadowSampler, sampleUV, shadowDepth);
  }
  return shadowTerm / 5.0;

#elif SHADOW_QUALITY == 2 // HIGH
  // High: 8-sample blocker search + 16-sample Poisson disk PCF
  const float2 blockerOffsets[] = {
    {0.1250f, 0.0000f},
    {-0.1768f, -0.1768f},
    {0.0000f, 0.3750f},
    {0.3536f, -0.3536f},
    {-0.6250f, 0.0000f},
    {0.5303f, 0.5303f},
    {-0.0000f, -0.8750f},
    {-0.7071f, 0.7071f},
  };

  const float2 filterOffsets[] = {
    {0.0625f, 0.0000f},
    {-0.0884f, -0.0884f},
    {0.0000f, 0.1875f},
    {0.1768f, -0.1768f},
    {-0.3125f, 0.0000f},
    {0.2652f, 0.2652f},
    {-0.0000f, -0.4375f},
    {-0.3536f, 0.3536f},
    {0.5000f, 0.0625f},
    {-0.4419f, -0.1913f},
    {0.1250f, 0.5303f},
    {0.3750f, -0.4419f},
    {-0.5625f, 0.2500f},
    {0.5303f, 0.3750f},
    {-0.2500f, -0.5625f},
    {-0.5303f, 0.5303f},
  };

  float searchRadius = max(lightSizeUV * 2.0, minPenumbra);

  float blockerSum = 0.0;
  float blockerCount = 0.0;
  [unroll] for (int i = 0; i < 8; ++i)
  {
    float2 offset = mul(randomRotation, blockerOffsets[i]) * searchRadius;
    float2 sampleUV = clamp(atlasUV + offset, pageMinUV, pageMaxUV);
    float sampledDepth = VSMPhysicalAtlas.SampleLevel(PointClampSampler, sampleUV, 0).r;
    if (sampledDepth < shadowDepth)
    {
      blockerSum += sampledDepth;
      blockerCount += 1.0;
    }
  }
  if (blockerCount < 0.5) return 1.0;

  float avgBlockerDepth = blockerSum / blockerCount;
  float estimatedPenumbra = max(lightSizeUV, minPenumbra)
                          * (shadowDepth - avgBlockerDepth) / max(avgBlockerDepth, 0.0001);
  estimatedPenumbra = clamp(estimatedPenumbra, minPenumbra, searchRadius);

  float shadowTerm = 0.0;
  [unroll] for (int j = 0; j < 16; ++j)
  {
    float2 offset = mul(randomRotation, filterOffsets[j]) * estimatedPenumbra;
    float2 sampleUV = clamp(atlasUV + offset, pageMinUV, pageMaxUV);
    shadowTerm += VSMPhysicalAtlas.SampleCmpLevelZero(ShadowSampler, sampleUV, shadowDepth);
  }
  return shadowTerm / 16.0;

#else // SHADOW_QUALITY_MEDIUM (default)
  // Medium: 8-sample PCSS (blocker search + variable-width PCF)
  const float2 offsets[] = {
    {0.1250f, 0.0000f},
    {-0.1768f, -0.1768f},
    {0.0000f, 0.3750f},
    {0.3536f, -0.3536f},
    {-0.6250f, 0.0000f},
    {0.5303f, 0.5303f},
    {-0.0000f, -0.8750f},
    {-0.7071f, 0.7071f},
  };

  float searchRadius = max(lightSizeUV * 2.0, minPenumbra);

  // Step 1: Blocker search — sample raw depth to find average blocker depth
  float blockerSum = 0.0;
  float blockerCount = 0.0;
  [unroll] for (int i = 0; i < 8; ++i)
  {
    float2 offset = mul(randomRotation, offsets[i]) * searchRadius;
    float2 sampleUV = clamp(atlasUV + offset, pageMinUV, pageMaxUV);
    float sampledDepth = VSMPhysicalAtlas.SampleLevel(PointClampSampler, sampleUV, 0).r;
    if (sampledDepth < shadowDepth)
    {
      blockerSum += sampledDepth;
      blockerCount += 1.0;
    }
  }
  if (blockerCount < 0.5) return 1.0; // No blockers — fully lit

  // Step 2: Penumbra estimation
  float avgBlockerDepth = blockerSum / blockerCount;
  float estimatedPenumbra = max(lightSizeUV, minPenumbra)
                          * (shadowDepth - avgBlockerDepth) / max(avgBlockerDepth, 0.0001);
  estimatedPenumbra = clamp(estimatedPenumbra, minPenumbra, searchRadius);

  // Step 3: Variable-width PCF
  float shadowTerm = 0.0;
  [unroll] for (int j = 0; j < 8; ++j)
  {
    float2 offset = mul(randomRotation, offsets[j]) * estimatedPenumbra;
    float2 sampleUV = clamp(atlasUV + offset, pageMinUV, pageMaxUV);
    shadowTerm += VSMPhysicalAtlas.SampleCmpLevelZero(ShadowSampler, sampleUV, shadowDepth);
  }
  return shadowTerm / 8.0;
#endif
}

#endif // PLATFORM_SHADER
