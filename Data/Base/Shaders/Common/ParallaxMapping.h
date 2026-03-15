#pragma once

/// Performs parallax occlusion mapping using steep parallax with binary search refinement.
///
/// \param heightTex Height map texture where white (1.0) is the surface and black (0.0) is the deepest point.
/// \param samp Sampler state for the height texture.
/// \param texCoords Original UV coordinates.
/// \param viewDirTS View direction in tangent space (must be normalized).
/// \param heightScale Maximum displacement depth in UV space.
/// \param maxSteps Upper bound on ray march steps. Actual count varies with view angle.
/// \return Parallax-offset UV coordinates.
float2 ParallaxOcclusionMapping(Texture2D heightTex, SamplerState samp, float2 texCoords, float3 viewDirTS, float heightScale, int maxSteps)
{
  // Fewer steps when looking straight down (viewDirTS.z close to 1), more at grazing angles
  int numSteps = (int)lerp((float)maxSteps, (float)max(maxSteps / 4, 4), abs(viewDirTS.z));

  float layerDepth = 1.0 / (float)numSteps;
  float currentLayerDepth = 0.0;

  // UV offset per step, scaled by height and projected by view angle
  float2 deltaUV = (viewDirTS.xy / max(viewDirTS.z, 0.001)) * heightScale / (float)numSteps;

  float2 currentUV = texCoords;
  float currentHeight = heightTex.SampleLevel(samp, currentUV, 0).r;

  // Steep parallax: march until we go below the surface
  [loop]
  for (int i = 0; i < maxSteps && currentLayerDepth < currentHeight; ++i)
  {
    currentUV -= deltaUV;
    currentHeight = heightTex.SampleLevel(samp, currentUV, 0).r;
    currentLayerDepth += layerDepth;
  }

  // Binary search refinement between the last two steps for sub-step precision
  float2 prevUV = currentUV + deltaUV;
  float prevLayerDepth = currentLayerDepth - layerDepth;

  float2 refinedUV = currentUV;

  [unroll]
  for (int j = 0; j < 4; ++j)
  {
    float2 midUV = (prevUV + currentUV) * 0.5;
    float midLayerDepth = (prevLayerDepth + currentLayerDepth) * 0.5;
    float midHeight = heightTex.SampleLevel(samp, midUV, 0).r;

    if (midHeight > midLayerDepth)
    {
      // Still above surface, step further in
      prevUV = midUV;
      prevLayerDepth = midLayerDepth;
    }
    else
    {
      // Below surface, step back
      currentUV = midUV;
      currentLayerDepth = midLayerDepth;
    }

    refinedUV = midUV;
  }

  return refinedUV;
}
