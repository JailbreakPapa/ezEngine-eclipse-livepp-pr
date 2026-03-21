#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorPluginAssets/HairStrandAsset/HairStrandAsset.h>
#include <EditorPluginAssets/HairStrandAsset/HairStrandAssetObjects.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <HairImporter/HairImporter.h>
#include <RendererCore/Meshes/HairStrandResource.h>
#include <ToolsFoundation/Application/ApplicationServices.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezHairStrandAssetDocument, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezHairStrandAssetDocument::ezHairStrandAssetDocument(ezStringView sDocumentPath)
  : ezSimpleAssetDocument<ezHairStrandAssetProperties>(sDocumentPath, ezAssetDocEngineConnection::None)
{
}

ezTransformStatus ezHairStrandAssetDocument::InternalTransformAsset(
  ezStreamWriter& stream, ezStringView sOutputTag, const ezPlatformProfile* pAssetProfile,
  const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags)
{
  const ezHairStrandAssetProperties* pProp = GetProperties();

  ezStringBuilder sAbsFilename = pProp->m_sSourceFile;

  if (sAbsFilename.IsEmpty())
  {
    return ezStatus("No source file specified for hair strand asset.");
  }

  if (!ezQtEditorApp::GetSingleton()->MakeDataDirectoryRelativePathAbsolute(sAbsFilename))
  {
    return ezStatus(ezFmt("Could not make path absolute: '{}'", sAbsFilename));
  }

  ezHairImporter::ImportOptions importOpt;
  importOpt.m_sSourceFile = sAbsFilename;
  importOpt.m_fGlobalWidthScale = pProp->m_fGlobalWidthScale;
  importOpt.m_fDefaultWidth = pProp->m_fDefaultWidth;
  importOpt.m_fTipWidthFraction = pProp->m_fTipWidthFraction;
  importOpt.m_uiMaxStrandsPerGroup = pProp->m_uiMaxStrandsPerGroup;
  importOpt.m_bGenerateUVs = pProp->m_bGenerateUVs;

  ezHairImporter::ImportResult importResult;
  if (ezHairImporter::Import(importOpt, importResult).Failed())
  {
    return ezStatus(ezFmt("Failed to import hair strands from '{}'", sAbsFilename));
  }

  importResult.m_Descriptor.Serialize(stream).IgnoreResult();

  return ezStatus(EZ_SUCCESS);
}

// --- Document Generator ---

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezHairStrandAssetDocumentGenerator, 1, ezRTTIDefaultAllocator<ezHairStrandAssetDocumentGenerator>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezHairStrandAssetDocumentGenerator::ezHairStrandAssetDocumentGenerator()
{
  AddSupportedFileType("abc");
}

ezHairStrandAssetDocumentGenerator::~ezHairStrandAssetDocumentGenerator() = default;

void ezHairStrandAssetDocumentGenerator::GetImportModes(
  ezStringView sAbsInputFile, ezDynamicArray<ezAssetDocumentGenerator::ImportMode>& out_modes) const
{
  {
    ezAssetDocumentGenerator::ImportMode& info = out_modes.ExpandAndGetRef();
    info.m_Priority = ezAssetDocGeneratorPriority::DefaultPriority;
    info.m_sName = "HairStrandImport";
    info.m_sIcon = ":/AssetIcons/HairStrands.svg";
  }
}

ezStatus ezHairStrandAssetDocumentGenerator::Generate(
  ezStringView sInputFileAbs, ezStringView sMode, ezDynamicArray<ezDocument*>& out_generatedDocuments)
{
  ezStringBuilder sOutFile = sInputFileAbs;
  sOutFile.ChangeFileExtension(GetDocumentExtension());

  if (ezOSFile::ExistsFile(sOutFile))
  {
    ezLog::Info("Skipping hair strand import, file has been imported before: '{}'", sOutFile);
    return ezStatus(EZ_SUCCESS);
  }

  auto pApp = ezQtEditorApp::GetSingleton();

  ezStringBuilder sInputFileRel = sInputFileAbs;
  pApp->MakePathDataDirectoryRelative(sInputFileRel);

  ezDocument* pDoc = pApp->CreateDocument(sOutFile, ezDocumentFlags::None);
  if (pDoc == nullptr)
    return ezStatus("Could not create target document");

  out_generatedDocuments.PushBack(pDoc);

  ezHairStrandAssetDocument* pAssetDoc = ezDynamicCast<ezHairStrandAssetDocument*>(pDoc);
  if (pAssetDoc != nullptr)
  {
    auto& accessor = pAssetDoc->GetPropertyObject()->GetTypeAccessor();
    accessor.SetValue("File", sInputFileRel.GetView());
  }

  ezLog::Success("Imported hair strand asset: '{}'", sOutFile);

  return ezStatus(EZ_SUCCESS);
}
