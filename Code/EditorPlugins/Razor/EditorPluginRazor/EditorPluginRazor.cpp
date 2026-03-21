#include <EditorPluginRazor/EditorPluginRazorPCH.h>

#include <EditorFramework/Actions/AssetActions.h>
#include <EditorFramework/Actions/ProjectActions.h>
#include <EditorPluginRazor/Actions/RazorAssetActions.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/Action/CommandHistoryActions.h>
#include <GuiFoundation/Action/DocumentActions.h>
#include <GuiFoundation/Action/StandardMenus.h>

static void OnLoadPlugin()
{
  // Register Razor actions
  ezRazorAssetActions::RegisterActions();

  // Razor Document
  {
    // Menu Bar
    {
      ezActionMapManager::RegisterActionMap("RazorAssetMenuBar", "AssetMenuBar");
      ezRazorAssetActions::MapMenuActions("RazorAssetMenuBar");
    }

    // Tool Bar
    {
      ezActionMapManager::RegisterActionMap("RazorAssetToolBar", "AssetToolbar");
      ezRazorAssetActions::MapToolbarActions("RazorAssetToolBar");
    }
  }
}

static void OnUnloadPlugin()
{
  ezRazorAssetActions::UnregisterActions();
}

EZ_PLUGIN_ON_LOADED()
{
  OnLoadPlugin();
}

EZ_PLUGIN_ON_UNLOADED()
{
  OnUnloadPlugin();
}
