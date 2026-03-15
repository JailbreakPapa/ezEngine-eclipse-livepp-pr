#include <ParticlePlugin/ParticlePluginPCH.h>

#include <Foundation/Math/Color16f.h>
#include <Foundation/Math/Float16.h>
#include <Core/Interfaces/WindWorldModule.h>
#include <Core/World/World.h>
#include <ParticlePlugin/Behavior/ParticleBehavior_Raycast.h>
#include <ParticlePlugin/Effect/ParticleEffectInstance.h>
#include <ParticlePlugin/System/ParticleSystemInstance.h>
#include <ParticlePlugin/Type/GPU/GPUParticleRenderer.h>
#include <ParticlePlugin/Type/GPU/ParticleTypeGPU.h>
#include <RendererCore/Lights/GPUParticleDataProvider.h>
#include <RendererCore/Pipeline/RenderData.h>
#include <RendererCore/Pipeline/RenderDataManager.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererCore/Textures/Texture2DResource.h>
#include <RendererFoundation/Device/Device.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleTypeGPUFactory, 1, ezRTTIDefaultAllocator<ezParticleTypeGPUFactory>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("MaxParticles", m_uiMaxParticles)->AddAttributes(new ezDefaultValueAttribute(65536), new ezClampValueAttribute(1024, 1048576)),
    EZ_ENUM_MEMBER_PROPERTY("RenderMode", ezParticleTypeRenderMode, m_RenderMode),
    EZ_ENUM_MEMBER_PROPERTY("GPURenderType", ezGPUParticleRenderType, m_GPURenderType),
    EZ_MEMBER_PROPERTY("Texture", m_sTexture)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Texture_2D")),
    EZ_MEMBER_PROPERTY("Gravity", m_fGravity)->AddAttributes(new ezDefaultValueAttribute(9.81f)),
    EZ_MEMBER_PROPERTY("Drag", m_fDragCoefficient)->AddAttributes(new ezDefaultValueAttribute(0.0f), new ezClampValueAttribute(0.0f, 10.0f)),
    EZ_MEMBER_PROPERTY("WindStrength", m_fWindStrength)->AddAttributes(new ezDefaultValueAttribute(0.0f)),
    EZ_MEMBER_PROPERTY("DepthCollision", m_bEnableDepthCollision),
    EZ_MEMBER_PROPERTY("SDFCollision", m_bEnableSDFCollision),
    EZ_ENUM_MEMBER_PROPERTY("CollisionReaction", ezParticleRaycastHitReaction, m_CollisionReaction),
    EZ_MEMBER_PROPERTY("BounceFactor", m_fCollisionBounceFactor)->AddAttributes(new ezDefaultValueAttribute(0.5f), new ezClampValueAttribute(0.0f, 1.0f)),
    EZ_MEMBER_PROPERTY("SlideFactor", m_fCollisionSlideFactor)->AddAttributes(new ezDefaultValueAttribute(0.5f), new ezClampValueAttribute(0.0f, 1.0f)),
    EZ_MEMBER_PROPERTY("CollisionThickness", m_fCollisionThickness)->AddAttributes(new ezDefaultValueAttribute(0.5f), new ezClampValueAttribute(0.01f, 5.0f)),
    EZ_MEMBER_PROPERTY("ColorStart", m_ColorStart)->AddAttributes(new ezDefaultValueAttribute(ezColor::White)),
    EZ_MEMBER_PROPERTY("ColorEnd", m_ColorEnd)->AddAttributes(new ezDefaultValueAttribute(ezColor(1, 1, 1, 0))),
    EZ_MEMBER_PROPERTY("MaxTrailPoints", m_uiMaxTrailPoints)->AddAttributes(new ezDefaultValueAttribute(16), new ezClampValueAttribute(4, 64)),
    EZ_MEMBER_PROPERTY("VelocityStretch", m_fVelocityStretch)->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.0f, 20.0f)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleTypeGPU, 1, ezRTTIDefaultAllocator<ezParticleTypeGPU>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

const ezRTTI* ezParticleTypeGPUFactory::GetTypeType() const
{
  return ezGetStaticRTTI<ezParticleTypeGPU>();
}

void ezParticleTypeGPUFactory::CopyTypeProperties(ezParticleType* pObject, bool bFirstTime) const
{
  ezParticleTypeGPU* pType = static_cast<ezParticleTypeGPU*>(pObject);

  pType->m_uiMaxGPUParticles = m_uiMaxParticles;
  pType->m_RenderMode = m_RenderMode;
  pType->m_hTexture = ezResourceManager::LoadResource<ezTexture2DResource>(m_sTexture);
  pType->m_fGravity = m_fGravity;
  pType->m_fDragCoefficient = m_fDragCoefficient;
  pType->m_fWindStrength = m_fWindStrength;
  pType->m_bEnableDepthCollision = m_bEnableDepthCollision;
  pType->m_bEnableSDFCollision = m_bEnableSDFCollision;
  pType->m_CollisionReaction = m_CollisionReaction;
  pType->m_fCollisionBounceFactor = m_fCollisionBounceFactor;
  pType->m_fCollisionSlideFactor = m_fCollisionSlideFactor;
  pType->m_fCollisionThickness = m_fCollisionThickness;
  pType->m_ColorStart = m_ColorStart;
  pType->m_ColorEnd = m_ColorEnd;
  pType->m_GPURenderType = m_GPURenderType;
  pType->m_uiMaxTrailPoints = m_uiMaxTrailPoints;
  pType->m_fVelocityStretch = m_fVelocityStretch;
}

enum class TypeGPUVersion
{
  Version_1 = 1,
  Version_2_RenderType,

  Version_Count,
  Version_Current = Version_Count - 1
};

void ezParticleTypeGPUFactory::Save(ezStreamWriter& inout_stream) const
{
  const ezUInt8 uiVersion = (ezUInt8)TypeGPUVersion::Version_Current;
  inout_stream << uiVersion;

  inout_stream << m_uiMaxParticles;
  inout_stream << m_RenderMode;
  inout_stream << m_sTexture;
  inout_stream << m_fGravity;
  inout_stream << m_fDragCoefficient;
  inout_stream << m_fWindStrength;
  inout_stream << m_bEnableDepthCollision;
  inout_stream << m_bEnableSDFCollision;
  inout_stream << m_CollisionReaction;
  inout_stream << m_fCollisionBounceFactor;
  inout_stream << m_fCollisionSlideFactor;
  inout_stream << m_fCollisionThickness;
  inout_stream << m_ColorStart;
  inout_stream << m_ColorEnd;

  // Version_2_RenderType
  inout_stream << m_GPURenderType;
  inout_stream << m_uiMaxTrailPoints;
  inout_stream << m_fVelocityStretch;
}

void ezParticleTypeGPUFactory::Load(ezStreamReader& inout_stream)
{
  ezUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  EZ_ASSERT_DEV(uiVersion <= (ezUInt8)TypeGPUVersion::Version_Current, "Invalid version {0}", uiVersion);

  inout_stream >> m_uiMaxParticles;
  inout_stream >> m_RenderMode;
  inout_stream >> m_sTexture;
  inout_stream >> m_fGravity;
  inout_stream >> m_fDragCoefficient;
  inout_stream >> m_fWindStrength;
  inout_stream >> m_bEnableDepthCollision;
  inout_stream >> m_bEnableSDFCollision;
  inout_stream >> m_CollisionReaction;
  inout_stream >> m_fCollisionBounceFactor;
  inout_stream >> m_fCollisionSlideFactor;
  inout_stream >> m_fCollisionThickness;
  inout_stream >> m_ColorStart;
  inout_stream >> m_ColorEnd;

  if (uiVersion >= (ezUInt8)TypeGPUVersion::Version_2_RenderType)
  {
    inout_stream >> m_GPURenderType;
    inout_stream >> m_uiMaxTrailPoints;
    inout_stream >> m_fVelocityStretch;
  }
}

//////////////////////////////////////////////////////////////////////////

ezParticleTypeGPU::ezParticleTypeGPU() = default;

ezParticleTypeGPU::~ezParticleTypeGPU()
{
  DestroyGPUBuffers();
}

void ezParticleTypeGPU::CreateRequiredStreams()
{
  CreateStream("Position", ezProcessingStream::DataType::Float4, &m_pStreamPosition, false);
  CreateStream("Velocity", ezProcessingStream::DataType::Half4, &m_pStreamVelocity, false);
  CreateStream("LifeTime", ezProcessingStream::DataType::Half2, &m_pStreamLifeTime, false);
  CreateStream("Size", ezProcessingStream::DataType::Half, &m_pStreamSize, false);
  CreateStream("Color", ezProcessingStream::DataType::Half4, &m_pStreamColor, false);
  CreateStream("RotationSpeed", ezProcessingStream::DataType::Half, &m_pStreamRotationSpeed, false);
  CreateStream("RotationOffset", ezProcessingStream::DataType::Half, &m_pStreamRotationOffset, false);
}

void ezParticleTypeGPU::EnsureGPUBuffers() const
{
  if (m_bGPUBuffersCreated)
    return;

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  // Particle structured buffer
  {
    ezGALBufferCreationDescription desc;
    desc.m_uiStructSize = sizeof(ezGPUParticle);
    desc.m_uiTotalSize = m_uiMaxGPUParticles * desc.m_uiStructSize;
    desc.m_BufferFlags = ezGALBufferUsageFlags::StructuredBuffer | ezGALBufferUsageFlags::ShaderResource | ezGALBufferUsageFlags::UnorderedAccess;
    desc.m_ResourceAccess.m_bImmutable = false;
    m_hParticleBuffer = pDevice->CreateBuffer(desc);
  }

  // Counter + freelist buffer (ByteAddressBuffer)
  // Layout: [aliveCount(4)][deadCount(4)][deadStack(maxParticles*4)]
  {
    ezGALBufferCreationDescription desc;
    desc.m_uiTotalSize = (2 + m_uiMaxGPUParticles) * sizeof(ezUInt32);
    desc.m_BufferFlags = ezGALBufferUsageFlags::UnorderedAccess | ezGALBufferUsageFlags::ShaderResource | ezGALBufferUsageFlags::ByteAddressBuffer;
    desc.m_ResourceAccess.m_bImmutable = false;
    m_hCounterBuffer = pDevice->CreateBuffer(desc);
  }

  // Trail position buffer (only for Trail render type)
  if (m_GPURenderType == ezGPUParticleRenderType::Trail)
  {
    ezGALBufferCreationDescription desc;
    desc.m_uiStructSize = sizeof(ezVec4);
    desc.m_uiTotalSize = m_uiMaxGPUParticles * m_uiMaxTrailPoints * desc.m_uiStructSize;
    desc.m_BufferFlags = ezGALBufferUsageFlags::StructuredBuffer | ezGALBufferUsageFlags::ShaderResource | ezGALBufferUsageFlags::UnorderedAccess;
    desc.m_ResourceAccess.m_bImmutable = false;
    m_hTrailPositionBuffer = pDevice->CreateBuffer(desc);
  }

  m_bGPUBuffersCreated = true;
  m_uiGPUEmitIndex = 0;
  m_uiTrailWriteIndex = 0;
}

void ezParticleTypeGPU::DestroyGPUBuffers()
{
  if (!m_bGPUBuffersCreated)
    return;

  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();

  if (!m_hParticleBuffer.IsInvalidated())
    pDevice->DestroyBuffer(m_hParticleBuffer);
  if (!m_hCounterBuffer.IsInvalidated())
    pDevice->DestroyBuffer(m_hCounterBuffer);
  if (!m_hTrailPositionBuffer.IsInvalidated())
    pDevice->DestroyBuffer(m_hTrailPositionBuffer);

  m_hParticleBuffer.Invalidate();
  m_hCounterBuffer.Invalidate();
  m_hTrailPositionBuffer.Invalidate();
  m_bGPUBuffersCreated = false;
}

void ezParticleTypeGPU::ExtractTypeRenderData(ezMsgExtractRenderData& ref_msg, const ezTransform& instanceTransform) const
{
  EnsureGPUBuffers();

  const ezUInt32 numParticles = (ezUInt32)GetOwnerSystem()->GetNumActiveParticles();

  // Pack newly emitted particles into GPU format
  ezArrayPtr<ezGPUParticle> newParticles;
  if (numParticles > 0)
  {
    newParticles = EZ_NEW_ARRAY(ezFrameAllocator::GetCurrentAllocator(), ezGPUParticle, numParticles);

    const ezVec4* pPosition = m_pStreamPosition->GetData<ezVec4>();
    const ezFloat16Vec4* pVelocity = m_pStreamVelocity->GetData<ezFloat16Vec4>();
    const ezFloat16Vec2* pLifeTime = m_pStreamLifeTime->GetData<ezFloat16Vec2>();
    const ezFloat16* pSize = m_pStreamSize->GetData<ezFloat16>();
    const ezColorLinear16f* pColor = m_pStreamColor->GetData<ezColorLinear16f>();
    const ezFloat16* pRotSpeed = m_pStreamRotationSpeed != nullptr ? m_pStreamRotationSpeed->GetData<ezFloat16>() : nullptr;
    const ezFloat16* pRotOffset = m_pStreamRotationOffset != nullptr ? m_pStreamRotationOffset->GetData<ezFloat16>() : nullptr;

    for (ezUInt32 i = 0; i < numParticles; ++i)
    {
      ezGPUParticle& p = newParticles[i];

      p.Position = pPosition[i].GetAsVec3();
      const float vx = (float)pVelocity[i].x;
      const float vy = (float)pVelocity[i].y;
      const float vz = (float)pVelocity[i].z;
      const float speed = (float)pVelocity[i].w;
      p.Velocity.Set(vx * speed, vy * speed, vz * speed);

      const float life = (float)pLifeTime[i].x;
      const float maxLife = (float)pLifeTime[i].y;
      p.Life = life / ezMath::Max(maxLife, 0.001f);
      p.MaxLife = 1.0f / ezMath::Max(maxLife, 0.001f);

      p.Size = pSize[i];
      p.InitialSize = p.Size;
      p.RotationOffset = pRotOffset != nullptr ? (float)pRotOffset[i] : 0.0f;
      p.RotationSpeed = pRotSpeed != nullptr ? (float)pRotSpeed[i] : 0.0f;
      p.Flags = 1; // alive

      // Pack color as RGBA16F
      ezColorLinear16f col = pColor[i];
      ezUInt32* pColorU32 = reinterpret_cast<ezUInt32*>(&p.Color);
      pColorU32[0] = (ezUInt32)col.r.GetRawData() | ((ezUInt32)col.g.GetRawData() << 16);
      pColorU32[1] = (ezUInt32)col.b.GetRawData() | ((ezUInt32)col.a.GetRawData() << 16);

      p.GPUPartPadding0 = 0;
    }
  }

  // Create render data
  auto pRenderData = ref_msg.m_pRenderDataManager->CreateRenderDataForThisFrame<ezGPUParticleRenderData>(nullptr);

  pRenderData->m_uiSortingKey = ComputeSortingKey(m_RenderMode, m_hTexture.GetResourceIDHash(), 0);
  pRenderData->m_vGlobalPosition = instanceTransform.m_vPosition;
  pRenderData->m_GlobalTransform = GetOwnerEffect()->NeedsToApplyTransform() ? instanceTransform : ezTransform::MakeIdentity();
  pRenderData->m_TotalEffectLifeTime = GetOwnerEffect()->GetTotalEffectLifeTime();

  pRenderData->m_hTexture = m_hTexture;
  pRenderData->m_RenderMode = (ezParticleTypeRenderMode::Enum)m_RenderMode;
  pRenderData->m_uiMaxParticles = m_uiMaxGPUParticles;

  pRenderData->m_hParticleBuffer = m_hParticleBuffer;
  pRenderData->m_hCounterBuffer = m_hCounterBuffer;

  pRenderData->m_NewParticles = newParticles;
  pRenderData->m_uiEmitStartIndex = m_uiGPUEmitIndex;

  pRenderData->m_fGravity = m_fGravity;
  pRenderData->m_fDragCoefficient = m_fDragCoefficient;
  pRenderData->m_fWindStrength = m_fWindStrength;
  pRenderData->m_bEnableDepthCollision = m_bEnableDepthCollision;
  pRenderData->m_bEnableSDFCollision = m_bEnableSDFCollision;
  pRenderData->m_uiCollisionReaction = m_CollisionReaction;
  pRenderData->m_fCollisionBounceFactor = m_fCollisionBounceFactor;
  pRenderData->m_fCollisionSlideFactor = m_fCollisionSlideFactor;
  pRenderData->m_fCollisionThickness = m_fCollisionThickness;
  pRenderData->m_ColorStart = m_ColorStart;
  pRenderData->m_ColorEnd = m_ColorEnd;

  pRenderData->m_uiGPURenderType = m_GPURenderType;
  pRenderData->m_uiMaxTrailPoints = m_uiMaxTrailPoints;
  pRenderData->m_hTrailPositionBuffer = m_hTrailPositionBuffer;
  pRenderData->m_fVelocityStretch = m_fVelocityStretch;

  ref_msg.AddRenderData(pRenderData, ezDefaultRenderDataCategories::LitTransparent, ezRenderData::Caching::Never);

  // Register with the data provider so the compute pass can simulate this system
  {
    ezGPUParticleSystemInfo sysInfo;
    sysInfo.m_hParticleBuffer = m_hParticleBuffer;
    sysInfo.m_hCounterBuffer = m_hCounterBuffer;
    sysInfo.m_uiMaxParticles = m_uiMaxGPUParticles;
    sysInfo.m_NewParticles = newParticles;
    sysInfo.m_uiEmitStartIndex = m_uiGPUEmitIndex;
    sysInfo.m_fGravity = m_fGravity;
    sysInfo.m_fDragCoefficient = m_fDragCoefficient;
    sysInfo.m_fWindStrength = m_fWindStrength;

    // Sample wind at the effect's position
    if (const ezWindWorldModuleInterface* pWind = GetOwnerEffect()->GetWorld()->GetModuleReadOnly<ezWindWorldModuleInterface>())
    {
      sysInfo.m_vWindDirection = pWind->GetWindAt(instanceTransform.m_vPosition);
    }

    sysInfo.m_bEnableDepthCollision = m_bEnableDepthCollision;
    sysInfo.m_bEnableSDFCollision = m_bEnableSDFCollision;
    sysInfo.m_uiCollisionReaction = m_CollisionReaction;
    sysInfo.m_fCollisionBounceFactor = m_fCollisionBounceFactor;
    sysInfo.m_fCollisionSlideFactor = m_fCollisionSlideFactor;
    sysInfo.m_fCollisionThickness = m_fCollisionThickness;
    sysInfo.m_ColorStart = m_ColorStart;
    sysInfo.m_ColorEnd = m_ColorEnd;

    sysInfo.m_uiGPURenderType = m_GPURenderType;
    sysInfo.m_uiMaxTrailPoints = m_uiMaxTrailPoints;
    sysInfo.m_uiTrailWriteIndex = m_uiTrailWriteIndex;
    sysInfo.m_hTrailPositionBuffer = m_hTrailPositionBuffer;
    sysInfo.m_fVelocityStretch = m_fVelocityStretch;

    ezGPUParticleDataProvider::QueueSystem(sysInfo);
  }

  // Advance emit index (wrap around)
  m_uiGPUEmitIndex = (m_uiGPUEmitIndex + numParticles) % m_uiMaxGPUParticles;

  // Advance trail write index for trail types
  if (m_GPURenderType == ezGPUParticleRenderType::Trail)
  {
    m_uiTrailWriteIndex++;
  }
}


EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Type_GPU_ParticleTypeGPU);
