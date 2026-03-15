#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionByteCode.h>
#include <Foundation/CodeUtils/Expression/ExpressionVM.h>
#include <ParticlePlugin/Behavior/ParticleBehavior.h>

/// Which particle stream an expression variable maps to
struct EZ_PARTICLEPLUGIN_DLL ezParticleExpressionBinding
{
  using StorageType = ezUInt8;

  enum Enum
  {
    PositionX,
    PositionY,
    PositionZ,
    VelocityX,
    VelocityY,
    VelocityZ,
    Speed,
    Size,
    LifeFraction,
    ColorR,
    ColorG,
    ColorB,
    ColorA,

    Default = PositionX
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_PARTICLEPLUGIN_DLL, ezParticleExpressionBinding);

/// Evaluates a math expression per-particle to modify a chosen attribute.
///
/// Variables a, b, c, d are mapped to particle streams. The result is written
/// to the output stream. Uses the ezExpressionVM for SIMD-batched evaluation.
///
/// Example: Expression = "a * sin(b * 3.14159)" with a = Size, b = LifeFraction
/// would create a pulsing size effect over the particle's lifetime.
class EZ_PARTICLEPLUGIN_DLL ezParticleBehaviorFactory_Expression final : public ezParticleBehaviorFactory
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehaviorFactory_Expression, ezParticleBehaviorFactory);

public:
  ezParticleBehaviorFactory_Expression();

  virtual const ezRTTI* GetBehaviorType() const override;
  virtual void CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const override;
  virtual void QueryFinalizerDependencies(ezSet<const ezRTTI*>& inout_finalizerDeps) const override;

  virtual void Save(ezStreamWriter& inout_stream) const override;
  virtual void Load(ezStreamReader& inout_stream) override;

  ezString m_sExpression;
  ezEnum<ezParticleExpressionBinding> m_InputA;
  ezEnum<ezParticleExpressionBinding> m_InputB;
  ezEnum<ezParticleExpressionBinding> m_InputC;
  ezEnum<ezParticleExpressionBinding> m_InputD;
  ezEnum<ezParticleExpressionBinding> m_Output;
};

class EZ_PARTICLEPLUGIN_DLL ezParticleBehavior_Expression final : public ezParticleBehavior
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehavior_Expression, ezParticleBehavior);

public:
  ezString m_sExpression;
  ezEnum<ezParticleExpressionBinding> m_InputA;
  ezEnum<ezParticleExpressionBinding> m_InputB;
  ezEnum<ezParticleExpressionBinding> m_InputC;
  ezEnum<ezParticleExpressionBinding> m_InputD;
  ezEnum<ezParticleExpressionBinding> m_Output;

  void CompileExpression();

protected:
  virtual void CreateRequiredStreams() override;
  virtual void QueryOptionalStreams() override;
  virtual void Process(ezUInt64 uiNumElements) override;

  float ExtractValue(ezParticleExpressionBinding::Enum binding, ezUInt32 uiIndex) const;
  void WriteValue(ezParticleExpressionBinding::Enum binding, ezUInt32 uiIndex, float fValue);

  ezProcessingStream* m_pStreamPosition = nullptr;
  ezProcessingStream* m_pStreamVelocity = nullptr;
  ezProcessingStream* m_pStreamSize = nullptr;
  ezProcessingStream* m_pStreamColor = nullptr;
  ezProcessingStream* m_pStreamLifeTime = nullptr;

  ezExpressionByteCode m_ByteCode;
  ezExpressionVM m_VM;
  bool m_bBytecodeValid = false;
  ezString m_sCompiledExpression; ///< The expression that was last successfully compiled
};
