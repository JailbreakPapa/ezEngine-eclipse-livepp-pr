#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

CONSTANT_BUFFER(ezDeferredLightingConstants, 4)
{
  FLOAT2(InverseGBufferSize);
  FLOAT1(Padding0);
  FLOAT1(Padding1);
};
