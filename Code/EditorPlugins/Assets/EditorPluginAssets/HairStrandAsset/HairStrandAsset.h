#pragma once

#include <EditorFramework/Assets/AssetDocumentGenerator.h>
#include <EditorFramework/Assets/SimpleAssetDocument.h>
#include <EditorPluginAssets/HairStrandAsset/HairStrandAssetObjects.h>

class ezHairStrandAssetDocument : public ezSimpleAssetDocument<ezHairStrandAssetProperties>
{
  EZ_ADD_DYNAMIC_REFLECTION(ezHairStrandAssetDocument, ezSimpleAssetDocument<ezHairStrandAssetProperties>);

public:
  ezHairStrandAssetDocument(ezStringView sDocumentPath);

protected:
  virtual ezTransformStatus InternalTransformAsset(ezStreamWriter& stream, ezStringView sOutputTag, const ezPlatformProfile* pAssetProfile,
    const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags) override;
};

class ezHairStrandAssetDocumentGenerator : public ezAssetDocumentGenerator
{
  EZ_ADD_DYNAMIC_REFLECTION(ezHairStrandAssetDocumentGenerator, ezAssetDocumentGenerator);

public:
  ezHairStrandAssetDocumentGenerator();
  ~ezHairStrandAssetDocumentGenerator();

  virtual void GetImportModes(ezStringView sAbsInputFile, ezDynamicArray<ezAssetDocumentGenerator::ImportMode>& out_modes) const override;
  virtual ezStringView GetDocumentExtension() const override { return "ezHairStrandAsset"; }
  virtual ezStringView GetGeneratorGroup() const override { return "Hair"; }
  virtual ezStatus Generate(ezStringView sInputFileAbs, ezStringView sMode, ezDynamicArray<ezDocument*>& out_generatedDocuments) override;
};
