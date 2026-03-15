#include <ParticlePlugin/ParticlePluginPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionCompiler.h>
#include <Foundation/CodeUtils/Expression/ExpressionParser.h>
#include <Foundation/DataProcessing/Stream/ProcessingStreamIterator.h>
#include <Foundation/Profiling/Profiling.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_Expression.h>
#include <ParticlePlugin/Effect/ParticleEffectInstance.h>
#include <ParticlePlugin/Finalizer/ParticleFinalizer_ApplyVelocity.h>
#include <ParticlePlugin/System/ParticleSystemInstance.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_ENUM(ezParticleExpressionBinding, 1)
  EZ_ENUM_CONSTANT(ezParticleExpressionBinding::PositionX),
  EZ_ENUM_CONSTANT(ezParticleExpressionBinding::PositionY),
  EZ_ENUM_CONSTANT(ezParticleExpressionBinding::PositionZ),
  EZ_ENUM_CONSTANT(ezParticleExpressionBinding::VelocityX),
  EZ_ENUM_CONSTANT(ezParticleExpressionBinding::VelocityY),
  EZ_ENUM_CONSTANT(ezParticleExpressionBinding::VelocityZ),
  EZ_ENUM_CONSTANT(ezParticleExpressionBinding::Speed),
  EZ_ENUM_CONSTANT(ezParticleExpressionBinding::Size),
  EZ_ENUM_CONSTANT(ezParticleExpressionBinding::LifeFraction),
  EZ_ENUM_CONSTANT(ezParticleExpressionBinding::ColorR),
  EZ_ENUM_CONSTANT(ezParticleExpressionBinding::ColorG),
  EZ_ENUM_CONSTANT(ezParticleExpressionBinding::ColorB),
  EZ_ENUM_CONSTANT(ezParticleExpressionBinding::ColorA),
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehaviorFactory_Expression, 1, ezRTTIDefaultAllocator<ezParticleBehaviorFactory_Expression>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Expression", m_sExpression),
    EZ_ENUM_MEMBER_PROPERTY("InputA", ezParticleExpressionBinding, m_InputA),
    EZ_ENUM_MEMBER_PROPERTY("InputB", ezParticleExpressionBinding, m_InputB),
    EZ_ENUM_MEMBER_PROPERTY("InputC", ezParticleExpressionBinding, m_InputC),
    EZ_ENUM_MEMBER_PROPERTY("InputD", ezParticleExpressionBinding, m_InputD),
    EZ_ENUM_MEMBER_PROPERTY("Output", ezParticleExpressionBinding, m_Output),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleBehavior_Expression, 1, ezRTTIDefaultAllocator<ezParticleBehavior_Expression>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezParticleBehaviorFactory_Expression::ezParticleBehaviorFactory_Expression() = default;

const ezRTTI* ezParticleBehaviorFactory_Expression::GetBehaviorType() const
{
  return ezGetStaticRTTI<ezParticleBehavior_Expression>();
}

void ezParticleBehaviorFactory_Expression::CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const
{
  ezParticleBehavior_Expression* pBehavior = static_cast<ezParticleBehavior_Expression*>(pObject);

  pBehavior->m_InputA = m_InputA;
  pBehavior->m_InputB = m_InputB;
  pBehavior->m_InputC = m_InputC;
  pBehavior->m_InputD = m_InputD;
  pBehavior->m_Output = m_Output;

  if (pBehavior->m_sExpression != m_sExpression)
  {
    pBehavior->m_sExpression = m_sExpression;
    pBehavior->CompileExpression();
  }
  else if (bFirstTime)
  {
    pBehavior->CompileExpression();
  }
}

void ezParticleBehaviorFactory_Expression::QueryFinalizerDependencies(ezSet<const ezRTTI*>& inout_finalizerDeps) const
{
  // If the output modifies velocity, we need the ApplyVelocity finalizer
  if (m_Output == ezParticleExpressionBinding::VelocityX ||
      m_Output == ezParticleExpressionBinding::VelocityY ||
      m_Output == ezParticleExpressionBinding::VelocityZ ||
      m_Output == ezParticleExpressionBinding::Speed)
  {
    inout_finalizerDeps.Insert(ezGetStaticRTTI<ezParticleFinalizerFactory_ApplyVelocity>());
  }
}

void ezParticleBehaviorFactory_Expression::Save(ezStreamWriter& inout_stream) const
{
  const ezUInt8 uiVersion = 1;
  inout_stream << uiVersion;

  inout_stream << m_sExpression;
  inout_stream << m_InputA;
  inout_stream << m_InputB;
  inout_stream << m_InputC;
  inout_stream << m_InputD;
  inout_stream << m_Output;
}

void ezParticleBehaviorFactory_Expression::Load(ezStreamReader& inout_stream)
{
  ezUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  EZ_ASSERT_DEV(uiVersion <= 1, "Invalid version {0}", uiVersion);

  inout_stream >> m_sExpression;
  inout_stream >> m_InputA;
  inout_stream >> m_InputB;
  inout_stream >> m_InputC;
  inout_stream >> m_InputD;
  inout_stream >> m_Output;
}

//////////////////////////////////////////////////////////////////////////

void ezParticleBehavior_Expression::CompileExpression()
{
  m_bBytecodeValid = false;

  if (m_sExpression.IsEmpty())
    return;

  ezExpressionParser parser;

  ezHybridArray<ezExpression::StreamDesc, 4> inputs;
  inputs.PushBack({ezMakeHashedString("a"), ezProcessingStream::DataType::Float});
  inputs.PushBack({ezMakeHashedString("b"), ezProcessingStream::DataType::Float});
  inputs.PushBack({ezMakeHashedString("c"), ezProcessingStream::DataType::Float});
  inputs.PushBack({ezMakeHashedString("d"), ezProcessingStream::DataType::Float});

  ezHybridArray<ezExpression::StreamDesc, 1> outputs;
  outputs.PushBack({ezMakeHashedString("out"), ezProcessingStream::DataType::Float});

  // Build expression: "out = <expression>"
  ezStringBuilder sFullExpression;
  sFullExpression.SetFormat("out = {0}", m_sExpression);

  ezExpressionParser::Options options;
  ezExpressionAST ast;
  if (parser.Parse(sFullExpression, inputs, outputs, options, ast).Failed())
  {
    ezLog::Warning("Particle Expression: Failed to parse expression '{0}'", m_sExpression);
    return;
  }

  ezExpressionCompiler compiler;
  if (compiler.Compile(ast, m_ByteCode).Failed())
  {
    ezLog::Warning("Particle Expression: Failed to compile expression '{0}'", m_sExpression);
    return;
  }

  m_sCompiledExpression = m_sExpression;
  m_bBytecodeValid = true;
}

void ezParticleBehavior_Expression::CreateRequiredStreams()
{
  CreateStream("Position", ezProcessingStream::DataType::Float4, &m_pStreamPosition, false);
  CreateStream("Velocity", ezProcessingStream::DataType::Half4, &m_pStreamVelocity, false);
}

void ezParticleBehavior_Expression::QueryOptionalStreams()
{
  m_pStreamSize = GetOwnerSystem()->QueryStream("Size", ezProcessingStream::DataType::Half);
  m_pStreamColor = GetOwnerSystem()->QueryStream("Color", ezProcessingStream::DataType::Half4);
  m_pStreamLifeTime = GetOwnerSystem()->QueryStream("LifeTime", ezProcessingStream::DataType::Float2);
}

float ezParticleBehavior_Expression::ExtractValue(ezParticleExpressionBinding::Enum binding, ezUInt32 uiIndex) const
{
  switch (binding)
  {
    case ezParticleExpressionBinding::PositionX:
      return m_pStreamPosition->GetData<ezSimdVec4f>()[uiIndex].GetComponent<0>();
    case ezParticleExpressionBinding::PositionY:
      return m_pStreamPosition->GetData<ezSimdVec4f>()[uiIndex].GetComponent<1>();
    case ezParticleExpressionBinding::PositionZ:
      return m_pStreamPosition->GetData<ezSimdVec4f>()[uiIndex].GetComponent<2>();
    case ezParticleExpressionBinding::VelocityX:
    {
      const ezFloat16Vec4& v = m_pStreamVelocity->GetData<ezFloat16Vec4>()[uiIndex];
      return static_cast<float>(v.x) * static_cast<float>(v.w); // dirX * speed
    }
    case ezParticleExpressionBinding::VelocityY:
    {
      const ezFloat16Vec4& v = m_pStreamVelocity->GetData<ezFloat16Vec4>()[uiIndex];
      return static_cast<float>(v.y) * static_cast<float>(v.w); // dirY * speed
    }
    case ezParticleExpressionBinding::VelocityZ:
    {
      const ezFloat16Vec4& v = m_pStreamVelocity->GetData<ezFloat16Vec4>()[uiIndex];
      return static_cast<float>(v.z) * static_cast<float>(v.w); // dirZ * speed
    }
    case ezParticleExpressionBinding::Speed:
    {
      const ezFloat16Vec4& v = m_pStreamVelocity->GetData<ezFloat16Vec4>()[uiIndex];
      return static_cast<float>(v.w);
    }
    case ezParticleExpressionBinding::Size:
    {
      if (m_pStreamSize)
        return static_cast<float>(m_pStreamSize->GetData<ezFloat16>()[uiIndex]);
      return 1.0f;
    }
    case ezParticleExpressionBinding::LifeFraction:
    {
      if (m_pStreamLifeTime)
      {
        // LifeTime stream stores (life, invMaxLife) as Float2
        const ezVec2& lt = m_pStreamLifeTime->GetData<ezVec2>()[uiIndex];
        return 1.0f - lt.x * lt.y; // 1 - (remainingLife * invMaxLife) = fraction consumed
      }
      return 0.0f;
    }
    case ezParticleExpressionBinding::ColorR:
    {
      if (m_pStreamColor)
        return static_cast<float>(m_pStreamColor->GetData<ezFloat16Vec4>()[uiIndex].x);
      return 1.0f;
    }
    case ezParticleExpressionBinding::ColorG:
    {
      if (m_pStreamColor)
        return static_cast<float>(m_pStreamColor->GetData<ezFloat16Vec4>()[uiIndex].y);
      return 1.0f;
    }
    case ezParticleExpressionBinding::ColorB:
    {
      if (m_pStreamColor)
        return static_cast<float>(m_pStreamColor->GetData<ezFloat16Vec4>()[uiIndex].z);
      return 1.0f;
    }
    case ezParticleExpressionBinding::ColorA:
    {
      if (m_pStreamColor)
        return static_cast<float>(m_pStreamColor->GetData<ezFloat16Vec4>()[uiIndex].w);
      return 1.0f;
    }
    default:
      return 0.0f;
  }
}

void ezParticleBehavior_Expression::WriteValue(ezParticleExpressionBinding::Enum binding, ezUInt32 uiIndex, float fValue)
{
  switch (binding)
  {
    case ezParticleExpressionBinding::PositionX:
    {
      ezSimdVec4f& pos = m_pStreamPosition->GetWritableData<ezSimdVec4f>()[uiIndex];
      pos.SetX(ezSimdFloat(fValue));
      break;
    }
    case ezParticleExpressionBinding::PositionY:
    {
      ezSimdVec4f& pos = m_pStreamPosition->GetWritableData<ezSimdVec4f>()[uiIndex];
      pos.SetY(ezSimdFloat(fValue));
      break;
    }
    case ezParticleExpressionBinding::PositionZ:
    {
      ezSimdVec4f& pos = m_pStreamPosition->GetWritableData<ezSimdVec4f>()[uiIndex];
      pos.SetZ(ezSimdFloat(fValue));
      break;
    }
    case ezParticleExpressionBinding::VelocityX:
    case ezParticleExpressionBinding::VelocityY:
    case ezParticleExpressionBinding::VelocityZ:
    {
      // Reconstruct velocity from direction * speed, modify one component, re-normalize
      ezFloat16Vec4& v = m_pStreamVelocity->GetWritableData<ezFloat16Vec4>()[uiIndex];
      float vx = static_cast<float>(v.x) * static_cast<float>(v.w);
      float vy = static_cast<float>(v.y) * static_cast<float>(v.w);
      float vz = static_cast<float>(v.z) * static_cast<float>(v.w);

      if (binding == ezParticleExpressionBinding::VelocityX)
        vx = fValue;
      else if (binding == ezParticleExpressionBinding::VelocityY)
        vy = fValue;
      else
        vz = fValue;

      const ezVec3 newVel(vx, vy, vz);
      const float newSpeed = newVel.GetLength();
      const ezVec3 newDir = newSpeed > 0.0f ? newVel / newSpeed : ezVec3(0, 0, 1);
      v = ezFloat16Vec4(ezVec4(newDir.x, newDir.y, newDir.z, newSpeed));
      break;
    }
    case ezParticleExpressionBinding::Speed:
    {
      ezFloat16Vec4& v = m_pStreamVelocity->GetWritableData<ezFloat16Vec4>()[uiIndex];
      v = ezFloat16Vec4(ezVec4(static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z), fValue));
      break;
    }
    case ezParticleExpressionBinding::Size:
    {
      if (m_pStreamSize)
        m_pStreamSize->GetWritableData<ezFloat16>()[uiIndex] = fValue;
      break;
    }
    case ezParticleExpressionBinding::ColorR:
    {
      if (m_pStreamColor)
      {
        ezFloat16Vec4& c = m_pStreamColor->GetWritableData<ezFloat16Vec4>()[uiIndex];
        c = ezFloat16Vec4(ezVec4(fValue, static_cast<float>(c.y), static_cast<float>(c.z), static_cast<float>(c.w)));
      }
      break;
    }
    case ezParticleExpressionBinding::ColorG:
    {
      if (m_pStreamColor)
      {
        ezFloat16Vec4& c = m_pStreamColor->GetWritableData<ezFloat16Vec4>()[uiIndex];
        c = ezFloat16Vec4(ezVec4(static_cast<float>(c.x), fValue, static_cast<float>(c.z), static_cast<float>(c.w)));
      }
      break;
    }
    case ezParticleExpressionBinding::ColorB:
    {
      if (m_pStreamColor)
      {
        ezFloat16Vec4& c = m_pStreamColor->GetWritableData<ezFloat16Vec4>()[uiIndex];
        c = ezFloat16Vec4(ezVec4(static_cast<float>(c.x), static_cast<float>(c.y), fValue, static_cast<float>(c.w)));
      }
      break;
    }
    case ezParticleExpressionBinding::ColorA:
    {
      if (m_pStreamColor)
      {
        ezFloat16Vec4& c = m_pStreamColor->GetWritableData<ezFloat16Vec4>()[uiIndex];
        c = ezFloat16Vec4(ezVec4(static_cast<float>(c.x), static_cast<float>(c.y), static_cast<float>(c.z), fValue));
      }
      break;
    }
    default:
      break;
  }
}

void ezParticleBehavior_Expression::Process(ezUInt64 uiNumElements)
{
  EZ_PROFILE_SCOPE("PFX: Expression");

  if (!m_bBytecodeValid || uiNumElements == 0)
    return;

  // Marshal particle data into flat float arrays for the expression VM
  const ezUInt32 uiCount = static_cast<ezUInt32>(uiNumElements);

  ezDynamicArray<float> inputA, inputB, inputC, inputD;
  ezDynamicArray<float> result;

  inputA.SetCountUninitialized(uiCount);
  inputB.SetCountUninitialized(uiCount);
  inputC.SetCountUninitialized(uiCount);
  inputD.SetCountUninitialized(uiCount);
  result.SetCountUninitialized(uiCount);

  for (ezUInt32 i = 0; i < uiCount; ++i)
  {
    inputA[i] = ExtractValue(m_InputA, i);
    inputB[i] = ExtractValue(m_InputB, i);
    inputC[i] = ExtractValue(m_InputC, i);
    inputD[i] = ExtractValue(m_InputD, i);
  }

  // Build streams for the VM
  ezHybridArray<ezProcessingStream, 4> inputs;
  inputs.PushBack(ezProcessingStream(ezMakeHashedString("a"), ezMakeArrayPtr(inputA).ToByteArray(), ezProcessingStream::DataType::Float));
  inputs.PushBack(ezProcessingStream(ezMakeHashedString("b"), ezMakeArrayPtr(inputB).ToByteArray(), ezProcessingStream::DataType::Float));
  inputs.PushBack(ezProcessingStream(ezMakeHashedString("c"), ezMakeArrayPtr(inputC).ToByteArray(), ezProcessingStream::DataType::Float));
  inputs.PushBack(ezProcessingStream(ezMakeHashedString("d"), ezMakeArrayPtr(inputD).ToByteArray(), ezProcessingStream::DataType::Float));

  ezHybridArray<ezProcessingStream, 1> outputs;
  outputs.PushBack(ezProcessingStream(ezMakeHashedString("out"), ezMakeArrayPtr(result).ToByteArray(), ezProcessingStream::DataType::Float));

  if (m_VM.Execute(m_ByteCode, inputs, outputs, uiCount).Failed())
  {
    ezLog::Warning("Particle Expression: Failed to execute expression '{0}'", m_sExpression);
    m_bBytecodeValid = false;
    return;
  }

  // Write results back to the particle streams
  for (ezUInt32 i = 0; i < uiCount; ++i)
  {
    WriteValue(m_Output, i, result[i]);
  }
}


EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Behavior_ParticleBehavior_Expression);
