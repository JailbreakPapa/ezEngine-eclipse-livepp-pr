#pragma once

#include "VirtualShadowMapData.h"

#if EZ_ENABLED(PLATFORM_SHADER)

// Page table textures - one per clipmap level, accessed as a Texture2DArray
Texture2DArray<uint> VSMPageTable;

// Physical shadow atlas (same format as the existing ShadowAtlasTexture)
Texture2D VSMPhysicalAtlas;

/// Samples the virtual shadow map for a directional light using the clipmap approach.
///
/// Finds the best clipmap level for the given world position, looks up the page
/// table to find the physical page, then samples the physical shadow atlas.
/// Returns -1.0 when no clipmap level contains the position or the page is not allocated
/// (caller falls back to CSM).
float SampleVirtualShadowMap(float3 worldPosition, float3 vertexNormal, float3 lightVector,
  float noise, float2x2 randomRotation)
{
  // Find the best clipmap level by projecting through each level's matrix
  // and checking if the UV falls within [0,1]. Smallest level first.
  float2 virtualUV;
  float shadowDepth;
  uint bestLevel = NumClipmapLevels;

  for (uint level = 0; level < NumClipmapLevels; ++level)
  {
    float4 clipmapUV4 = mul(ClipmapWorldToUV[level], float4(worldPosition, 1.0));
    if (all(clipmapUV4.xy >= 0.0) && all(clipmapUV4.xy <= 1.0))
    {
      virtualUV = clipmapUV4.xy;
      shadowDepth = clipmapUV4.z;
      bestLevel = level;
      break;
    }
  }

  // No clipmap level contains this position — fall back to CSM
  if (bestLevel >= NumClipmapLevels)
    return -1.0;

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

  // Apply normal bias
  float normalBias = VSMNormalBias;
  float normalOffsetScale = normalBias - dot(vertexNormal, lightVector) * normalBias;
  shadowDepth -= VSMDepthBias;

  // PCF sampling using the same spiral pattern as the existing SampleShadow
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

  float penumbraSize = 1.0 / float(VSM_PHYSICAL_ATLAS_SIZE);
  float shadowTerm = 0.0;

  for (int i = 0; i < 8; ++i)
  {
    float2 offset = mul(randomRotation, offsets[i]) * penumbraSize;
    float2 sampleUV = atlasUV + offset;
    shadowTerm += VSMPhysicalAtlas.SampleCmpLevelZero(ShadowSampler, sampleUV, shadowDepth);
  }

  return shadowTerm / 8.0;
}

#endif // PLATFORM_SHADER
