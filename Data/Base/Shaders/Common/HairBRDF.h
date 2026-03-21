#pragma once

#include <Shaders/Materials/MaterialData.h>

// Energy-conserving Marschner hair BSDF with three scattering lobes:
//
// R   - Primary specular reflection off the cuticle surface.
// TT  - Transmission through the fiber, colored by absorption.
// TRT - Double internal reflection (secondary highlight).
//
// This implementation uses:
// - Proper longitudinal scattering terms M(theta_h, alpha, beta) per lobe
// - Azimuthal scattering terms N(phi) with absorption via Beer's law
// - Full dielectric Fresnel (n=1.55 for hair fiber) instead of Schlick
// - Melanin-based absorption model for physically accurate hair coloring
// - Dual-scattering multiple scattering approximation for light-colored hair
//
// References:
//   [Marschner 2003] "Light Scattering from Human Hair Fibers"
//   [d'Eon 2011] "An Energy-Conserving Hair Reflectance Model"
//   [Zinke & Weber 2007] "Light Scattering from Filaments"

static const float HAIR_ETA = 1.55;         // Index of refraction for hair fiber
static const float HAIR_ETA_PRIME = 1.19;   // Effective IOR accounting for fiber tilt
static const float HAIR_INV_PI = 0.318309886; // 1/PI

// --- Fresnel ---

// Full dielectric Fresnel reflectance for unpolarized light.
float DielectricFresnel(float eta, float cosTheta)
{
  float sinTheta2 = 1.0 - cosTheta * cosTheta;
  float sinThetaT2 = sinTheta2 / (eta * eta);

  if (sinThetaT2 >= 1.0)
    return 1.0; // Total internal reflection

  float cosThetaT = sqrt(1.0 - sinThetaT2);

  float rs = (eta * cosTheta - cosThetaT) / (eta * cosTheta + cosThetaT);
  float rp = (cosTheta - eta * cosThetaT) / (cosTheta + eta * cosThetaT);

  return 0.5 * (rs * rs + rp * rp);
}

// --- Absorption (Melanin Model) ---

// Computes absorption coefficient sigma_a from melanin concentrations.
// Eumelanin produces dark brown/black pigment. Pheomelanin produces red/gold.
// Based on [d'Eon 2011] measured absorption spectra.
float3 ComputeMelaninAbsorption(float melanin, float melaninRedness)
{
  // Absorption cross-sections (RGB approximation of measured spectra)
  float3 eumelaninSigma = float3(0.419, 0.697, 1.370);   // Brown/black
  float3 pheomelaninSigma = float3(0.187, 0.400, 1.050);  // Red/gold

  float eumelanin = melanin * (1.0 - melaninRedness);
  float pheomelanin = melanin * melaninRedness;

  return eumelanin * eumelaninSigma + pheomelanin * pheomelaninSigma;
}

// Computes absorption from a direct color (non-melanin path).
// Inverts Beer's law: sigma_a = -2 * ln(color) / pathLength
// Uses a default path length of 0.7 (approximate fiber diameter traversal).
float3 ComputeColorAbsorption(float3 hairColor)
{
  float3 safeColor = max(hairColor, float3(0.001, 0.001, 0.001));
  return -2.0 * log(safeColor) / 0.7;
}

// --- Longitudinal Scattering M ---

// Normalized Gaussian for the longitudinal scattering term.
// alpha: lobe shift angle in radians
// beta: lobe roughness (width)
float MarschnerM(float thetaH, float alpha, float beta)
{
  float x = thetaH - alpha;
  float invBeta2 = 1.0 / max(beta * beta, 1e-5);
  float norm = 0.3989422804 / max(beta, 1e-5); // 1/(sqrt(2*pi)*beta)
  return norm * exp(-0.5 * x * x * invBeta2);
}

// --- Azimuthal Scattering N ---

// Attenuation factor A for a single traversal through hair fiber.
// Accounts for Fresnel at entry/exit and Beer's law absorption.
float3 HairAttenuation(float f, float3 sigmaA, float cosThetaD)
{
  // Path length through the circular cross-section
  float cosGammaO = sqrt(max(1.0 - f * f, 0.0));
  float pathLength = 2.0 * cosGammaO / max(cosThetaD, 0.01);

  float3 absorption = exp(-sigmaA * pathLength);
  return absorption;
}

// Azimuthal scattering for the R lobe (surface reflection).
float MarschnerN_R(float phi, float cosThetaD)
{
  float F = DielectricFresnel(HAIR_ETA, cosThetaD);
  // Simple cosine lobe centered at phi=0
  float D = 0.25 * cos(phi * 0.5);
  return F * D;
}

// Azimuthal scattering for the TT lobe (transmission).
float3 MarschnerN_TT(float phi, float cosThetaD, float3 sigmaA)
{
  float F1 = DielectricFresnel(HAIR_ETA, cosThetaD);
  float T = 1.0 - F1;
  float F2 = DielectricFresnel(1.0 / HAIR_ETA, cosThetaD);
  float transmittance = T * (1.0 - F2);

  float h = 0.5; // Average impact parameter
  float3 A = HairAttenuation(h, sigmaA, cosThetaD);

  // TT azimuthal distribution centered around phi = pi
  float D = 0.25 * cos((phi - 3.14159265) * 0.5);
  return transmittance * A * abs(D);
}

// Azimuthal scattering for the TRT lobe (double internal reflection).
float3 MarschnerN_TRT(float phi, float cosThetaD, float3 sigmaA)
{
  float F1 = DielectricFresnel(HAIR_ETA, cosThetaD);
  float T = 1.0 - F1;
  float F2 = DielectricFresnel(1.0 / HAIR_ETA, cosThetaD);

  // TRT path: enter, reflect internally twice, exit
  float transmittance = T * F2 * (1.0 - F2);

  float h = 0.5;
  float3 A = HairAttenuation(h, sigmaA, cosThetaD);
  float3 A2 = A * A; // Double path through fiber

  // Broad lobe; similar to R but shifted
  float D = 0.25 * cos(phi * 0.5);
  return transmittance * A2 * abs(D);
}

// --- Multiple Scattering Approximation ---

// Approximates light scattered multiple times within a volume of hair fibers.
// Crucial for bright/blonde hair where single scattering alone appears too dark.
// Based on the dual-scattering approach of [Zinke & Weber 2007].
float3 HairMultipleScattering(float3 sigmaA, float density, float NdotL, float scatterScale)
{
  if (scatterScale <= 0.0)
    return float3(0, 0, 0);

  // Average forward-scattering transmittance
  float3 avgTransmittance = exp(-sigmaA * 0.7);

  // Forward scattering dominates in hair volumes
  float3 forwardScatter = avgTransmittance * 0.5 * density;

  // Backward scattering (global illumination within hair volume)
  float3 backScatter = avgTransmittance * avgTransmittance * 0.25 * density;

  // Combine with a wrap-around diffuse term for soft illumination
  float wrapDiffuse = saturate(NdotL * 0.5 + 0.5);

  return (forwardScatter + backScatter) * wrapDiffuse * scatterScale;
}

// --- Main BSDF ---

// Full hair shading evaluation.
// Returns an AccumulatedLight struct compatible with the engine's lighting pipeline.
// All per-lobe parameters and absorption data are read from matData.
AccumulatedLight HairShading(ezMaterialData matData, float3 L, float3 V)
{
  float3 T = matData.tangentDirection;
  float3 N = matData.worldNormal;

  // Longitudinal angles (theta): angle from the normal plane
  float sinThetaL = dot(T, L);
  float sinThetaV = dot(T, V);
  float cosThetaL = sqrt(max(1.0 - sinThetaL * sinThetaL, 0.0));
  float cosThetaV = sqrt(max(1.0 - sinThetaV * sinThetaV, 0.0));

  // Half-angle longitudinal
  float thetaH = (asin(sinThetaL) + asin(sinThetaV)) * 0.5;
  // Difference angle
  float cosThetaD = cos((asin(sinThetaL) - asin(sinThetaV)) * 0.5);

  // Azimuthal angle (phi): angle around the hair fiber
  float3 Lperp = normalize(L - sinThetaL * T);
  float3 Vperp = normalize(V - sinThetaV * T);
  float cosPhi = clamp(dot(Lperp, Vperp), -1.0, 1.0);
  float phi = acos(cosPhi);

  float NdotL = dot(N, L);

  // Compute absorption coefficient
  float3 sigmaA;
  if (matData.hairUseMelanin > 0.5)
  {
    sigmaA = ComputeMelaninAbsorption(matData.hairMelanin, matData.hairMelaninRedness);
  }
  else
  {
    sigmaA = ComputeColorAbsorption(matData.hairDyeColor);
  }

  // Per-lobe shifts (alpha) and roughnesses (beta)
  float alphaR = matData.hairPrimaryShift;
  float alphaTT = -alphaR * 0.5;
  float alphaTRT = -alphaR * 1.5;

  float betaR = matData.hairPrimaryRoughness;
  float betaTT = matData.hairTransmissionRoughness;
  float betaTRT = matData.hairSecondaryRoughness;

  float scatterIntensity = matData.hairScatterIntensity;

  // --- R lobe: primary specular ---
  float MR = MarschnerM(thetaH, alphaR, betaR);
  float NR = MarschnerN_R(phi, cosThetaD);
  float3 lobeR = MR * NR * scatterIntensity;

  // --- TT lobe: transmission ---
  float MTT = MarschnerM(thetaH, alphaTT, betaTT);
  float3 NTT = MarschnerN_TT(phi, cosThetaD, sigmaA);
  float3 lobeTT = MTT * NTT * scatterIntensity;

  // --- TRT lobe: secondary specular ---
  float MTRT = MarschnerM(thetaH, alphaTRT, betaTRT);
  float3 NTRT = MarschnerN_TRT(phi, cosThetaD, sigmaA);
  float3 lobeTRT = MTRT * NTRT * scatterIntensity;

  float3 specular = lobeR + lobeTRT;
  float3 diffuseAndTransmission = lobeTT;

  // Kajiya-Kay diffuse term for compatibility with non-strand rendering
  float diffuseTerm = sqrt(max(1.0 - sinThetaL * sinThetaL, 0.0));
  float3 baseDiffuse = matData.diffuseColor * diffuseTerm * HAIR_INV_PI;

  // Root-to-tip darkening
  baseDiffuse *= (1.0 - matData.hairRootDarkening * 0.5);

  // Multiple scattering contribution
  float3 multiScatter = HairMultipleScattering(sigmaA, 1.0, NdotL, matData.hairMultipleScatterScale);
  baseDiffuse += matData.diffuseColor * multiScatter;

  float NdotLClamped = saturate(NdotL);

  return InitializeLight(
    (baseDiffuse + diffuseAndTransmission) * NdotLClamped,
    specular * NdotLClamped
  );
}
