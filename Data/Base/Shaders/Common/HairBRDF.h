#pragma once

#include <Shaders/Materials/MaterialData.h>

// Marschner-based hair BSDF with three lobes:
//
// R   - Primary specular reflection off the cuticle surface (white highlight).
// TT  - Transmission through the fiber, colored by hair pigment.
// TRT - Double internal reflection (secondary highlight, tinted by scatter color).
//
// Uses shifted Kajiya-Kay specular terms per lobe. The tangent direction stored
// in matData.tangentDirection defines the strand orientation.

// Longitudinal Kajiya-Kay specular term for a single lobe.
// Shifts the tangent along the normal by 'shift', then computes a Gaussian
// over the sine of the angle between the shifted tangent and the half-vector.
float HairSpecularLobe(float3 T, float3 N, float3 H, float shift, float roughness)
{
  float3 shiftedT = normalize(T + shift * N);
  float sinTH = dot(shiftedT, H);
  float cosTH2 = 1.0 - sinTH * sinTH;
  float invR2 = 1.0 / max(roughness * roughness, 1e-4);
  return exp(-cosTH2 * invR2);
}

// Full hair shading evaluation.
// Returns an AccumulatedLight struct compatible with the engine's lighting pipeline.
AccumulatedLight HairShading(ezMaterialData matData, float3 L, float3 V)
{
  float3 T = matData.tangentDirection;
  float3 N = matData.worldNormal;
  float3 H = normalize(V + L);

  float NdotL = saturate(dot(N, L));

  // Kajiya-Kay diffuse: sqrt(1 - (T.L)^2) gives a wrap-around diffuse based on tangent
  float sinTL = dot(T, L);
  float diffuseTerm = sqrt(max(1.0 - sinTL * sinTL, 0.0));

  float3 diffuse = matData.diffuseColor * diffuseTerm;

  // R lobe: primary white specular highlight
  // Shifted toward the hair root (negative shift), narrow roughness
  float R = HairSpecularLobe(T, N, H, -0.1, 0.1);
  float VdotH = saturate(dot(V, H));
  float3 colorR = R * FresnelSchlick(float3(0.04, 0.04, 0.04), VdotH);

  // TT lobe: transmission through the fiber, colored by hair pigment
  // Shifted toward the tip (positive shift), medium roughness
  float TT = HairSpecularLobe(T, N, H, 0.05, 0.15);
  float fresnelTT = 1.0 - FresnelSchlick(float3(0.04, 0.04, 0.04), VdotH).x;
  float3 colorTT = TT * matData.diffuseColor * fresnelTT * 0.5;

  // TRT lobe: double internal reflection, colored by specular (scatter) color
  // Shifted more toward root, broader roughness
  float TRT = HairSpecularLobe(T, N, H, -0.15, 0.3);
  float3 colorTRT = TRT * matData.specularColor;

  float3 specular = colorR + colorTT + colorTRT;

  return InitializeLight(diffuse * NdotL, specular * NdotL);
}
