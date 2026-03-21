#include <EditorPluginRazor/EditorPluginRazorPCH.h>

#include <EditorPluginRazor/RazorAsset/RazorAssetObjects.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorDocumentAssetProperties, 1, ezRTTIDefaultAllocator<ezRazorDocumentAssetProperties>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("RazorFile", m_sRazorFile)->AddAttributes(new ezFileBrowserAttribute("Select Razor Document", "*.razor.xml")),
    EZ_MEMBER_PROPERTY("StyleSheetFile", m_sStyleSheetFile)->AddAttributes(new ezFileBrowserAttribute("Select Style Sheet", "*.razor.css")),
    EZ_MEMBER_PROPERTY("ReferenceResolution", m_ReferenceResolution)->AddAttributes(new ezDefaultValueAttribute(ezVec2U32(1920, 1080))),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

EZ_STATICLINK_FILE(EditorPluginRazor, EditorPluginRazor_RazorAsset_RazorAssetObjects);
