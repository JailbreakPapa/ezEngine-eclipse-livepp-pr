#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

#define GODRAYS_THREAD_GROUP_X 8
#define GODRAYS_THREAD_GROUP_Y 8

CONSTANT_BUFFER(ezGodRaysConstants, 4)
{
  UINT1(GodRaysResolutionX);
  UINT1(GodRaysResolutionY);
  UINT1(GodRaysNumSamples);
  FLOAT1(GodRaysDensity);

  FLOAT1(GodRaysWeight);
  FLOAT1(GodRaysDecay);
  FLOAT1(GodRaysExposure);
  FLOAT1(GodRaysIntensity);

  FLOAT1(GodRaysMaxRayLength);
  FLOAT1(GodRaysDepthThreshold);
  FLOAT1(GodRaysPadding0);
  FLOAT1(GodRaysPadding1);
};
