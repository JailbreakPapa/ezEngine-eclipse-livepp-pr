#ifndef NOISE_HELPERS_H
#define NOISE_HELPERS_H

// Hash function for noise
float3 NoiseHash3(float3 p)
{
  p = float3(
    dot(p, float3(127.1, 311.7, 74.7)),
    dot(p, float3(269.5, 183.3, 246.1)),
    dot(p, float3(113.5, 271.9, 124.6)));
  return frac(sin(p) * 43758.5453) * 2.0 - 1.0;
}

// 3D value noise
float ValueNoise3D(float3 p)
{
  float3 i = floor(p);
  float3 f = frac(p);
  float3 u = f * f * (3.0 - 2.0 * f);

  float n000 = dot(NoiseHash3(i + float3(0, 0, 0)), f - float3(0, 0, 0));
  float n100 = dot(NoiseHash3(i + float3(1, 0, 0)), f - float3(1, 0, 0));
  float n010 = dot(NoiseHash3(i + float3(0, 1, 0)), f - float3(0, 1, 0));
  float n110 = dot(NoiseHash3(i + float3(1, 1, 0)), f - float3(1, 1, 0));
  float n001 = dot(NoiseHash3(i + float3(0, 0, 1)), f - float3(0, 0, 1));
  float n101 = dot(NoiseHash3(i + float3(1, 0, 1)), f - float3(1, 0, 1));
  float n011 = dot(NoiseHash3(i + float3(0, 1, 1)), f - float3(0, 1, 1));
  float n111 = dot(NoiseHash3(i + float3(1, 1, 1)), f - float3(1, 1, 1));

  float nx00 = lerp(n000, n100, u.x);
  float nx10 = lerp(n010, n110, u.x);
  float nx01 = lerp(n001, n101, u.x);
  float nx11 = lerp(n011, n111, u.x);

  float nxy0 = lerp(nx00, nx10, u.y);
  float nxy1 = lerp(nx01, nx11, u.y);

  return lerp(nxy0, nxy1, u.z);
}

// Curl noise for divergence-free turbulence
float3 CurlNoise3D(float3 p)
{
  float eps = 0.01;

  float nx = ValueNoise3D(p + float3(eps, 0, 0)) - ValueNoise3D(p - float3(eps, 0, 0));
  float ny = ValueNoise3D(p + float3(0, eps, 0)) - ValueNoise3D(p - float3(0, eps, 0));
  float nz = ValueNoise3D(p + float3(0, 0, eps)) - ValueNoise3D(p - float3(0, 0, eps));

  // Use offset sampling for the second component to get a 3D curl
  float3 p2 = p + float3(31.416, 17.32, 47.85);
  float nx2 = ValueNoise3D(p2 + float3(eps, 0, 0)) - ValueNoise3D(p2 - float3(eps, 0, 0));
  float ny2 = ValueNoise3D(p2 + float3(0, eps, 0)) - ValueNoise3D(p2 - float3(0, eps, 0));
  float nz2 = ValueNoise3D(p2 + float3(0, 0, eps)) - ValueNoise3D(p2 - float3(0, 0, eps));

  float inv2eps = 1.0 / (2.0 * eps);

  return float3(
    (nz2 - ny) * inv2eps,
    (nx - nz) * inv2eps,
    (ny2 - nx2) * inv2eps);
}

#endif // NOISE_HELPERS_H
