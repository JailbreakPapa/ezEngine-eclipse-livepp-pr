#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

CONSTANT_BUFFER(ezScreenSpaceShadowConstants, 4)
{
  FLOAT4(SSSLightCoordinate);
  INT2(SSSWaveOffset);
  FLOAT1(SSSSurfaceThickness);
  FLOAT1(SSSBilinearThreshold);

  FLOAT1(SSSShadowContrast);
  FLOAT1(SSSFarDepthValue);
  FLOAT1(SSSNearDepthValue);
  UINT1(SSSResolutionX);

  UINT1(SSSResolutionY);
  FLOAT2(SSSInvDepthTextureSize);
  UINT1(SSSPadding0);
};
