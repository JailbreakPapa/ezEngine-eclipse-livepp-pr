#pragma once

#include <Foundation/Math/Float16.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/SimdMath/SimdVec4f.h>
#include <ParticlePlugin/ParticlePluginDLL.h>

class ezProcessingStream;

/// Particle attribute that can be read or written by conditional behaviors.
///
/// Used by IF, Switch, Remap, and other logic nodes to select which
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

/// Comparison operator for conditional behaviors.
struct EZ_PARTICLEPLUGIN_DLL ezParticleConditionOp
{
  using StorageType = ezUInt8;

  enum Enum
  {
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Equal,    ///< Within epsilon (0.001)
    NotEqual, ///< Outside epsilon (0.001)

    Default = Less
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_PARTICLEPLUGIN_DLL, ezParticleConditionOp);

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

/// Evaluates a comparison between fValue and fThreshold.
EZ_PARTICLEPLUGIN_DLL bool ezEvaluateParticleCondition(
  ezParticleConditionOp::Enum op, float fValue, float fThreshold);
