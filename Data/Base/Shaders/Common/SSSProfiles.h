#pragma once

#if EZ_ENABLED(PLATFORM_SHADER)

// Separable subsurface scattering diffusion profiles.
// Based on "Separable Subsurface Scattering" (Jimenez et al. 2015).
//
// Each profile is a set of Gaussian kernel weights that approximate the diffusion
// of light within a specific material. The kernel is applied as a separable (horizontal + vertical)
// screen-space blur, weighted by depth similarity to avoid bleeding across edges.

// Profile data: each entry is (weight, variance) for a sum-of-Gaussians fit
// to the diffusion profile. RGB channels have independent variances.

struct SSSKernelSample
{
  float3 weight;
  float offset;
};

// Built-in skin diffusion profile kernel (25 samples).
// Fitted to a sum of 6 Gaussians per channel from the Christensen-Burley model.
// Reference: "Approximating Translucent Appearance" (Christensen, Burley 2015)
static const SSSKernelSample SkinKernel[SSS_MAX_KERNEL_SIZE] =
{
  { float3(0.530, 0.450, 0.340),  0.000 },
  { float3(0.048, 0.042, 0.030),  0.080 },
  { float3(0.048, 0.042, 0.030), -0.080 },
  { float3(0.040, 0.038, 0.025),  0.160 },
  { float3(0.040, 0.038, 0.025), -0.160 },
  { float3(0.032, 0.032, 0.020),  0.280 },
  { float3(0.032, 0.032, 0.020), -0.280 },
  { float3(0.024, 0.025, 0.016),  0.440 },
  { float3(0.024, 0.025, 0.016), -0.440 },
  { float3(0.018, 0.020, 0.012),  0.640 },
  { float3(0.018, 0.020, 0.012), -0.640 },
  { float3(0.013, 0.015, 0.009),  0.880 },
  { float3(0.013, 0.015, 0.009), -0.880 },
  { float3(0.009, 0.012, 0.007),  1.160 },
  { float3(0.009, 0.012, 0.007), -1.160 },
  { float3(0.006, 0.009, 0.005),  1.480 },
  { float3(0.006, 0.009, 0.005), -1.480 },
  { float3(0.004, 0.006, 0.003),  1.840 },
  { float3(0.004, 0.006, 0.003), -1.840 },
  { float3(0.003, 0.004, 0.002),  2.240 },
  { float3(0.003, 0.004, 0.002), -2.240 },
  { float3(0.002, 0.003, 0.002),  2.680 },
  { float3(0.002, 0.003, 0.002), -2.680 },
  { float3(0.001, 0.002, 0.001),  3.160 },
  { float3(0.001, 0.002, 0.001), -3.160 },
};

// Marble/wax profile: highly translucent, warm tones
static const SSSKernelSample WaxKernel[SSS_MAX_KERNEL_SIZE] =
{
  { float3(0.440, 0.380, 0.280),  0.000 },
  { float3(0.058, 0.052, 0.035),  0.100 },
  { float3(0.058, 0.052, 0.035), -0.100 },
  { float3(0.050, 0.046, 0.028),  0.220 },
  { float3(0.050, 0.046, 0.028), -0.220 },
  { float3(0.040, 0.040, 0.022),  0.380 },
  { float3(0.040, 0.040, 0.022), -0.380 },
  { float3(0.030, 0.032, 0.018),  0.580 },
  { float3(0.030, 0.032, 0.018), -0.580 },
  { float3(0.022, 0.025, 0.014),  0.820 },
  { float3(0.022, 0.025, 0.014), -0.820 },
  { float3(0.015, 0.018, 0.010),  1.100 },
  { float3(0.015, 0.018, 0.010), -1.100 },
  { float3(0.010, 0.013, 0.008),  1.420 },
  { float3(0.010, 0.013, 0.008), -1.420 },
  { float3(0.007, 0.009, 0.005),  1.780 },
  { float3(0.007, 0.009, 0.005), -1.780 },
  { float3(0.005, 0.006, 0.004),  2.180 },
  { float3(0.005, 0.006, 0.004), -2.180 },
  { float3(0.003, 0.004, 0.003),  2.620 },
  { float3(0.003, 0.004, 0.003), -2.620 },
  { float3(0.002, 0.003, 0.002),  3.100 },
  { float3(0.002, 0.003, 0.002), -3.100 },
  { float3(0.001, 0.002, 0.001),  3.620 },
  { float3(0.001, 0.002, 0.001), -3.620 },
};

// Green: jade/plant-like subsurface
static const SSSKernelSample JadeKernel[SSS_MAX_KERNEL_SIZE] =
{
  { float3(0.360, 0.480, 0.340),  0.000 },
  { float3(0.038, 0.055, 0.035),  0.090 },
  { float3(0.038, 0.055, 0.035), -0.090 },
  { float3(0.032, 0.048, 0.028),  0.200 },
  { float3(0.032, 0.048, 0.028), -0.200 },
  { float3(0.026, 0.040, 0.022),  0.340 },
  { float3(0.026, 0.040, 0.022), -0.340 },
  { float3(0.020, 0.032, 0.018),  0.520 },
  { float3(0.020, 0.032, 0.018), -0.520 },
  { float3(0.015, 0.025, 0.014),  0.740 },
  { float3(0.015, 0.025, 0.014), -0.740 },
  { float3(0.011, 0.019, 0.010),  1.000 },
  { float3(0.011, 0.019, 0.010), -1.000 },
  { float3(0.008, 0.014, 0.008),  1.300 },
  { float3(0.008, 0.014, 0.008), -1.300 },
  { float3(0.005, 0.010, 0.005),  1.640 },
  { float3(0.005, 0.010, 0.005), -1.640 },
  { float3(0.004, 0.007, 0.004),  2.020 },
  { float3(0.004, 0.007, 0.004), -2.020 },
  { float3(0.002, 0.005, 0.003),  2.440 },
  { float3(0.002, 0.005, 0.003), -2.440 },
  { float3(0.002, 0.003, 0.002),  2.900 },
  { float3(0.002, 0.003, 0.002), -2.900 },
  { float3(0.001, 0.002, 0.001),  3.400 },
  { float3(0.001, 0.002, 0.001), -3.400 },
};

// Milk: nearly white, isotropic scattering
static const SSSKernelSample MilkKernel[SSS_MAX_KERNEL_SIZE] =
{
  { float3(0.500, 0.490, 0.460),  0.000 },
  { float3(0.045, 0.044, 0.040),  0.070 },
  { float3(0.045, 0.044, 0.040), -0.070 },
  { float3(0.038, 0.038, 0.034),  0.150 },
  { float3(0.038, 0.038, 0.034), -0.150 },
  { float3(0.030, 0.030, 0.028),  0.260 },
  { float3(0.030, 0.030, 0.028), -0.260 },
  { float3(0.022, 0.023, 0.022),  0.400 },
  { float3(0.022, 0.023, 0.022), -0.400 },
  { float3(0.016, 0.017, 0.016),  0.580 },
  { float3(0.016, 0.017, 0.016), -0.580 },
  { float3(0.012, 0.012, 0.012),  0.800 },
  { float3(0.012, 0.012, 0.012), -0.800 },
  { float3(0.008, 0.009, 0.008),  1.060 },
  { float3(0.008, 0.009, 0.008), -1.060 },
  { float3(0.005, 0.006, 0.006),  1.360 },
  { float3(0.005, 0.006, 0.006), -1.360 },
  { float3(0.004, 0.004, 0.004),  1.700 },
  { float3(0.004, 0.004, 0.004), -1.700 },
  { float3(0.003, 0.003, 0.003),  2.080 },
  { float3(0.003, 0.003, 0.003), -2.080 },
  { float3(0.002, 0.002, 0.002),  2.500 },
  { float3(0.002, 0.002, 0.002), -2.500 },
  { float3(0.001, 0.001, 0.001),  2.960 },
  { float3(0.001, 0.001, 0.001), -2.960 },
};

// Foliage: thin leaves with strong green transmission and tight blur
// Optimized for backlit translucency common in vegetation
static const SSSKernelSample FoliageKernel[SSS_MAX_KERNEL_SIZE] =
{
  { float3(0.320, 0.520, 0.280),  0.000 },  // Strong green center
  { float3(0.042, 0.068, 0.035),  0.060 },
  { float3(0.042, 0.068, 0.035), -0.060 },
  { float3(0.035, 0.058, 0.028),  0.130 },
  { float3(0.035, 0.058, 0.028), -0.130 },
  { float3(0.028, 0.048, 0.022),  0.220 },
  { float3(0.028, 0.048, 0.022), -0.220 },
  { float3(0.022, 0.038, 0.017),  0.330 },
  { float3(0.022, 0.038, 0.017), -0.330 },
  { float3(0.016, 0.030, 0.013),  0.460 },
  { float3(0.016, 0.030, 0.013), -0.460 },
  { float3(0.012, 0.022, 0.010),  0.610 },
  { float3(0.012, 0.022, 0.010), -0.610 },
  { float3(0.008, 0.016, 0.007),  0.780 },
  { float3(0.008, 0.016, 0.007), -0.780 },
  { float3(0.006, 0.011, 0.005),  0.970 },
  { float3(0.006, 0.011, 0.005), -0.970 },
  { float3(0.004, 0.008, 0.004),  1.180 },
  { float3(0.004, 0.008, 0.004), -1.180 },
  { float3(0.003, 0.005, 0.003),  1.410 },
  { float3(0.003, 0.005, 0.003), -1.410 },
  { float3(0.002, 0.004, 0.002),  1.660 },
  { float3(0.002, 0.004, 0.002), -1.660 },
  { float3(0.001, 0.002, 0.001),  1.930 },
  { float3(0.001, 0.002, 0.001), -1.930 },
};

// Select kernel based on profile index
SSSKernelSample GetKernelSample(uint profileIndex, uint sampleIndex)
{
  if (profileIndex == 1)
    return WaxKernel[sampleIndex];
  else if (profileIndex == 2)
    return JadeKernel[sampleIndex];
  else if (profileIndex == 3)
    return MilkKernel[sampleIndex];
  else if (profileIndex == 4)
    return FoliageKernel[sampleIndex];
  else
    return SkinKernel[sampleIndex]; // Default: skin
}

#endif // PLATFORM_SHADER
