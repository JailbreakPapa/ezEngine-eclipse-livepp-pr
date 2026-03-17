#pragma once

#include <Foundation/Math/Declarations.h>
#include <Foundation/Math/Float16.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/SimdMath/SimdVec4f.h>
#include <ParticlePlugin/ParticlePluginDLL.h>

class ezProcessingStream;

/// Particle attribute that can be read or written by conditional behaviors.
///
/// Used by IF, Switch, Remap, ConditionalKill, and other logic nodes to select which
/// particle property to evaluate or modify.
struct EZ_PARTICLEPLUGIN_DLL ezParticleAttribute
{
  using StorageType = ezUInt8;

  enum Enum
  {
    PositionX,
    PositionY,
    PositionZ,
    Speed,
    Size,
    LifeFraction,
    ColorR,
    ColorG,
    ColorB,
    ColorA,

    Default = Size
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_PARTICLEPLUGIN_DLL, ezParticleAttribute);

/// Reads a particle attribute value at the given index.
/// Returns 0 if the required stream is null.
EZ_PARTICLEPLUGIN_DLL float ezReadParticleAttribute(
  ezParticleAttribute::Enum attr, ezUInt32 uiIndex,
  const ezProcessingStream* pPosition,
  const ezProcessingStream* pVelocity,
  const ezProcessingStream* pSize,
  const ezProcessingStream* pColor,
  const ezProcessingStream* pLifeTime);

/// Writes a float value to a particle attribute at the given index.
EZ_PARTICLEPLUGIN_DLL void ezWriteParticleAttribute(
  ezParticleAttribute::Enum attr, ezUInt32 uiIndex, float fValue,
  ezProcessingStream* pPosition,
  ezProcessingStream* pVelocity,
  ezProcessingStream* pSize,
  ezProcessingStream* pColor);

/// Evaluates a comparison between fValue and fThreshold using the standard ezComparisonOperator.
/// Equal and NotEqual use an epsilon of 0.001 for float comparison.
EZ_PARTICLEPLUGIN_DLL bool ezEvaluateParticleCondition(
  ezComparisonOperator::Enum op, float fValue, float fThreshold);
