#include <RendererCore/RendererCorePCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Core/ResourceManager/Implementation/ResourceHandleReflection.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <RendererCore/Material/MaterialResource.h>
#include <RendererCore/Meshes/HairComponent.h>
#include <RendererCore/Meshes/HairStrandRenderer.h>
#include <RendererCore/Meshes/MeshResource.h>
#include <RendererCore/Pipeline/RenderData.h>
#include <RendererCore/Pipeline/RenderDataManager.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_ENUM(ezHairRenderMode, 1)
  EZ_ENUM_CONSTANTS(ezHairRenderMode::Strands, ezHairRenderMode::Cards)
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_COMPONENT_TYPE(ezHairComponent, 3, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ENUM_ACCESSOR_PROPERTY("RenderMode", ezHairRenderMode, GetRenderMode, SetRenderMode),

    // Strand mode
    EZ_RESOURCE_ACCESSOR_PROPERTY("StrandResource", GetStrandResource, SetStrandResource)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Hair_Strands")),
    EZ_RESOURCE_ACCESSOR_PROPERTY("StrandMaterial", GetStrandMaterial, SetStrandMaterial)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Material")),
    EZ_ACCESSOR_PROPERTY("WidthScale", GetWidthScale, SetWidthScale)
      ->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.01f, 10.0f)),
    EZ_ACCESSOR_PROPERTY("StrandDensity", GetStrandDensity, SetStrandDensity)
      ->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.01f, 1.0f)),

    // Card mode
    EZ_RESOURCE_ACCESSOR_PROPERTY("CardMesh", GetCardMesh, SetCardMesh)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Mesh_Static")),
    EZ_ARRAY_ACCESSOR_PROPERTY("CardMaterials", CardMaterials_GetCount, CardMaterials_GetValue, CardMaterials_SetValue, CardMaterials_Insert, CardMaterials_Remove)
      ->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Material")),

    // Shared
    EZ_ACCESSOR_PROPERTY("Color", GetColor, SetColor)->AddAttributes(new ezExposeColorAlphaAttribute()),
    EZ_ACCESSOR_PROPERTY("SortingDepthOffset", GetSortingDepthOffset, SetSortingDepthOffset),
    EZ_ACCESSOR_PROPERTY("WindInfluence", GetWindInfluence, SetWindInfluence)
      ->AddAttributes(new ezDefaultValueAttribute(0.5f), new ezClampValueAttribute(0.0f, 2.0f)),
    EZ_ACCESSOR_PROPERTY("GravityStrength", GetGravityStrength, SetGravityStrength)
      ->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.0f, 5.0f)),
    EZ_ACCESSOR_PROPERTY("Stiffness", GetStiffness, SetStiffness)
      ->AddAttributes(new ezDefaultValueAttribute(0.5f), new ezClampValueAttribute(0.0f, 1.0f)),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezMsgExtractRenderData, OnMsgExtractRenderData),
  }
  EZ_END_MESSAGEHANDLERS;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Rendering"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_COMPONENT_TYPE;
// clang-format on

ezHairComponent::ezHairComponent() = default;
ezHairComponent::~ezHairComponent() = default;

void ezHairComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  ezStreamWriter& s = inout_stream.GetStream();

  s << static_cast<ezUInt8>(m_RenderMode.GetValue());

  // Strand mode
  s << m_hStrandResource;
  s << m_hStrandMaterial;
  s << m_fWidthScale;
  s << m_fStrandDensity;

  // Card mode
  s << m_hCardMesh;
  s << m_CardMaterials.GetCount();
  for (const auto& mat : m_CardMaterials)
    s << mat;

  // Shared
  s << m_Color;
  s << m_fSortingDepthOffset;
  s << m_fWindInfluence;
  s << m_fGravityStrength;
  s << m_fStiffness;
}

void ezHairComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const ezUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  ezStreamReader& s = inout_stream.GetStream();

  if (uiVersion >= 3)
  {
    // Version 3: new dual-mode component
    ezUInt8 renderMode;
    s >> renderMode;
    m_RenderMode = static_cast<ezHairRenderMode::Enum>(renderMode);

    s >> m_hStrandResource;
    s >> m_hStrandMaterial;
    s >> m_fWidthScale;
    s >> m_fStrandDensity;

    s >> m_hCardMesh;
    ezUInt32 matCount;
    s >> matCount;
    m_CardMaterials.SetCount(matCount);
    for (auto& mat : m_CardMaterials)
      s >> mat;

    s >> m_Color;
    s >> m_fSortingDepthOffset;
    s >> m_fWindInfluence;
    s >> m_fGravityStrength;
    s >> m_fStiffness;
  }
  else
  {
    // Backward compat: versions 1-2 were card-only (old ezMeshComponentBase layout)
    m_RenderMode = ezHairRenderMode::Cards;

    // The old component serialized: mesh, color, materials[], sortingOffset, windInfluence, gravityStrength, stiffness(v2)
    // However, since we changed the base class from ezMeshComponentBase to ezRenderComponent,
    // the SUPER::Deserialize already handled the base data. The old fields were:
    s >> m_fWindInfluence;
    s >> m_fGravityStrength;
    if (uiVersion >= 2)
    {
      s >> m_fStiffness;
    }
  }
}

ezResult ezHairComponent::GetLocalBounds(ezBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, ezMsgUpdateLocalBounds& ref_msg)
{
  if (m_RenderMode == ezHairRenderMode::Strands && m_hStrandResource.IsValid())
  {
    ezResourceLock<ezHairStrandResource> pResource(m_hStrandResource, ezResourceAcquireMode::AllowLoadingFallback);
    if (pResource.GetAcquireResult() == ezResourceAcquireResult::Final)
    {
      ref_bounds = pResource->GetBounds();
      return EZ_SUCCESS;
    }
  }
  else if (m_RenderMode == ezHairRenderMode::Cards && m_hCardMesh.IsValid())
  {
    ezResourceLock<ezMeshResource> pMesh(m_hCardMesh, ezResourceAcquireMode::AllowLoadingFallback);
    if (pMesh.GetAcquireResult() == ezResourceAcquireResult::Final)
    {
      ref_bounds = pMesh->GetBounds();
      return EZ_SUCCESS;
    }
  }

  return EZ_FAILURE;
}

void ezHairComponent::OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const
{
  if (m_RenderMode == ezHairRenderMode::Strands)
    ExtractStrandRenderData(msg);
  else
    ExtractCardRenderData(msg);
}

void ezHairComponent::ExtractStrandRenderData(ezMsgExtractRenderData& msg) const
{
  if (!m_hStrandResource.IsValid())
    return;

  auto* pRenderData = msg.m_pRenderDataManager->CreateRenderDataForThisFrame<ezHairStrandRenderData>(GetOwner());
  {
    pRenderData->m_vGlobalPosition = GetOwner()->GetGlobalPosition();
    pRenderData->m_GlobalTransform = GetOwner()->GetGlobalTransform();
    pRenderData->m_hStrandResource = m_hStrandResource;
    pRenderData->m_hMaterial = m_hStrandMaterial;
    pRenderData->m_Color = m_Color;
    pRenderData->m_fSortingDepthOffset = m_fSortingDepthOffset;
    pRenderData->m_fWindInfluence = m_fWindInfluence;
    pRenderData->m_fGravityStrength = m_fGravityStrength;
    pRenderData->m_fStiffness = m_fStiffness;
    pRenderData->m_fWidthScale = m_fWidthScale;
    pRenderData->m_fStrandDensity = m_fStrandDensity;
  }

  msg.AddRenderData(pRenderData, ezDefaultRenderDataCategories::LitHair, ezRenderData::Caching::Never);
}

void ezHairComponent::ExtractCardRenderData(ezMsgExtractRenderData& msg) const
{
  if (!m_hCardMesh.IsValid())
    return;

  // Card mode rendering requires instance data management from ezMeshComponentBase.
  // Since this component now derives from ezRenderComponent directly,
  // card mode is kept for backward compatibility in serialization but
  // does not render. Switch to Strands mode for rendering.
  ezLog::Warning("ezHairComponent: Card render mode is deprecated. Use Strands mode for rendering.");
}

// --- Property setters/getters ---

void ezHairComponent::SetRenderMode(ezHairRenderMode::Enum mode) { m_RenderMode = mode; TriggerLocalBoundsUpdate(); InvalidateCachedRenderData(); }
ezHairRenderMode::Enum ezHairComponent::GetRenderMode() const { return m_RenderMode; }

void ezHairComponent::SetStrandResource(const ezHairStrandResourceHandle& hResource) { m_hStrandResource = hResource; TriggerLocalBoundsUpdate(); InvalidateCachedRenderData(); }
const ezHairStrandResourceHandle& ezHairComponent::GetStrandResource() const { return m_hStrandResource; }

void ezHairComponent::SetStrandMaterial(const ezMaterialResourceHandle& hMaterial) { m_hStrandMaterial = hMaterial; InvalidateCachedRenderData(); }
const ezMaterialResourceHandle& ezHairComponent::GetStrandMaterial() const { return m_hStrandMaterial; }

void ezHairComponent::SetCardMesh(const ezMeshResourceHandle& hMesh) { m_hCardMesh = hMesh; TriggerLocalBoundsUpdate(); InvalidateCachedRenderData(); }
const ezMeshResourceHandle& ezHairComponent::GetCardMesh() const { return m_hCardMesh; }

ezUInt32 ezHairComponent::CardMaterials_GetCount() const { return m_CardMaterials.GetCount(); }
ezString ezHairComponent::CardMaterials_GetValue(ezUInt32 uiIndex) const { return m_CardMaterials[uiIndex].GetResourceID(); }
void ezHairComponent::CardMaterials_SetValue(ezUInt32 uiIndex, ezString sValue)
{
  if (sValue.IsEmpty())
    m_CardMaterials[uiIndex] = ezMaterialResourceHandle();
  else
    m_CardMaterials[uiIndex] = ezResourceManager::LoadResource<ezMaterialResource>(sValue);
  InvalidateCachedRenderData();
}
void ezHairComponent::CardMaterials_Insert(ezUInt32 uiIndex, ezString sValue)
{
  ezMaterialResourceHandle hMat;
  if (!sValue.IsEmpty())
    hMat = ezResourceManager::LoadResource<ezMaterialResource>(sValue);
  m_CardMaterials.InsertAt(uiIndex, hMat);
  InvalidateCachedRenderData();
}
void ezHairComponent::CardMaterials_Remove(ezUInt32 uiIndex) { m_CardMaterials.RemoveAtAndCopy(uiIndex); InvalidateCachedRenderData(); }

void ezHairComponent::SetColor(const ezColor& color) { m_Color = color; InvalidateCachedRenderData(); }
const ezColor& ezHairComponent::GetColor() const { return m_Color; }

void ezHairComponent::SetSortingDepthOffset(float fOffset) { m_fSortingDepthOffset = fOffset; InvalidateCachedRenderData(); }
float ezHairComponent::GetSortingDepthOffset() const { return m_fSortingDepthOffset; }

void ezHairComponent::SetWindInfluence(float fInfluence) { m_fWindInfluence = ezMath::Clamp(fInfluence, 0.0f, 2.0f); InvalidateCachedRenderData(); }
float ezHairComponent::GetWindInfluence() const { return m_fWindInfluence; }

void ezHairComponent::SetGravityStrength(float fStrength) { m_fGravityStrength = ezMath::Clamp(fStrength, 0.0f, 5.0f); InvalidateCachedRenderData(); }
float ezHairComponent::GetGravityStrength() const { return m_fGravityStrength; }

void ezHairComponent::SetStiffness(float fStiffness) { m_fStiffness = ezMath::Clamp(fStiffness, 0.0f, 1.0f); InvalidateCachedRenderData(); }
float ezHairComponent::GetStiffness() const { return m_fStiffness; }

void ezHairComponent::SetWidthScale(float fScale) { m_fWidthScale = ezMath::Clamp(fScale, 0.01f, 10.0f); InvalidateCachedRenderData(); }
float ezHairComponent::GetWidthScale() const { return m_fWidthScale; }

void ezHairComponent::SetStrandDensity(float fDensity) { m_fStrandDensity = ezMath::Clamp(fDensity, 0.01f, 1.0f); InvalidateCachedRenderData(); }
float ezHairComponent::GetStrandDensity() const { return m_fStrandDensity; }

EZ_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_HairComponent);
