#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorPluginAssets/AnimationGraphAsset/AnimGraphActions.h>
#include <EditorPluginAssets/AnimationGraphAsset/AnimationGraphAsset.h>
#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezAnimGraphAction, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

ezActionDescriptorHandle ezAnimGraphActions::s_hCategory;
ezActionDescriptorHandle ezAnimGraphActions::s_hRenderBones;
ezActionDescriptorHandle ezAnimGraphActions::s_hRenderPreviewMesh;

void ezAnimGraphActions::RegisterActions()
{
  s_hCategory = EZ_REGISTER_CATEGORY("AnimGraphCategory");
  s_hRenderBones = EZ_REGISTER_ACTION_1("AnimGraph.RenderBones", ezActionScope::Document, "Animation Graph", "", ezAnimGraphAction, ezAnimGraphAction::ActionType::RenderBones);
  s_hRenderPreviewMesh = EZ_REGISTER_ACTION_1("AnimGraph.RenderPreviewMesh", ezActionScope::Document, "Animation Graph", "", ezAnimGraphAction, ezAnimGraphAction::ActionType::RenderPreviewMesh);
}

void ezAnimGraphActions::UnregisterActions()
{
  ezActionManager::UnregisterAction(s_hCategory);
  ezActionManager::UnregisterAction(s_hRenderBones);
  ezActionManager::UnregisterAction(s_hRenderPreviewMesh);
}

void ezAnimGraphActions::MapActions(ezStringView sMapping)
{
  ezActionMap* pMap = ezActionMapManager::GetActionMap(sMapping);
  EZ_ASSERT_DEV(pMap != nullptr, "The given mapping ('{0}') does not exist, mapping the actions failed!", sMapping);

  pMap->MapAction(s_hCategory, "", 11.0f);

  const char* szSubPath = "AnimGraphCategory";

  pMap->MapAction(s_hRenderBones, szSubPath, 1.0f);
  pMap->MapAction(s_hRenderPreviewMesh, szSubPath, 2.0f);
}

ezAnimGraphAction::ezAnimGraphAction(const ezActionContext& context, const char* szName, ezAnimGraphAction::ActionType type)
  : ezButtonAction(context, szName, false, "")
{
  m_Type = type;

  m_pDocument = const_cast<ezAnimationGraphAssetDocument*>(static_cast<const ezAnimationGraphAssetDocument*>(context.m_pDocument));
  m_pDocument->Events().AddEventHandler(ezMakeDelegate(&ezAnimGraphAction::AssetEventHandler, this));

  switch (m_Type)
  {
    case ActionType::RenderBones:
      SetIconPath(":/EditorPluginAssets/SkeletonBones.svg");
      break;

    case ActionType::RenderPreviewMesh:
      SetIconPath(":/EditorPluginAssets/PreviewMesh.svg");
      break;
  }

  UpdateState();
}

ezAnimGraphAction::~ezAnimGraphAction()
{
  m_pDocument->Events().RemoveEventHandler(ezMakeDelegate(&ezAnimGraphAction::AssetEventHandler, this));
}

void ezAnimGraphAction::Execute(const ezVariant& value)
{
  switch (m_Type)
  {
    case ActionType::RenderBones:
      m_pDocument->SetRenderBones(!m_pDocument->GetRenderBones());
      return;

    case ActionType::RenderPreviewMesh:
      m_pDocument->SetRenderPreviewMesh(!m_pDocument->GetRenderPreviewMesh());
      return;
  }
}

void ezAnimGraphAction::AssetEventHandler(const ezAnimationGraphAssetEvent& e)
{
  switch (e.m_Type)
  {
    case ezAnimationGraphAssetEvent::RenderStateChanged:
      UpdateState();
      break;
    default:
      break;
  }
}

void ezAnimGraphAction::UpdateState()
{
  switch (m_Type)
  {
    case ActionType::RenderBones:
      SetCheckable(true);
      SetChecked(m_pDocument->GetRenderBones());
      break;

    case ActionType::RenderPreviewMesh:
      SetCheckable(true);
      SetChecked(m_pDocument->GetRenderPreviewMesh());
      break;
  }
}
