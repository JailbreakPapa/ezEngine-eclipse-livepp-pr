#pragma once

#include "GPUParticleConstants.h"

struct EZ_SHADER_STRUCT ezGPUParticle
{
  FLOAT3(Position);
  FLOAT1(Size);

  FLOAT3(Velocity);
  FLOAT1(Life);

  FLOAT1(MaxLife);
  FLOAT1(RotationOffset);
  FLOAT1(RotationSpeed);
  UINT1(Flags);

  PACKEDCOLOR4H(Color);
  FLOAT1(InitialSize);
  UINT1(GPUPartPadding0);
};

#if EZ_ENABLED(PLATFORM_SHADER)

StructuredBuffer<ezGPUParticle> gpuParticlesRead BIND_GROUP(BG_DRAW_CALL);

#else // C++

static_assert(sizeof(ezGPUParticle) == 64);

#endif
