#pragma once

#if EZ_ENABLED(PLATFORM_SHADER)

// Pre-integrated skin BRDF for direct lighting.
// Based on "Pre-Integrated Skin Shading" (Penner, Borshukov GDC 2011).
//
// Instead of using a lookup texture, this provides an analytical approximation
// that accounts for curvature-dependent subsurface light diffusion. The diffuse
// wrap term is modulated by local surface curvature so that thin features like
// nostrils and ears show more translucency while flat areas look like standard diffuse.

// Analytical approximation of the pre-integrated skin BRDF.
// NdotL: dot(normal, lightDir), range [-1, 1]
// curvature: approximate local surface curvature (higher = light wraps further around)
//
// Returns a scalar diffuse contribution that replaces the standard NdotL clamp.
float PreIntegratedSkinBRDF(float NdotL, float curvature)
{
  // Approximate the pre-integrated diffuse profile as a smooth wrap function
  // that varies with curvature. At zero curvature, this converges to the standard
  // saturate(NdotL). As curvature increases, light wraps further into the shadow region.
  float wrapWidth = saturate(curvature * 2.0);

  // Soft wrap: smoothstep-based falloff that depends on curvature
  float wrap = smoothstep(-wrapWidth, 1.0, NdotL);

  // Add a subtle red shift in the shadow-to-light transition
  // This simulates the blood/skin color shift seen in pre-integrated skin LUTs
  return wrap;
}

// RGB per-channel pre-integrated skin BRDF.
// This version provides the characteristic red shift at shadow boundaries seen in real skin.
float3 PreIntegratedSkinBRDFRGB(float NdotL, float curvature)
{
  // Each color channel has a slightly different wrap width,
  // simulating the wavelength-dependent absorption in skin
  float wrapR = saturate(curvature * 2.5);
  float wrapG = saturate(curvature * 1.8);
  float wrapB = saturate(curvature * 1.2);

  float3 result;
  result.r = smoothstep(-wrapR, 1.0, NdotL);
  result.g = smoothstep(-wrapG, 1.0, NdotL);
  result.b = smoothstep(-wrapB, 1.0, NdotL);

  return result;
}

// Approximate surface curvature from screen-space derivatives of the world normal.
// This is a cheap alternative to precomputing curvature maps.
float EstimateCurvature(float3 worldNormal, float3 worldPosition)
{
  float3 dnx = ddx(worldNormal);
  float3 dny = ddy(worldNormal);
  float3 dpx = ddx(worldPosition);
  float3 dpy = ddy(worldPosition);

  // Curvature = |dN/dp| where dp is world space. Approximate as the magnitude
  // of the normal derivative divided by the position derivative.
  float normalVariation = length(dnx) + length(dny);
  float positionVariation = max(length(dpx) + length(dpy), 0.0001);

  return saturate(normalVariation / positionVariation);
}

#endif // PLATFORM_SHADER
