#pragma once

#include <RazorPlugin/RazorPluginDLL.h>

#include <Core/ResourceManager/Resource.h>
#include <Foundation/IO/DependencyFile.h>

/// Scale mode for a Razor UI document when displayed on screen.
struct EZ_RAZORPLUGIN_DLL ezRazorScaleMode
{
  using StorageType = ezUInt8;

  enum Enum
  {
    Fixed,
    WithScreenSize,

    Default = WithScreenSize
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_RAZORPLUGIN_DLL, ezRazorScaleMode);

/// Descriptor written by the asset pipeline and loaded at runtime.
struct EZ_RAZORPLUGIN_DLL ezRazorDocumentResourceDescriptor
{
  ezResult Save(ezStreamWriter& inout_stream);
  ezResult Load(ezStreamReader& inout_stream);

  ezDependencyFile m_DependencyFile;

  /// Compiled Razor document blob (DOM + stylesheet + script refs).
  ezDataBuffer m_CompiledData;

  ezEnum<ezRazorScaleMode> m_ScaleMode;
  ezVec2U32 m_ReferenceResolution;
};

using ezRazorDocumentResourceHandle = ezTypedResourceHandle<class ezRazorDocumentResource>;

/// Runtime resource representing a compiled Razor UI document.
class EZ_RAZORPLUGIN_DLL ezRazorDocumentResource : public ezResource
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorDocumentResource, ezResource);
  EZ_RESOURCE_DECLARE_COMMON_CODE(ezRazorDocumentResource);
  EZ_RESOURCE_DECLARE_CREATEABLE(ezRazorDocumentResource, ezRazorDocumentResourceDescriptor);

public:
  ezRazorDocumentResource();

  const ezDataBuffer& GetCompiledData() const { return m_CompiledData; }
  const ezEnum<ezRazorScaleMode>& GetScaleMode() const { return m_ScaleMode; }
  const ezVec2U32& GetReferenceResolution() const { return m_vReferenceResolution; }

private:
  virtual ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual ezResourceLoadDesc UpdateContent(ezStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  ezDataBuffer m_CompiledData;
  ezEnum<ezRazorScaleMode> m_ScaleMode;
  ezVec2U32 m_vReferenceResolution = ezVec2U32::MakeZero();
};

class ezRazorDocumentResourceLoader : public ezResourceLoaderFromFile
{
};
