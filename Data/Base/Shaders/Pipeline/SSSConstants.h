#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

#define SSS_MAX_KERNEL_SIZE 25
#define SSS_NUM_PROFILES 4

CONSTANT_BUFFER(ezSSSConstants, 4)
{
  FLOAT2(InverseScreenSize);
  UINT1(KernelSize);
  FLOAT1(SSSStrength);

  FLOAT2(BlurDirection);
  FLOAT1(DepthThreshold);
  UINT1(ProfileIndex);
};
