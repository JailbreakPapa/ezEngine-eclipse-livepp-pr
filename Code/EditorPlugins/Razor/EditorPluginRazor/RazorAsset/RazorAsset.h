#pragma once

#include <EditorFramework/Assets/SimpleAssetDocument.h>
#include <EditorPluginRazor/RazorAsset/RazorAssetObjects.h>

/// Asset document for Razor UI files.
///
/// Compiles XML + CSS source into a binary Razor document blob for runtime use.
class ezRazorDocumentAsset : public ezSimpleAssetDocument<ezRazorDocumentAssetProperties>
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorDocumentAsset, ezSimpleAssetDocument<ezRazorDocumentAssetProperties>);

public:
  ezRazorDocumentAsset(ezStringView sDocumentPath);

protected:
  virtual ezTransformStatus InternalTransformAsset(ezStreamWriter& stream, ezStringView sOutputTag, const ezPlatformProfile* pAssetProfile,
    const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags) override;

  virtual void UpdateAssetDocumentInfo(ezAssetDocumentInfo* pInfo) const override;
};
