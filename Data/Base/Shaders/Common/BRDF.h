#pragma once

#include <Shaders/Materials/MaterialData.h>
#include <Shaders/Common/PreIntegratedSkin.h>

// TODO:
// check roughness^2 vs. roughness^4
// try different visibility function (SmithGGXCorrelated)

struct AccumulatedLight
{
  float3 diffuseLight;
  float3 specularLight;
};

AccumulatedLight InitializeLight(float3 diff, float3 spec)
{
  AccumulatedLight result;
  result.diffuseLight = diff;
  result.specularLight = spec;
  return result;
}

void AccumulateLight(inout AccumulatedLight result, AccumulatedLight light)
{
  result.diffuseLight += light.diffuseLight;
  result.specularLight += light.specularLight;
}

void AccumulateLight(inout AccumulatedLight result, AccumulatedLight light, float3 color)
{
  result.diffuseLight += light.diffuseLight * color;
  result.specularLight += light.specularLight * color;
}

void AccumulateLight(inout AccumulatedLight result, AccumulatedLight light, float3 color, float specularMultiplier)
{
  result.diffuseLight += light.diffuseLight * color;
  result.specularLight += (light.specularLight * color) * specularMultiplier;
}

///////////////////////////////////////////////////////////////////////////////////

float RoughnessFromMipLevel(uint mipLevel, uint mipCount)
{
  return pow(mipLevel / (float)(mipCount - 1), 2.0f);
}

float MipLevelFromRoughness(float roughness, uint mipCount)
{
  return (mipCount - 1) * sqrt(roughness);
}

float RoughnessFromPerceptualRoughness(float perceptualRoughness)
{
  return perceptualRoughness * perceptualRoughness;
}

float PerceptualRoughnessFromRoughness(float roughness)
{
  return sqrt(roughness);
}

float3 DiffuseLambert(float3 diffuseColor)
{
  return diffuseColor;
}

// divide by PI postponed
float SpecularGGX(float roughness, float NdotH)
{
  // mad friendly reformulation of:
  //
  //              a^2
  // --------------------------------
  // PI * ((N.H)^2 * (a^2 - 1) + 1)^2

  float a2 = roughness * roughness;
  float f = (NdotH * a2 - NdotH) * NdotH + 1.0f;
  return a2 / (f * f);
}

float VisibilitySmithCorrelated(float roughness, float NdotV, float NdotL)
{
  float a2 = roughness * roughness;
  float lambdaV = NdotL * sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
  float lambdaL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);
  return 0.5f / (lambdaV + lambdaL);
}

float VisibilitySmithJointApprox(float roughness, float NdotV, float NdotL)
{
  float a = roughness;
  float b = 1.0f - a;
  float lambdaV = NdotL * (NdotV * b + a);
  float lambdaL = NdotV * (NdotL * b + a);
  return 0.5f / (lambdaV + lambdaL);
}

float3 FresnelSchlick(float3 specularColor, float VdotH)
{
  float f = 1.0f - VdotH;
  float ff = f * f;
  float f5 = ff * ff * f;

  // specularColor below 2% is considered to be shadowing
  return saturate(50.0 * GetLuminance(specularColor)) * f5 + (1 - f5) * specularColor;
}

// note that 1/PI is applied later
AccumulatedLight DefaultShading(ezMaterialData matData, float3 L, float3 V)
{
  float3 N = matData.worldNormal;
  float3 H = normalize(V + L);
  float NdotL = saturate(dot(N, L));
  float NdotV = max(dot(N, V), 1e-5f);
  float NdotH = saturate(dot(N, H));
  float VdotH = saturate(dot(V, H));

  // Cook-Torrance microfacet BRDF
  //
  //    D * G * F                                  G
  //  ------------- = D * Vis * F with Vis = -------------
  //  4 * N.L * N.V                          4 * N.L * N.V

  float D = SpecularGGX(matData.roughness, NdotH);
  float Vis = VisibilitySmithJointApprox(matData.roughness, NdotV, NdotL);
  float3 F = FresnelSchlick(matData.specularColor, VdotH);

  float3 diffuse = DiffuseLambert(matData.diffuseColor);

  return InitializeLight(diffuse * NdotL, F * (D * Vis * NdotL));
}

AccumulatedLight SubsurfaceShading(ezMaterialData matData, float3 L, float3 V)
{
  float3 N = matData.worldNormal;
  float NdotL = dot(N, L);

  // Pre-integrated skin BRDF: curvature-dependent diffuse wrap with per-channel color shift.
  // At low curvature this converges to standard NdotL; at high curvature light wraps
  // further into the shadow region with a red shift simulating blood absorption in skin.
  float curvature = EstimateCurvature(N, matData.worldPosition);
  float3 sssLight = PreIntegratedSkinBRDFRGB(NdotL, curvature);

  // Forward scatter: view-dependent transmission through thin features
  float3 distortedLightDir = L + N * 0.1;
  float inScatter = pow(saturate(dot(V, -distortedLightDir)), matData.subsurfaceScatterPower);

  float3 subsurface = matData.subsurfaceColor * lerp(sssLight, 1, inScatter);

  return InitializeLight(subsurface, 0.0);
}

float3 EnvironmentBRDF(float3 specularColor, float roughness, float NoV)
{
  // [ Lazarov 2013, "Getting More Physical in Call of Duty: Black Ops II" ]
  // Adaptation to fit our G term.
  const float4 c0 = {-1, -0.0275, -0.572, 0.022};
  const float4 c1 = {1, 0.0425, 1.04, -0.04};
  float4 r = roughness * c0 + c1;
  float a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
  float2 AB = float2(-1.04, 1.04) * a004 + r.zw;

  // specularColor below 2% is considered to be shadowing
  float F90 = saturate(50.0 * GetLuminance(specularColor));

  return specularColor * AB.x + F90 * AB.y;
}

///////////////////////////////////////////////////////////////////////////////////
// Area Light Helpers
// Based on Wicked Engine's implementation (Turánszki) which follows
// Karis 2013 (MRP), Lagarde & de Rousiers 2014 (Frostbite rect diffuse).
///////////////////////////////////////////////////////////////////////////////////

/// Projects point P onto the line segment from A to B; returns the closest point on the segment.
float3 ClosestPointOnSegment(float3 P, float3 A, float3 B)
{
  float3 AB = B - A;
  float t = saturate(dot(P - A, AB) / dot(AB, AB));
  return A + t * AB;
}

/// Projects a point onto a plane defined by an origin and normal.
float3 PointOnPlane(float3 P, float3 planeOrigin, float3 planeNormal)
{
  float dist = dot(P - planeOrigin, planeNormal);
  return P - dist * planeNormal;
}

/// Intersects a ray (origin o, direction d) with a plane. Returns the ray parameter t.
float TracePlane(float3 o, float3 d, float3 planeOrigin, float3 planeNormal)
{
  return dot(planeNormal, (planeOrigin - o) / dot(planeNormal, d));
}

/// Fast polynomial approximation of acos.
float AcosFast(float x)
{
  float y = abs(x);
  float p = -0.1565827 * y + 1.570796;
  p *= sqrt(1.0 - y);
  return x >= 0.0 ? p : PI - p;
}

/// Smooth windowing attenuation: saturate(1 - (d^2/r^2)^2) / max(eps, d^2).
/// Provides inverse-square falloff with a smooth fade to zero at the range boundary.
float AttenuationPointLight(float dist2, float range2)
{
  float distPerRange = dist2 / range2;
  distPerRange *= distPerRange;
  return saturate(1.0 - distPerRange) / max(0.0001, dist2);
}

/// Evaluates shading for a rect light.
///
/// Diffuse uses the Frostbite solid angle method (Lagarde & de Rousiers 2014):
/// the subtended solid angle of the rectangle weighted by averaged corner visibility.
/// Specular uses the reflection-plane intersection clamped to the rectangle bounds.
AccumulatedLight RectLightShading(ezMaterialData matData, float3 V, float3 lightPos,
  float3 lightForward, float3 lightRight, float3 lightUp, float halfWidth, float halfHeight)
{
  float3 N = matData.worldNormal;
  float3 worldPos = matData.worldPosition;

  // Front-face check: rect emits from front only
  if (dot(worldPos - lightPos, lightForward) <= 0)
    return InitializeLight(0, 0);

  // Rectangle corners
  float3 p0 = lightPos - lightRight * halfWidth + lightUp * halfHeight;
  float3 p1 = lightPos + lightRight * halfWidth + lightUp * halfHeight;
  float3 p2 = lightPos + lightRight * halfWidth - lightUp * halfHeight;
  float3 p3 = lightPos - lightRight * halfWidth - lightUp * halfHeight;

  // Closest point on rectangle for attenuation
  float3 closestOnPlane = PointOnPlane(worldPos, lightPos, lightForward);
  float3 toClosest = closestOnPlane - lightPos;
  float2 planeCoord = float2(dot(toClosest, lightRight), dot(toClosest, lightUp));
  float2 clamped = float2(
    clamp(planeCoord.x, -halfWidth, halfWidth),
    clamp(planeCoord.y, -halfHeight, halfHeight));
  float3 rectPoint = lightPos + lightRight * clamped.x + lightUp * clamped.y;
  float3 Lunnormalized = rectPoint - worldPos;
  float dist2 = dot(Lunnormalized, Lunnormalized);
  float3 L = Lunnormalized / max(sqrt(dist2), 1e-5);
  float NdotL = saturate(dot(N, L));

  // --- Diffuse: Frostbite solid angle method ---
  float3 v0 = normalize(p0 - worldPos);
  float3 v1 = normalize(p1 - worldPos);
  float3 v2 = normalize(p2 - worldPos);
  float3 v3 = normalize(p3 - worldPos);

  float3 n0 = normalize(cross(v0, v1));
  float3 n1 = normalize(cross(v1, v2));
  float3 n2 = normalize(cross(v2, v3));
  float3 n3 = normalize(cross(v3, v0));

  float g0 = AcosFast(dot(-n0, n1));
  float g1 = AcosFast(dot(-n1, n2));
  float g2 = AcosFast(dot(-n2, n3));
  float g3 = AcosFast(dot(-n3, n0));

  float solidAngle = saturate(g0 + g1 + g2 + g3 - 2.0 * PI);

  // Average visibility of corners + closest-point direction with surface normal
  float diffNdotL = solidAngle * 0.2 * (
    saturate(dot(v0, N)) +
    saturate(dot(v1, N)) +
    saturate(dot(v2, N)) +
    saturate(dot(v3, N)) +
    NdotL);

  float3 diffuse = DiffuseLambert(matData.diffuseColor) * diffNdotL;

  // --- Specular: reflection-plane intersection clamped to rectangle ---
  float3 R = reflect(-V, N);
  float traceT = TracePlane(worldPos, R, lightPos, lightForward);
  float3 intersectPoint = worldPos + R * traceT;
  float3 intersectVec = intersectPoint - lightPos;
  float2 intersect2D = float2(dot(intersectVec, lightRight), dot(intersectVec, lightUp));
  float2 nearest2D = float2(
    clamp(intersect2D.x, -halfWidth, halfWidth),
    clamp(intersect2D.y, -halfHeight, halfHeight));
  float3 specRepPt = lightPos + lightRight * nearest2D.x + lightUp * nearest2D.y;

  float3 Lspec = specRepPt - worldPos;
  float specDist = length(Lspec);
  Lspec /= max(specDist, 1e-5);

  float NdotV = max(dot(N, V), 1e-5);
  float specNdotL = saturate(dot(N, Lspec));
  float3 H = normalize(V + Lspec);
  float NdotH = saturate(dot(N, H));
  float VdotH = saturate(dot(V, H));

  float roughnessBRDF = matData.roughness * matData.roughness;
  roughnessBRDF = max(roughnessBRDF, 0.001);
  float D = SpecularGGX(roughnessBRDF, NdotH);
  float Vis = VisibilitySmithJointApprox(roughnessBRDF, NdotV, specNdotL);
  float3 F = FresnelSchlick(matData.specularColor, VdotH);

  float3 specular = F * (D * Vis * specNdotL);

  return InitializeLight(diffuse, specular);
}

/// Evaluates shading for a tube (capsule) light.
///
/// Diffuse uses the closest point on the tube axis. Specular projects the
/// reflection ray onto the line segment, then offsets by the tube radius
/// toward the reflection ray (sphere representative point technique).
AccumulatedLight TubeLightShading(ezMaterialData matData, float3 V, float3 lightPos,
  float3 lightAxis, float halfLength, float radius)
{
  float3 N = matData.worldNormal;
  float3 worldPos = matData.worldPosition;
  float3 R = reflect(-V, N);

  float3 endA = lightPos - lightAxis * halfLength;
  float3 endB = lightPos + lightAxis * halfLength;

  // --- Diffuse: closest point on tube axis to surface ---
  float3 closestOnAxis = ClosestPointOnSegment(worldPos, endA, endB);
  float3 Lunnormalized = closestOnAxis - worldPos;
  float3 Ldiff = normalize(Lunnormalized);
  float diffNdotL = saturate(dot(N, Ldiff));

  float3 diffuse = DiffuseLambert(matData.diffuseColor) * diffNdotL;

  // --- Specular: project reflection ray onto tube segment ---
  // Find point on segment closest to the reflection ray (Karis 2013 / Picott 1992)
  float3 L0 = endA - worldPos;
  float3 L1 = endB - worldPos;
  float3 Ld = L1 - L0;
  float RdotLd = dot(R, Ld);
  float t = dot(R, L0) * RdotLd - dot(L0, Ld);
  t /= dot(Ld, Ld) - RdotLd * RdotLd;
  Lunnormalized = L0 + saturate(t) * Ld;

  // Offset by tube radius toward the reflection ray (sphere representative point)
  if (radius > 0)
  {
    float3 centerToRay = mad(dot(Lunnormalized, R), R, -Lunnormalized);
    Lunnormalized = mad(centerToRay, saturate(radius / length(centerToRay)), Lunnormalized);
  }

  float3 Lspec = normalize(Lunnormalized);

  float NdotV = max(dot(N, V), 1e-5);
  float NdotL = saturate(dot(N, Lspec));
  float3 H = normalize(V + Lspec);
  float NdotH = saturate(dot(N, H));
  float VdotH = saturate(dot(V, H));

  float roughnessBRDF = matData.roughness * matData.roughness;
  roughnessBRDF = max(roughnessBRDF, 0.001);
  float D = SpecularGGX(roughnessBRDF, NdotH);
  float Vis = VisibilitySmithJointApprox(roughnessBRDF, NdotV, NdotL);
  float3 F = FresnelSchlick(matData.specularColor, VdotH);

  float3 specular = F * (D * Vis * NdotL);

  return InitializeLight(diffuse, specular);
}
