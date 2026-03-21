#pragma once

#include <Shaders/Common/ConstantBufferMacros.h>

// Shared GPU data structures for hair strand rendering.
// This header is included from both HLSL shaders and C++ code.

struct EZ_SHADER_STRUCT ezHairStrandPointData
{
  FLOAT3(Position);
  FLOAT1(Width);
};

struct EZ_SHADER_STRUCT ezHairStrandInfoData
{
  UINT1(PointOffset);
  UINT1(PointCount);
  FLOAT1(RootU);
  FLOAT1(RootV);
  FLOAT1(Random);
  FLOAT1(Pad0);
  FLOAT1(Pad1);
  FLOAT1(Pad2);
};

struct EZ_SHADER_STRUCT ezHairTessVertexData
{
  FLOAT3(Position);
  FLOAT1(StrandParam);
  FLOAT3(Normal);
  FLOAT1(Pad0);
  FLOAT3(Tangent);
  FLOAT1(Pad1);
  FLOAT2(TexCoord);
  FLOAT1(Pad2);
  FLOAT1(Pad3);
};

// Constants passed to the tessellation compute shader per dispatch.
CONSTANT_BUFFER(ezHairTessellateConstants, 3)
{
  UINT1(NumStrands);
  UINT1(StrandOffset);
  UINT1(TessVertexOffset);
  UINT1(Pad0);

  TRANSFORM(ObjectToWorld);
  FLOAT3(CameraPosition);
  FLOAT1(WidthScale);
  FLOAT3(WindDirection);
  FLOAT1(WindStrength);
  FLOAT1(Time);
  FLOAT1(GravityStrength);
  FLOAT1(Stiffness);
  FLOAT1(Pad1);
};

// this is only defined during shader compilation
#if EZ_ENABLED(PLATFORM_SHADER)

StructuredBuffer<ezHairStrandPointData> hairStrandPoints BIND_GROUP(BG_DRAW_CALL);
StructuredBuffer<ezHairStrandInfoData> hairStrandInfos BIND_GROUP(BG_DRAW_CALL);

#endif
