#include <ParticlePlugin/ParticlePluginPCH.h>

#include <Foundation/DataProcessing/Stream/ProcessingStream.h>
#include <ParticlePlugin/Behavior/ParticleConditionalCommon.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_ENUM(ezParticleAttribute, 1)
  EZ_ENUM_CONSTANT(ezParticleAttribute::PositionX),
  EZ_ENUM_CONSTANT(ezParticleAttribute::PositionY),
  EZ_ENUM_CONSTANT(ezParticleAttribute::PositionZ),
  EZ_ENUM_CONSTANT(ezParticleAttribute::Speed),
  EZ_ENUM_CONSTANT(ezParticleAttribute::Size),
  EZ_ENUM_CONSTANT(ezParticleAttribute::LifeFraction),
  EZ_ENUM_CONSTANT(ezParticleAttribute::ColorR),
  EZ_ENUM_CONSTANT(ezParticleAttribute::ColorG),
  EZ_ENUM_CONSTANT(ezParticleAttribute::ColorB),
  EZ_ENUM_CONSTANT(ezParticleAttribute::ColorA),
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezParticleConditionOp, 1)
  EZ_ENUM_CONSTANT(ezParticleConditionOp::Less),
  EZ_ENUM_CONSTANT(ezParticleConditionOp::LessEqual),
  EZ_ENUM_CONSTANT(ezParticleConditionOp::Greater),
  EZ_ENUM_CONSTANT(ezParticleConditionOp::GreaterEqual),
  EZ_ENUM_CONSTANT(ezParticleConditionOp::Equal),
  EZ_ENUM_CONSTANT(ezParticleConditionOp::NotEqual),
EZ_END_STATIC_REFLECTED_ENUM;
// clang-format on

float ezReadParticleAttribute(
  ezParticleAttribute::Enum attr, ezUInt32 uiIndex,
  const ezProcessingStream* pPosition,
  const ezProcessingStream* pVelocity,
  const ezProcessingStream* pSize,
  const ezProcessingStream* pColor,
  const ezProcessingStream* pLifeTime)
{
  switch (attr)
  {
    case ezParticleAttribute::PositionX:
      return pPosition ? static_cast<float>(pPosition->GetData<ezSimdVec4f>()[uiIndex].GetComponent<0>()) : 0.0f;
    case ezParticleAttribute::PositionY:
      return pPosition ? static_cast<float>(pPosition->GetData<ezSimdVec4f>()[uiIndex].GetComponent<1>()) : 0.0f;
    case ezParticleAttribute::PositionZ:
      return pPosition ? static_cast<float>(pPosition->GetData<ezSimdVec4f>()[uiIndex].GetComponent<2>()) : 0.0f;
    case ezParticleAttribute::Speed:
      return pVelocity ? static_cast<float>(pVelocity->GetData<ezFloat16Vec4>()[uiIndex].w) : 0.0f;
    case ezParticleAttribute::Size:
      return pSize ? static_cast<float>(pSize->GetData<ezFloat16>()[uiIndex]) : 1.0f;
    case ezParticleAttribute::LifeFraction:
    {
      if (pLifeTime)
      {
        const ezVec2& lt = pLifeTime->GetData<ezVec2>()[uiIndex];
        return 1.0f - lt.x * lt.y;
      }
      return 0.0f;
    }
    case ezParticleAttribute::ColorR:
      return pColor ? static_cast<float>(pColor->GetData<ezFloat16Vec4>()[uiIndex].x) : 1.0f;
    case ezParticleAttribute::ColorG:
      return pColor ? static_cast<float>(pColor->GetData<ezFloat16Vec4>()[uiIndex].y) : 1.0f;
    case ezParticleAttribute::ColorB:
      return pColor ? static_cast<float>(pColor->GetData<ezFloat16Vec4>()[uiIndex].z) : 1.0f;
    case ezParticleAttribute::ColorA:
      return pColor ? static_cast<float>(pColor->GetData<ezFloat16Vec4>()[uiIndex].w) : 1.0f;
    default:
      return 0.0f;
  }
}

void ezWriteParticleAttribute(
  ezParticleAttribute::Enum attr, ezUInt32 uiIndex, float fValue,
  ezProcessingStream* pPosition,
  ezProcessingStream* pVelocity,
  ezProcessingStream* pSize,
  ezProcessingStream* pColor)
{
  switch (attr)
  {
    case ezParticleAttribute::PositionX:
    {
      if (pPosition)
      {
        ezSimdVec4f& pos = pPosition->GetWritableData<ezSimdVec4f>()[uiIndex];
        pos.SetX(ezSimdFloat(fValue));
      }
      break;
    }
    case ezParticleAttribute::PositionY:
    {
      if (pPosition)
      {
        ezSimdVec4f& pos = pPosition->GetWritableData<ezSimdVec4f>()[uiIndex];
        pos.SetY(ezSimdFloat(fValue));
      }
      break;
    }
    case ezParticleAttribute::PositionZ:
    {
      if (pPosition)
      {
        ezSimdVec4f& pos = pPosition->GetWritableData<ezSimdVec4f>()[uiIndex];
        pos.SetZ(ezSimdFloat(fValue));
      }
      break;
    }
    case ezParticleAttribute::Speed:
    {
      if (pVelocity)
      {
        ezFloat16Vec4& v = pVelocity->GetWritableData<ezFloat16Vec4>()[uiIndex];
        v = ezFloat16Vec4(ezVec4(static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z), fValue));
      }
      break;
    }
    case ezParticleAttribute::Size:
    {
      if (pSize)
        pSize->GetWritableData<ezFloat16>()[uiIndex] = fValue;
      break;
    }
    case ezParticleAttribute::ColorR:
    {
      if (pColor)
      {
        ezFloat16Vec4& c = pColor->GetWritableData<ezFloat16Vec4>()[uiIndex];
        c = ezFloat16Vec4(ezVec4(fValue, static_cast<float>(c.y), static_cast<float>(c.z), static_cast<float>(c.w)));
      }
      break;
    }
    case ezParticleAttribute::ColorG:
    {
      if (pColor)
      {
        ezFloat16Vec4& c = pColor->GetWritableData<ezFloat16Vec4>()[uiIndex];
        c = ezFloat16Vec4(ezVec4(static_cast<float>(c.x), fValue, static_cast<float>(c.z), static_cast<float>(c.w)));
      }
      break;
    }
    case ezParticleAttribute::ColorB:
    {
      if (pColor)
      {
        ezFloat16Vec4& c = pColor->GetWritableData<ezFloat16Vec4>()[uiIndex];
        c = ezFloat16Vec4(ezVec4(static_cast<float>(c.x), static_cast<float>(c.y), fValue, static_cast<float>(c.w)));
      }
      break;
    }
    case ezParticleAttribute::ColorA:
    {
      if (pColor)
      {
        ezFloat16Vec4& c = pColor->GetWritableData<ezFloat16Vec4>()[uiIndex];
        c = ezFloat16Vec4(ezVec4(static_cast<float>(c.x), static_cast<float>(c.y), static_cast<float>(c.z), fValue));
      }
      break;
    }
    case ezParticleAttribute::LifeFraction:
      // LifeFraction is read-only (derived from remaining life)
      break;
    default:
      break;
  }
}

bool ezEvaluateParticleCondition(ezParticleConditionOp::Enum op, float fValue, float fThreshold)
{
  switch (op)
  {
    case ezParticleConditionOp::Less:
      return fValue < fThreshold;
    case ezParticleConditionOp::LessEqual:
      return fValue <= fThreshold;
    case ezParticleConditionOp::Greater:
      return fValue > fThreshold;
    case ezParticleConditionOp::GreaterEqual:
      return fValue >= fThreshold;
    case ezParticleConditionOp::Equal:
      return ezMath::IsEqual(fValue, fThreshold, 0.001f);
    case ezParticleConditionOp::NotEqual:
      return !ezMath::IsEqual(fValue, fThreshold, 0.001f);
    default:
      return false;
  }
}


EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Behavior_ParticleConditionalCommon);
