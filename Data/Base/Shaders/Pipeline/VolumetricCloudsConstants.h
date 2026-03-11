#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

#define CLOUD_THREAD_GROUP_X 8
#define CLOUD_THREAD_GROUP_Y 8

CONSTANT_BUFFER(ezVolumetricCloudsConstants, 4)
{
  FLOAT1(CloudLayerBottomAltitude);
  FLOAT1(CloudLayerTopAltitude);
  FLOAT1(CloudCoverage);
  FLOAT1(CloudDensity);

  FLOAT3(CloudWindDirection);
  FLOAT1(CloudWindSpeed);

  FLOAT3(CloudScatterColor);
  FLOAT1(CloudAbsorption);

  FLOAT3(CloudAmbientColor);
  FLOAT1(CloudPhaseG);

  FLOAT1(CloudDetailScale);
  FLOAT1(CloudShapeScale);
  FLOAT1(SilverLiningIntensity);
  FLOAT1(SilverLiningSpread);

  FLOAT2(CloudTextureSize);
  FLOAT1(TemporalBlendWeight);
  UINT1(FrameIndex);

  MAT4(PrevWorldToClipMatrix);
};
