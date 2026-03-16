#pragma once

#include <EditorPluginAssets/EditorPluginAssetsDLL.h>
#include <GuiFoundation/Action/BaseActions.h>
#include <GuiFoundation/GuiFoundationDLL.h>

class ezAnimationGraphAssetDocument;
struct ezAnimationGraphAssetEvent;

class ezAnimGraphActions
{
public:
  static void RegisterActions();
  static void UnregisterActions();

  static void MapActions(ezStringView sMapping);

  static ezActionDescriptorHandle s_hCategory;
  static ezActionDescriptorHandle s_hRenderBones;
  static ezActionDescriptorHandle s_hRenderPreviewMesh;
};

class ezAnimGraphAction : public ezButtonAction
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAnimGraphAction, ezButtonAction);

public:
  enum class ActionType
  {
    RenderBones,
    RenderPreviewMesh,
  };

  ezAnimGraphAction(const ezActionContext& context, const char* szName, ActionType type);
  ~ezAnimGraphAction();

  virtual void Execute(const ezVariant& value) override;

private:
  void AssetEventHandler(const ezAnimationGraphAssetEvent& e);
  void UpdateState();

  ezAnimationGraphAssetDocument* m_pDocument = nullptr;
  ActionType m_Type;
};
