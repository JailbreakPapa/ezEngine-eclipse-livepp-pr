#pragma once

#include <EditorEngineProcessFramework/EngineProcess/ViewRenderSettings.h>
#include <EditorFramework/Assets/AssetDocument.h>
#include <Foundation/Communication/Event.h>
#include <ParticlePlugin/Effect/ParticleEffectDescriptor.h>

class ezParticleEffectAssetDocument;
struct ezPropertyMetaStateEvent;

struct ezParticleEffectAssetEvent
{
  enum Type
  {
    RestartEffect,
    AutoRestartChanged,
    SimulationSpeedChanged,
    RenderVisualizersChanged,
  };

  ezParticleEffectAssetDocument* m_pDocument;
  Type m_Type;
};

class ezParticleEffectAssetDocument : public ezAssetDocument
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleEffectAssetDocument, ezAssetDocument);

public:
  ezParticleEffectAssetDocument(ezStringView sDocumentPath);
  ~ezParticleEffectAssetDocument();

  static void PropertyMetaStateEventHandler(ezPropertyMetaStateEvent& e);

  void WriteResource(ezStreamWriter& inout_stream) const;

  void TriggerRestartEffect();

  ezEvent<const ezParticleEffectAssetEvent&> m_Events;

  void SetAutoRestart(bool bEnable);
  bool GetAutoRestart() const { return m_bAutoRestart; }

  void SetSimulationPaused(bool bPaused);
  bool GetSimulationPaused() const { return m_bSimulationPaused; }

  void SetSimulationSpeed(float fSpeed);
  float GetSimulationSpeed() const { return m_fSimulationSpeed; }

  bool GetRenderVisualizers() const { return m_bRenderVisualizers; }
  void SetRenderVisualizers(bool b);

  virtual ezResult ComputeObjectTransformation(const ezDocumentObject* pObject, ezTransform& out_result) const override;

protected:
  virtual void InitializeAfterLoading(bool bFirstTimeCreation) override;
  virtual void UpdateAssetDocumentInfo(ezAssetDocumentInfo* pInfo) const override;
  virtual ezTransformStatus InternalTransformAsset(ezStreamWriter& stream, ezStringView sOutputTag, const ezPlatformProfile* pAssetProfile,
    const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags) override;
  virtual ezTransformStatus InternalCreateThumbnail(const ThumbnailInfo& ThumbnailInfo) override;

  virtual void InternalGetMetaDataHash(const ezDocumentObject* pObject, ezUInt64& inout_uiHash) const override;
  virtual void AttachMetaDataBeforeSaving(ezAbstractObjectGraph& graph) const override;
  virtual void RestoreMetaDataAfterLoading(const ezAbstractObjectGraph& graph, bool bUndoable) override;

  virtual void GetSupportedMimeTypesForPasting(ezHybridArray<ezString, 4>& out_MimeTypes) const override;
  virtual bool CopySelectedObjects(ezAbstractObjectGraph& out_objectGraph, ezStringBuilder& out_MimeType) const override;
  virtual bool Paste(const ezArrayPtr<PasteInfo>& info, const ezAbstractObjectGraph& objectGraph, bool bAllowPickedPosition, ezStringView sMimeType) override;

private:
  void BuildDescriptorFromGraph(ezParticleEffectDescriptor& out_desc) const;
  const ezDocumentObject* FindEffectNode() const;
  void CollectConnectedNodes(const ezDocumentObject* pNode, ezStringView sPinName, ezDynamicArray<const ezDocumentObject*>& out_nodes) const;

  ezEngineViewLightSettings m_LightSettings;

  bool m_bSimulationPaused = false;
  bool m_bAutoRestart = true;
  bool m_bRenderVisualizers = false;
  float m_fSimulationSpeed = 1.0f;
};
