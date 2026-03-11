#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

#define VOLUMETRIC_FOG_THREAD_GROUP_X 8
#define VOLUMETRIC_FOG_THREAD_GROUP_Y 8
#define VOLUMETRIC_FOG_THREAD_GROUP_Z 1

CONSTANT_BUFFER(ezVolumetricFogConstants, 4)
{
  UINT1(FroxelGridSizeX);
  UINT1(FroxelGridSizeY);
  UINT1(FroxelGridSizeZ);
  FLOAT1(FroxelNearPlane);

  FLOAT1(FroxelFarPlane);
  FLOAT1(FroxelDepthSliceScale);
  FLOAT1(FroxelDepthSliceBias);
  FLOAT1(FogDensityScale);

  FLOAT3(FogAlbedo);
  FLOAT1(FogAnisotropy);

  FLOAT1(TemporalBlendWeight);
  FLOAT1(FogHeightDensityFalloff);
  FLOAT1(FogBaseHeight);
  FLOAT1(VFogStartDistance);

  MAT4(PrevWorldToClipMatrix);

  FLOAT3(AmbientLight);
  FLOAT1(Padding0);
};
