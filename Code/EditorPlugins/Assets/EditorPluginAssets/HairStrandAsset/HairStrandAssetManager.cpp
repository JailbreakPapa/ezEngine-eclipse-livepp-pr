#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorPluginAssets/HairStrandAsset/HairStrandAsset.h>
#include <EditorPluginAssets/HairStrandAsset/HairStrandAssetManager.h>
#include <EditorPluginAssets/HairStrandAsset/HairStrandAssetWindow.moc.h>
#include <ToolsFoundation/Assets/AssetFileExtensionWhitelist.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezHairStrandAssetDocumentManager, 1, ezRTTIDefaultAllocator<ezHairStrandAssetDocumentManager>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezHairStrandAssetDocumentManager::ezHairStrandAssetDocumentManager()
{
  ezDocumentManager::s_Events.AddEventHandler(ezMakeDelegate(&ezHairStrandAssetDocumentManager::OnDocumentManagerEvent, this));

  ezAssetFileExtensionWhitelist::AddAssetFileExtension("CompatibleAsset_Hair_Strands", "ezBinHairStrands");

  m_DocTypeDesc.m_sDocumentTypeName = "Hair Strands";
  m_DocTypeDesc.m_sFileExtension = "ezHairStrandAsset";
  m_DocTypeDesc.m_sIcon = ":/AssetIcons/HairStrands.svg";
  m_DocTypeDesc.m_sAssetCategory = "Rendering";
  m_DocTypeDesc.m_pDocumentType = ezGetStaticRTTI<ezHairStrandAssetDocument>();
  m_DocTypeDesc.m_pManager = this;
  m_DocTypeDesc.m_CompatibleTypes.PushBack("CompatibleAsset_Hair_Strands");

  m_DocTypeDesc.m_sResourceFileExtension = "ezBinHairStrands";
  m_DocTypeDesc.m_AssetDocumentFlags = ezAssetDocumentFlags::AutoTransformOnSave;
}

ezHairStrandAssetDocumentManager::~ezHairStrandAssetDocumentManager()
{
  ezDocumentManager::s_Events.RemoveEventHandler(ezMakeDelegate(&ezHairStrandAssetDocumentManager::OnDocumentManagerEvent, this));
}

void ezHairStrandAssetDocumentManager::OnDocumentManagerEvent(const ezDocumentManager::Event& e)
{
  switch (e.m_Type)
  {
    case ezDocumentManager::Event::Type::DocumentWindowRequested:
    {
      if (e.m_pDocument->GetDynamicRTTI() == ezGetStaticRTTI<ezHairStrandAssetDocument>())
      {
        new ezQtHairStrandAssetDocumentWindow(static_cast<ezHairStrandAssetDocument*>(e.m_pDocument)); // NOLINT
      }
    }
    break;
    default:
      break;
  }
}

void ezHairStrandAssetDocumentManager::InternalCreateDocument(
  ezStringView sDocumentTypeName, ezStringView sPath, bool bCreateNewDocument, ezDocument*& out_pDocument, const ezDocumentObject* pOpenContext)
{
  out_pDocument = new ezHairStrandAssetDocument(sPath);
}

void ezHairStrandAssetDocumentManager::InternalGetSupportedDocumentTypes(ezDynamicArray<const ezDocumentTypeDescriptor*>& inout_DocumentTypes) const
{
  inout_DocumentTypes.PushBack(&m_DocTypeDesc);
}
