#pragma once

#include <EditorPluginRazor/EditorPluginRazorDLL.h>

#include <ToolsFoundation/Reflection/ReflectedType.h>

class EZ_EDITORPLUGINRAZOR_DLL ezRazorDocumentAssetProperties : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorDocumentAssetProperties, ezReflectedClass);

public:
  ezRazorDocumentAssetProperties() = default;

  ezString m_sRazorFile;
  ezString m_sStyleSheetFile;

  ezVec2U32 m_ReferenceResolution = ezVec2U32(1920, 1080);
};
