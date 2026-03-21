#include <EditorPluginRazor/EditorPluginRazorPCH.h>

#include <EditorPluginRazor/Actions/RazorAssetActions.h>
#include <EditorPluginRazor/RazorAsset/RazorAsset.h>
#include <EditorPluginRazor/RazorAsset/RazorAssetWindow.moc.h>
#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/ContainerWindow/ContainerWindow.moc.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorHotReloadAction, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezActionDescriptorHandle ezRazorAssetActions::s_hCategory;
ezActionDescriptorHandle ezRazorAssetActions::s_hHotReload;

void ezRazorAssetActions::RegisterActions()
{
  s_hCategory = EZ_REGISTER_CATEGORY("RazorCategory");
  s_hHotReload = EZ_REGISTER_ACTION_1("Razor.HotReload", ezActionScope::Document, "Razor", "Ctrl+R", ezRazorHotReloadAction, ":/EditorPluginRazor/Icons/Refresh.svg");
}

void ezRazorAssetActions::UnregisterActions()
{
  ezActionManager::UnregisterAction(s_hCategory);
  ezActionManager::UnregisterAction(s_hHotReload);
}

void ezRazorAssetActions::MapMenuActions(ezStringView sMapping)
{
  ezActionMap* pMap = ezActionMapManager::GetActionMap(sMapping);
  EZ_ASSERT_DEV(pMap != nullptr, "The given action map '{0}' does not exist", sMapping);

  pMap->MapAction(s_hCategory, "G.Asset", 5.0f);
  pMap->MapAction(s_hHotReload, "RazorCategory", 1.0f);
}

void ezRazorAssetActions::MapToolbarActions(ezStringView sMapping)
{
  ezActionMap* pMap = ezActionMapManager::GetActionMap(sMapping);
  EZ_ASSERT_DEV(pMap != nullptr, "The given action map '{0}' does not exist", sMapping);

  pMap->MapAction(s_hHotReload, "", 1.0f);
}

//////////////////////////////////////////////////////////////////////////
// ezRazorHotReloadAction
//////////////////////////////////////////////////////////////////////////

ezRazorHotReloadAction::ezRazorHotReloadAction(const ezActionContext& context, const char* szName, const char* szIconPath)
  : ezButtonAction(context, szName, false, szIconPath)
{
  m_pDocument = static_cast<ezRazorDocumentAsset*>(context.m_pDocument);
}

ezRazorHotReloadAction::~ezRazorHotReloadAction() = default;

void ezRazorHotReloadAction::Execute(const ezVariant& value)
{
  EZ_IGNORE_UNUSED(value);

  // Find the window and trigger hot reload
  auto* pContainer = ezQtContainerWindow::GetContainerWindow();
  if (pContainer)
  {
    ezHybridArray<ezQtDocumentWindow*, 16> docWindows;
    pContainer->GetDocumentWindows(docWindows);

    for (auto* pDocWindow : docWindows)
    {
      if (pDocWindow->GetDocument() == m_pDocument)
      {
        if (auto* pRazorWindow = qobject_cast<ezQtRazorAssetDocumentWindow*>(pDocWindow))
        {
          pRazorWindow->OnHotReloadClicked();
          return;
        }
      }
    }
  }
}

EZ_STATICLINK_FILE(EditorPluginRazor, EditorPluginRazor_Actions_RazorAssetActions);
