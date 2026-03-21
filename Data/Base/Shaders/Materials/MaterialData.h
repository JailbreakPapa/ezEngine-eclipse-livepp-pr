#pragma once

#include <Shaders/Common/Common.h>

struct ezMaterialData
{
  float3 worldPosition;
  float3 worldNormal;
  float3 vertexNormal;

  float3 diffuseColor;
  float3 specularColor;
  float3 emissiveColor;
  float4 refractionColor;
  float roughness;
  float perceptualRoughness;
  float occlusion;
  float opacity;

  float3 subsurfaceColor;
  float subsurfaceScatterPower;
  float subsurfaceShadowFalloff;
  uint subsurfaceProfileIndex; // 0=skin, 1=wax, 2=jade, 3=milk

  float3 tangentDirection;

  // Hair BRDF parameters, set by FinalizeMaterial when USE_HAIR_SHADING is defined.
  float hairPrimaryShift;
  float hairSecondaryShift;
  float hairPrimaryRoughness;
  float hairSecondaryRoughness;
  float hairTransmissionRoughness;
  float hairSpecularIntensity;
  float hairRootDarkening;

  // Physically-based hair color via melanin absorption model.
  // When hairUseMelanin is true, the BRDF computes absorption coefficients from melanin
  // concentrations instead of using diffuseColor directly.
  float hairMelanin;               // Eumelanin concentration [0,1] (dark brown/black pigment)
  float hairMelaninRedness;        // Pheomelanin ratio [0,1] (red/blonde tones)
  float3 hairDyeColor;             // Custom override color when not using melanin model
  float hairUseMelanin;            // > 0.5 to use melanin-based absorption
  float hairMultipleScatterScale;  // Intensity of multiple scattering approximation
  float hairScatterIntensity;      // Overall scatter/specular brightness multiplier
};
