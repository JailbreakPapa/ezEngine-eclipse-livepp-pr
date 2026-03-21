#include <RazorPlugin/RazorPluginPCH.h>

#include <RazorPlugin/Resources/RazorDocumentResource.h>

#include <Foundation/IO/Stream.h>
#include <Foundation/Utilities/AssetFileHeader.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_ENUM(ezRazorScaleMode, 1)
  EZ_ENUM_CONSTANTS(ezRazorScaleMode::Fixed, ezRazorScaleMode::WithScreenSize)
EZ_END_STATIC_REFLECTED_ENUM;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorDocumentResource, 1, ezRTTIDefaultAllocator<ezRazorDocumentResource>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_RESOURCE_IMPLEMENT_COMMON_CODE(ezRazorDocumentResource);
// clang-format on

ezRazorDocumentResource::ezRazorDocumentResource()
  : ezResource(DoUpdate::OnAnyThread, 1)
{
}

ezResult ezRazorDocumentResourceDescriptor::Save(ezStreamWriter& inout_stream)
{
  const ezUInt32 uiVersion = 1;
  inout_stream << uiVersion;

  m_DependencyFile.StoreCurrentTimeStamp();
  EZ_SUCCEED_OR_RETURN(m_DependencyFile.WriteDependencyFile(inout_stream));

  inout_stream << m_ScaleMode;
  inout_stream << m_ReferenceResolution;

  inout_stream << m_CompiledData.GetCount();
  if (!m_CompiledData.IsEmpty())
  {
    EZ_SUCCEED_OR_RETURN(inout_stream.WriteBytes(m_CompiledData.GetData(), m_CompiledData.GetCount()));
  }

  return EZ_SUCCESS;
}

ezResult ezRazorDocumentResourceDescriptor::Load(ezStreamReader& inout_stream)
{
  ezUInt32 uiVersion = 0;
  inout_stream >> uiVersion;

  if (uiVersion != 1)
    return EZ_FAILURE;

  EZ_SUCCEED_OR_RETURN(m_DependencyFile.ReadDependencyFile(inout_stream));

  inout_stream >> m_ScaleMode;
  inout_stream >> m_ReferenceResolution;

  ezUInt32 uiDataSize = 0;
  inout_stream >> uiDataSize;
  m_CompiledData.SetCountUninitialized(uiDataSize);
  if (uiDataSize > 0)
  {
    inout_stream.ReadBytes(m_CompiledData.GetData(), uiDataSize);
  }

  return EZ_SUCCESS;
}

ezResourceLoadDesc ezRazorDocumentResource::UnloadData(Unload WhatToUnload)
{
  m_CompiledData.Clear();

  ezResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = ezResourceState::Unloaded;

  return res;
}

ezResourceLoadDesc ezRazorDocumentResource::UpdateContent(ezStreamReader* Stream)
{
  ezResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  if (Stream == nullptr)
  {
    res.m_State = ezResourceState::LoadedResourceMissing;
    return res;
  }

  // The ezResourceLoaderFromFile writes the absolute file path first.
  ezStringBuilder sAbsFilePath;
  (*Stream) >> sAbsFilePath;

  // Skip the asset file header.
  ezAssetFileHeader assetHeader;
  assetHeader.Read(*Stream).IgnoreResult();

  ezRazorDocumentResourceDescriptor desc;
  if (desc.Load(*Stream).Failed())
  {
    res.m_State = ezResourceState::LoadedResourceMissing;
    return res;
  }

  m_CompiledData = std::move(desc.m_CompiledData);
  m_ScaleMode = desc.m_ScaleMode;
  m_vReferenceResolution = desc.m_ReferenceResolution;

  res.m_State = ezResourceState::Loaded;
  return res;
}

void ezRazorDocumentResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(ezRazorDocumentResource) + m_CompiledData.GetCount();
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
}

EZ_RESOURCE_IMPLEMENT_CREATEABLE(ezRazorDocumentResource, ezRazorDocumentResourceDescriptor)
{
  m_CompiledData = std::move(descriptor.m_CompiledData);
  m_ScaleMode = descriptor.m_ScaleMode;
  m_vReferenceResolution = descriptor.m_ReferenceResolution;

  ezResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = ezResourceState::Loaded;
  return res;
}

EZ_STATICLINK_FILE(RazorPlugin, RazorPlugin_Resources_RazorDocumentResource);
