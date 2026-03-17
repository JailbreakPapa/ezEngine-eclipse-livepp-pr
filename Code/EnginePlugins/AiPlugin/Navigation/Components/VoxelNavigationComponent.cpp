#include <AiPlugin/AiPluginPCH.h>

#include <AiPlugin/Navigation/Components/VoxelNavigationComponent.h>
#include <AiPlugin/Navigation/VoxelWorldModule.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Debug/DebugRenderer.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_BITFLAGS(ezAiVoxelNavigationDebugFlags, 1)
  EZ_BITFLAGS_CONSTANTS(ezAiVoxelNavigationDebugFlags::PrintState, ezAiVoxelNavigationDebugFlags::VisPath, ezAiVoxelNavigationDebugFlags::VisGrid)
EZ_END_STATIC_REFLECTED_BITFLAGS;

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezAiVoxelNavigationComponentState, 1)
  EZ_ENUM_CONSTANTS(ezAiVoxelNavigationComponentState::Idle, ezAiVoxelNavigationComponentState::Moving, ezAiVoxelNavigationComponentState::Failed)
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_COMPONENT_TYPE(ezAiVoxelNavigationComponent, 1, ezComponentMode::Dynamic)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Speed", m_fSpeed)->AddAttributes(new ezDefaultValueAttribute(5.0f)),
    EZ_MEMBER_PROPERTY("Acceleration", m_fAcceleration)->AddAttributes(new ezDefaultValueAttribute(3.0f)),
    EZ_MEMBER_PROPERTY("Deceleration", m_fDeceleration)->AddAttributes(new ezDefaultValueAttribute(8.0f)),
    EZ_MEMBER_PROPERTY("ReachedDistance", m_fReachedDistance)->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.0f, 10.0f)),
    EZ_MEMBER_PROPERTY("ApplySteering", m_bApplySteering)->AddAttributes(new ezDefaultValueAttribute(true)),
    EZ_BITFLAGS_MEMBER_PROPERTY("DebugFlags", ezAiVoxelNavigationDebugFlags, m_DebugFlags),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("AI/Navigation"),
  }
  EZ_END_ATTRIBUTES;
  EZ_BEGIN_FUNCTIONS
  {
    EZ_SCRIPT_FUNCTION_PROPERTY(SetDestination, In, "Destination"),
    EZ_SCRIPT_FUNCTION_PROPERTY(CancelNavigation),
    EZ_SCRIPT_FUNCTION_PROPERTY(GetState),
    EZ_SCRIPT_FUNCTION_PROPERTY(GetSteeringPosition),
    EZ_SCRIPT_FUNCTION_PROPERTY(GetSteeringRotation),
  }
  EZ_END_FUNCTIONS;
}
EZ_END_COMPONENT_TYPE
// clang-format on

ezAiVoxelNavigationComponent::ezAiVoxelNavigationComponent() = default;
ezAiVoxelNavigationComponent::~ezAiVoxelNavigationComponent() = default;

void ezAiVoxelNavigationComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  m_uiSkipNextFrames = 3;
  m_vSteerPosition = GetOwner()->GetGlobalPosition();
  m_qSteerRotation = GetOwner()->GetGlobalRotation();
  m_vVelocity = ezVec3::MakeZero();
}

void ezAiVoxelNavigationComponent::SetDestination(const ezVec3& vGlobalPos)
{
  auto* pVoxelModule = GetWorld()->GetOrCreateModule<ezAiVoxelWorldModule>();
  if (pVoxelModule == nullptr)
  {
    m_State = ezAiVoxelNavigationComponentState::Failed;
    return;
  }

  m_Navigation.SetVoxelGrid(pVoxelModule->GetVoxelGrid());

  const ezVec3 vCurrentPos = GetOwner()->GetGlobalPosition();
  const auto result = m_Navigation.FindPath(vCurrentPos, vGlobalPos);

  if (result == ezAiVoxelNavigation::State::PathFound)
  {
    m_State = ezAiVoxelNavigationComponentState::Moving;
    m_vVelocity = ezVec3::MakeZero();
  }
  else
  {
    m_State = ezAiVoxelNavigationComponentState::Failed;
  }
}

void ezAiVoxelNavigationComponent::CancelNavigation()
{
  m_Navigation.CancelNavigation();
  m_State = ezAiVoxelNavigationComponentState::Idle;
  m_vVelocity = ezVec3::MakeZero();
}

ezVec3 ezAiVoxelNavigationComponent::GetSteeringPosition() const
{
  return m_vSteerPosition;
}

ezQuat ezAiVoxelNavigationComponent::GetSteeringRotation() const
{
  return m_qSteerRotation;
}

void ezAiVoxelNavigationComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  ezStreamWriter& s = inout_stream.GetStream();

  s << m_fSpeed;
  s << m_fAcceleration;
  s << m_fDeceleration;
  s << m_fReachedDistance;
  s << m_bApplySteering;
  s << m_DebugFlags;
}

void ezAiVoxelNavigationComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  ezStreamReader& s = inout_stream.GetStream();

  s >> m_fSpeed;
  s >> m_fAcceleration;
  s >> m_fDeceleration;
  s >> m_fReachedDistance;
  s >> m_bApplySteering;
  s >> m_DebugFlags;
}

void ezAiVoxelNavigationComponent::Update()
{
  if (m_uiSkipNextFrames > 0)
  {
    m_uiSkipNextFrames--;
    return;
  }

  const float tDiff = GetWorld()->GetClock().GetTimeDiff().AsFloatInSeconds();

  if (m_State == ezAiVoxelNavigationComponentState::Moving)
  {
    MoveTowardsWaypoint(tDiff);
  }

  if (m_DebugFlags.IsAnyFlagSet())
  {
    if (m_DebugFlags.IsSet(ezAiVoxelNavigationDebugFlags::PrintState))
    {
      const ezVec3 vPosition = GetOwner()->GetGlobalPosition() + ezVec3(0, 0, 1.5f);

      switch (m_State)
      {
        case ezAiVoxelNavigationComponentState::Idle:
          ezDebugRenderer::Draw3DText(GetWorld(), "Idle", vPosition, ezColor::Grey);
          break;
        case ezAiVoxelNavigationComponentState::Moving:
          ezDebugRenderer::Draw3DText(GetWorld(), "Moving", vPosition, ezColor::Yellow);
          break;
        case ezAiVoxelNavigationComponentState::Failed:
          ezDebugRenderer::Draw3DText(GetWorld(), "Failed", vPosition, ezColor::Red);
          break;
      }
    }

    if (m_DebugFlags.IsSet(ezAiVoxelNavigationDebugFlags::VisPath))
    {
      m_Navigation.DebugDrawPath(GetWorld(), ezColor::DeepSkyBlue);
    }

    if (m_DebugFlags.IsSet(ezAiVoxelNavigationDebugFlags::VisGrid))
    {
      auto* pVoxelModule = GetWorld()->GetModule<ezAiVoxelWorldModule>();
      if (pVoxelModule != nullptr)
      {
        pVoxelModule->GetVoxelGrid()->DebugDraw(GetWorld(), ezColor::LimeGreen.WithAlpha(0.1f));
      }
    }
  }
}

void ezAiVoxelNavigationComponent::MoveTowardsWaypoint(float fTimeDiff)
{
  if (m_Navigation.IsPathComplete())
  {
    m_State = ezAiVoxelNavigationComponentState::Idle;
    m_vVelocity = ezVec3::MakeZero();
    return;
  }

  const ezVec3 vCurrentPos = m_vSteerPosition;
  const ezVec3 vTargetWaypoint = m_Navigation.GetNextWaypoint();
  ezVec3 vDirection = vTargetWaypoint - vCurrentPos;
  const float fDistanceToWaypoint = vDirection.GetLength();

  if (fDistanceToWaypoint < 0.001f)
  {
    if (!m_Navigation.AdvanceWaypoint())
    {
      m_State = ezAiVoxelNavigationComponentState::Idle;
      m_vVelocity = ezVec3::MakeZero();
      return;
    }
    return;
  }

  vDirection /= fDistanceToWaypoint;

  // Compute target speed with braking
  float fTargetSpeed = m_fSpeed;

  // Check if we should start decelerating
  const float fBrakingDistance = (m_fSpeed * m_fSpeed) / (2.0f * m_fDeceleration);

  if (m_Navigation.IsPathComplete() || m_Navigation.GetCurrentWaypointIndex() >= m_Navigation.GetWaypoints().GetCount() - 2)
  {
    // Near the end of the path, consider arrival distance
    const ezVec3 vFinalTarget = m_Navigation.GetWaypoints()[m_Navigation.GetWaypoints().GetCount() - 1];
    const float fDistToEnd = (vFinalTarget - vCurrentPos).GetLength();

    if (fDistToEnd < fBrakingDistance)
    {
      fTargetSpeed = m_fSpeed * (fDistToEnd / fBrakingDistance);
      fTargetSpeed = ezMath::Max(fTargetSpeed, 0.5f);
    }
  }

  // Accelerate or decelerate
  const float fCurrentSpeed = m_vVelocity.GetLength();
  float fNewSpeed;

  if (fCurrentSpeed < fTargetSpeed)
  {
    fNewSpeed = ezMath::Min(fCurrentSpeed + m_fAcceleration * fTimeDiff, fTargetSpeed);
  }
  else
  {
    fNewSpeed = ezMath::Max(fCurrentSpeed - m_fDeceleration * fTimeDiff, fTargetSpeed);
  }

  m_vVelocity = vDirection * fNewSpeed;

  // Move position
  m_vSteerPosition = vCurrentPos + m_vVelocity * fTimeDiff;

  // Update rotation to face movement direction
  if (fNewSpeed > 0.1f)
  {
    m_qSteerRotation = ezQuat::MakeShortestRotation(ezVec3::MakeAxisX(), vDirection);
  }

  // Check if we reached the waypoint
  if (fDistanceToWaypoint <= m_fReachedDistance)
  {
    if (!m_Navigation.AdvanceWaypoint())
    {
      m_State = ezAiVoxelNavigationComponentState::Idle;
      m_vVelocity = ezVec3::MakeZero();
    }
  }

  if (m_bApplySteering)
  {
    GetOwner()->SetGlobalPosition(m_vSteerPosition);
    GetOwner()->SetGlobalRotation(m_qSteerRotation);
  }
}

EZ_STATICLINK_FILE(AiPlugin, AiPlugin_Navigation_Components_VoxelNavigationComponent);
