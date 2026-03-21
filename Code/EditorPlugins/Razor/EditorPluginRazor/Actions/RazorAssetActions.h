#pragma once

#include <EditorPluginRazor/EditorPluginRazorDLL.h>
#include <GuiFoundation/Action/BaseActions.h>

class ezRazorDocumentAsset;

/// Actions for Razor asset document toolbar.
class ezRazorAssetActions
{
public:
  static void RegisterActions();
  static void UnregisterActions();

  static void MapMenuActions(ezStringView sMapping);
  static void MapToolbarActions(ezStringView sMapping);

  static ezActionDescriptorHandle s_hCategory;
  static ezActionDescriptorHandle s_hHotReload;
};

/// Hot reload action for Razor documents.
class ezRazorHotReloadAction : public ezButtonAction
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorHotReloadAction, ezButtonAction);

public:
  ezRazorHotReloadAction(const ezActionContext& context, const char* szName, const char* szIconPath);
  ~ezRazorHotReloadAction();

  virtual void Execute(const ezVariant& value) override;

private:
  ezRazorDocumentAsset* m_pDocument = nullptr;
};
