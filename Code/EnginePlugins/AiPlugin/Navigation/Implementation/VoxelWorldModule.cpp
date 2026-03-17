#include <AiPlugin/AiPluginPCH.h>

#include <AiPlugin/Navigation/Components/VoxelGridSettingsComponent.h>
#include <AiPlugin/Navigation/VoxelWorldModule.h>
#include <Core/Interfaces/PhysicsWorldModule.h>
#include <Core/World/World.h>
#include <Foundation/Configuration/CVar.h>

ezCVarBool cvar_VoxelGridVisualize("AI.VoxelGrid.Visualize", false, ezCVarFlags::None, "Visualize the voxel grid.");

// clang-format off
EZ_IMPLEMENT_WORLD_MODULE(ezAiVoxelWorldModule);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAiVoxelWorldModule, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezAiVoxelWorldModule::ezAiVoxelWorldModule(ezWorld* pWorld)
  : ezWorldModule(pWorld)
{
}

ezAiVoxelWorldModule::~ezAiVoxelWorldModule() = default;

void ezAiVoxelWorldModule::Initialize()
{
  SUPER::Initialize();

  {
    auto updateDesc = EZ_CREATE_MODULE_UPDATE_FUNCTION_DESC(ezAiVoxelWorldModule::Update, this);
    updateDesc.m_Phase = ezWorldUpdatePhase::PostTransform;
    updateDesc.m_bOnlyUpdateWhenSimulating = true;

    RegisterUpdateFunction(updateDesc);
  }

  m_bNeedsVoxelization = true;
}

void ezAiVoxelWorldModule::Deinitialize()
{
  SUPER::Deinitialize();
}

void ezAiVoxelWorldModule::Update(const UpdateContext& ctxt)
{
  if (m_uiUpdateDelay > 0)
  {
    --m_uiUpdateDelay;
    return;
  }

  if (m_bNeedsVoxelization)
  {
    m_bNeedsVoxelization = false;

    // Read settings from the settings component if one exists in the scene
    if (auto* pSettingsManager = GetWorld()->GetComponentManager<ezAiVoxelGridSettingsComponentManager>())
    {
      if (auto* pSettings = pSettingsManager->GetSingletonComponent())
      {
        m_uiResolutionX = pSettings->GetResolutionX();
        m_uiResolutionY = pSettings->GetResolutionY();
        m_uiResolutionZ = pSettings->GetResolutionZ();
        m_fVoxelSize = pSettings->GetVoxelSize();
        m_uiCollisionLayer = pSettings->GetCollisionLayer();
        m_vGridCenter = pSettings->GetOwner()->GetGlobalPosition();
      }
    }

    m_VoxelGrid.Init(m_uiResolutionX, m_uiResolutionY, m_uiResolutionZ);
    m_VoxelGrid.SetWorldParameters(m_vGridCenter, m_fVoxelSize);
    VoxelizeWorld(m_uiCollisionLayer);
  }

  if (cvar_VoxelGridVisualize)
  {
    m_VoxelGrid.DebugDraw(GetWorld(), ezColor::LimeGreen.WithAlpha(0.1f));
  }
}

void ezAiVoxelWorldModule::VoxelizeWorld(ezUInt32 uiCollisionLayer)
{
  auto* pPhysics = GetWorld()->GetModule<ezPhysicsWorldModuleInterface>();
  if (pPhysics == nullptr)
  {
    ezLog::Warning("ezAiVoxelWorldModule: No physics module available for voxelization.");
    return;
  }

  m_VoxelGrid.ClearData();

  const ezPhysicsQueryParameters queryParams(uiCollisionLayer, ezPhysicsShapeType::Static | ezPhysicsShapeType::Dynamic);
  const float fHalfVoxel = m_VoxelGrid.GetVoxelSize() * 0.5f;
  const ezVec3 vBoxExtents(fHalfVoxel, fHalfVoxel, fHalfVoxel);

  const ezUInt32 uiDimX = m_VoxelGrid.GetDimX();
  const ezUInt32 uiDimY = m_VoxelGrid.GetDimY();
  const ezUInt32 uiDimZ = m_VoxelGrid.GetDimZ();

  for (ezUInt32 z = 0; z < uiDimZ; ++z)
  {
    for (ezUInt32 y = 0; y < uiDimY; ++y)
    {
      for (ezUInt32 x = 0; x < uiDimX; ++x)
      {
        const ezVec3I32 vCoord((ezInt32)x, (ezInt32)y, (ezInt32)z);
        const ezVec3 vWorldPos = m_VoxelGrid.CoordToWorld(vCoord);

        if (pPhysics->OverlapTestBox(vBoxExtents, vWorldPos, ezTransform::MakeIdentity(), queryParams))
        {
          m_VoxelGrid.SetVoxel(vCoord, true);
        }
      }
    }
  }

  ezLog::Info("ezAiVoxelWorldModule: Voxelized world ({}x{}x{}, voxel size {}).",
    uiDimX, uiDimY, uiDimZ, m_VoxelGrid.GetVoxelSize());
}

void ezAiVoxelWorldModule::InjectObstacle(const ezBoundingBox& box)
{
  m_VoxelGrid.InjectBox(box);
}

void ezAiVoxelWorldModule::RemoveObstacle(const ezBoundingBox& box)
{
  m_VoxelGrid.SubtractBox(box);
}

EZ_STATICLINK_FILE(AiPlugin, AiPlugin_Navigation_Implementation_VoxelWorldModule);
