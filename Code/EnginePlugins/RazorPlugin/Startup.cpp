#include <RazorPlugin/RazorPluginPCH.h>

#include <Foundation/Configuration/Plugin.h>
#include <Foundation/Configuration/Startup.h>
#include <RazorPlugin/Resources/RazorDocumentResource.h>
#include <RazorPlugin/RazorSystem.h>

static ezRazorDocumentResourceLoader s_RazorDocumentResourceLoader;

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(Razor, RazorPlugin)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
    ezResourceManager::SetResourceTypeLoader<ezRazorDocumentResource>(&s_RazorDocumentResourceLoader);

    ezResourceManager::RegisterResourceForAssetType("Razor Document", ezGetStaticRTTI<ezRazorDocumentResource>());

    {
      ezRazorDocumentResourceDescriptor desc;
      ezRazorDocumentResourceHandle hResource = ezResourceManager::CreateResource<ezRazorDocumentResource>("RazorDocumentMissing", std::move(desc), "Fallback for missing Razor document resource");
      ezResourceManager::SetResourceTypeMissingFallback<ezRazorDocumentResource>(hResource);
    }

    if (ezRazorSystem::GetSingleton() == nullptr)
    {
      EZ_DEFAULT_NEW(ezRazorSystem);
    }
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    if (ezRazorSystem* pSystem = ezRazorSystem::GetSingleton())
    {
      EZ_DEFAULT_DELETE(pSystem);
    }

    ezResourceManager::SetResourceTypeLoader<ezRazorDocumentResource>(nullptr);

    ezRazorDocumentResource::CleanupDynamicPluginReferences();
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

EZ_STATICLINK_FILE(RazorPlugin, RazorPlugin_Startup);
