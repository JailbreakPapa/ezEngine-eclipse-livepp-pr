#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

#define GPU_PARTICLE_SIM_THREAD_GROUP_SIZE 256
#define GPU_PARTICLE_EMIT_THREAD_GROUP_SIZE 64
#define GPU_PARTICLE_SDF_THREAD_GROUP_SIZE 8

CONSTANT_BUFFER(ezGPUParticleConstants, 4)
{
  FLOAT1(GPUPartDeltaTime);
  FLOAT1(GPUPartTotalTime);
  UINT1(GPUPartMaxParticles);
  UINT1(GPUPartNumAlive);

  UINT1(GPUPartNumToEmit);
  UINT1(GPUPartEmitStartIndex);
  FLOAT1(GPUPartGravity);
  FLOAT1(GPUPartDragCoefficient);

  FLOAT3(GPUPartWindDirection);
  FLOAT1(GPUPartWindStrength);

  FLOAT1(GPUPartCollisionBounceFactor);
  FLOAT1(GPUPartCollisionSlideFactor);
  UINT1(GPUPartCollisionReaction);
  FLOAT1(GPUPartCollisionThickness);

  FLOAT3(GPUPartSDFWorldMin);
  FLOAT1(GPUPartSDFVoxelSize);

  FLOAT3(GPUPartSDFWorldMax);
  UINT1(GPUPartSDFResolution);

  UINT1(GPUPartEnableDepthCollision);
  UINT1(GPUPartEnableSDFCollision);
  FLOAT1(GPUPartDepthBufferWidth);
  FLOAT1(GPUPartDepthBufferHeight);

  MAT4(GPUPartViewProjectionMatrix);
  MAT4(GPUPartInverseViewProjectionMatrix);

  FLOAT4(GPUPartSizeOverLifeKeyframes0);
  FLOAT4(GPUPartSizeOverLifeKeyframes1);
  FLOAT4(GPUPartColorOverLifeStart);
  FLOAT4(GPUPartColorOverLifeEnd);

  MAT4(GPUPartObjectToWorldMatrix);

  UINT1(GPUPartMaxTrailPoints);
  UINT1(GPUPartTrailWriteIndex);
  FLOAT1(GPUPartVelocityStretch);
  UINT1(GPUPartUseColorGradientTexture);

  FLOAT1(GPUPartNoiseStrength);
  FLOAT1(GPUPartNoiseFrequency);
  FLOAT1(GPUPartNoiseSpeed);
  UINT1(GPUPartSimulateInLocalSpace);

  MAT4(GPUPartWorldToObjectMatrix);
};
