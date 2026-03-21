#include <EditorPluginRazor/EditorPluginRazorPCH.h>

#include <EditorPluginRazor/RazorAsset/RazorAsset.h>

#include <Foundation/Utilities/AssetFileHeader.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/MemoryStream.h>
#include <EditorFramework/Assets/AssetCurator.h>
#include <RazorPlugin/Resources/RazorDocumentResource.h>
#include <RazorCore/DOM/RazorDocument.h>
#include <RazorCore/Parser/RazorXmlParser.h>
#include <RazorCore/Parser/RazorCssParser.h>
#include <RazorCore/Style/RazorStyleSheet.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorDocumentAsset, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezRazorDocumentAsset::ezRazorDocumentAsset(ezStringView sDocumentPath)
  : ezSimpleAssetDocument<ezRazorDocumentAssetProperties>(sDocumentPath, ezAssetDocEngineConnection::Simple)
{
}

ezTransformStatus ezRazorDocumentAsset::InternalTransformAsset(ezStreamWriter& stream, ezStringView sOutputTag, const ezPlatformProfile* pAssetProfile,
  const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags)
{
  const ezRazorDocumentAssetProperties* pProp = GetProperties();

  ezRazorDocumentResourceDescriptor desc;
  desc.m_ReferenceResolution = pProp->m_ReferenceResolution;

  // Parse XML source file if specified
  ezRazorDocument document;
  if (!pProp->m_sRazorFile.IsEmpty())
  {
    ezFileReader reader;
    if (reader.Open(pProp->m_sRazorFile).Succeeded())
    {
      ezStringBuilder xmlContent;
      xmlContent.ReadAll(reader);

      if (ezRazorXmlParser::Parse(xmlContent, document).Failed())
      {
        return ezStatus(ezFmt("Failed to parse Razor XML file: {}", pProp->m_sRazorFile));
      }
    }
    else
    {
      return ezStatus(ezFmt("Failed to open Razor XML file: {}", pProp->m_sRazorFile));
    }
  }

  // Parse CSS source file if specified
  ezRazorStyleSheet stylesheet;
  if (!pProp->m_sStyleSheetFile.IsEmpty())
  {
    ezFileReader reader;
    if (reader.Open(pProp->m_sStyleSheetFile).Succeeded())
    {
      ezStringBuilder cssContent;
      cssContent.ReadAll(reader);

      if (ezRazorCssParser::Parse(cssContent, stylesheet).Failed())
      {
        return ezStatus(ezFmt("Failed to parse CSS file: {}", pProp->m_sStyleSheetFile));
      }
    }
    else
    {
      return ezStatus(ezFmt("Failed to open CSS file: {}", pProp->m_sStyleSheetFile));
    }
  }

  // Serialize compiled data to buffer
  {
    ezMemoryStreamContainerWrapperStorage<ezDataBuffer> storage(&desc.m_CompiledData);
    ezMemoryStreamWriter writer(&storage);

    const ezUInt32 uiVersion = 1;
    writer << uiVersion;

    // Serialize the DOM
    if (document.Serialize(writer).Failed())
    {
      return ezStatus("Failed to serialize document");
    }

    // Serialize the stylesheet
    if (stylesheet.Serialize(writer).Failed())
    {
      return ezStatus("Failed to serialize stylesheet");
    }
  }

  EZ_SUCCEED_OR_RETURN(desc.Save(stream));

  return ezStatus(EZ_SUCCESS);
}

void ezRazorDocumentAsset::UpdateAssetDocumentInfo(ezAssetDocumentInfo* pInfo) const
{
  SUPER::UpdateAssetDocumentInfo(pInfo);

  const ezRazorDocumentAssetProperties* pProp = GetProperties();

  if (!pProp->m_sRazorFile.IsEmpty())
  {
    pInfo->m_TransformDependencies.Insert(pProp->m_sRazorFile);
  }

  if (!pProp->m_sStyleSheetFile.IsEmpty())
  {
    pInfo->m_TransformDependencies.Insert(pProp->m_sStyleSheetFile);
  }
}

EZ_STATICLINK_FILE(EditorPluginRazor, EditorPluginRazor_RazorAsset_RazorAsset);
