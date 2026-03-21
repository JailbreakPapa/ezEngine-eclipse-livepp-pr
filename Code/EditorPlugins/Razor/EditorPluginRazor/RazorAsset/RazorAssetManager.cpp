#include <EditorPluginRazor/EditorPluginRazorPCH.h>

#include <EditorPluginRazor/RazorAsset/RazorAsset.h>
#include <EditorPluginRazor/RazorAsset/RazorAssetManager.h>
#include <EditorPluginRazor/RazorAsset/RazorAssetWindow.moc.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorDocumentAssetManager, 1, ezRTTIDefaultAllocator<ezRazorDocumentAssetManager>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezRazorDocumentAssetManager::ezRazorDocumentAssetManager()
{
  ezDocumentManager::s_Events.AddEventHandler(ezMakeDelegate(&ezRazorDocumentAssetManager::OnDocumentManagerEvent, this));

  m_DocTypeDesc.m_sDocumentTypeName = "Razor Document";
  m_DocTypeDesc.m_sFileExtension = "ezRazorDocumentAsset";
  m_DocTypeDesc.m_sIcon = ":/AssetIcons/Razor.svg";
  m_DocTypeDesc.m_sAssetCategory = "Input";
  m_DocTypeDesc.m_pDocumentType = ezGetStaticRTTI<ezRazorDocumentAsset>();
  m_DocTypeDesc.m_pManager = this;
  m_DocTypeDesc.m_CompatibleTypes.PushBack("CompatibleAsset_Razor_Document");

  m_DocTypeDesc.m_sResourceFileExtension = "ezBinRazor";
  m_DocTypeDesc.m_AssetDocumentFlags = ezAssetDocumentFlags::None; // TODO: implement SupportsThumbnail
}

ezRazorDocumentAssetManager::~ezRazorDocumentAssetManager()
{
  ezDocumentManager::s_Events.RemoveEventHandler(ezMakeDelegate(&ezRazorDocumentAssetManager::OnDocumentManagerEvent, this));
}

void ezRazorDocumentAssetManager::OnDocumentManagerEvent(const ezDocumentManager::Event& e)
{
  switch (e.m_Type)
  {
    case ezDocumentManager::Event::Type::DocumentWindowRequested:
    {
      if (e.m_pDocument->GetDynamicRTTI() == ezGetStaticRTTI<ezRazorDocumentAsset>())
      {
        new ezQtRazorAssetDocumentWindow(static_cast<ezRazorDocumentAsset*>(e.m_pDocument));
      }
    }
    break;

    default:
      break;
  }
}

void ezRazorDocumentAssetManager::InternalCreateDocument(
  ezStringView sDocumentTypeName, ezStringView sPath, bool bCreateNewDocument, ezDocument*& out_pDocument, const ezDocumentObject* pOpenContext)
{
  out_pDocument = new ezRazorDocumentAsset(sPath);
}

void ezRazorDocumentAssetManager::InternalGetSupportedDocumentTypes(ezDynamicArray<const ezDocumentTypeDescriptor*>& inout_DocumentTypes) const
{
  inout_DocumentTypes.PushBack(&m_DocTypeDesc);
}

EZ_STATICLINK_FILE(EditorPluginRazor, EditorPluginRazor_RazorAsset_RazorAssetManager);
