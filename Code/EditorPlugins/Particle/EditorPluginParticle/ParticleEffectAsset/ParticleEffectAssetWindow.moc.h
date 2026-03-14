#pragma once

#include <EditorEngineProcessFramework/EngineProcess/ViewRenderSettings.h>
#include <EditorFramework/DocumentWindow/EngineDocumentWindow.moc.h>
#include <EditorPluginParticle/ParticleEffectAsset/ParticleEffectAsset.h>
#include <Foundation/Basics.h>
#include <GuiFoundation/DocumentWindow/DocumentWindow.moc.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class ezQtOrbitCamViewWidget;
class ezParticleEffectAssetDocument;
class ezQtPropertyGridWidget;
class ezQtParticleEffectGraphScene;
class ezQtVisualGraphView;

class ezQtParticleEffectAssetDocumentWindow : public ezQtEngineDocumentWindow
{
  Q_OBJECT

public:
  ezQtParticleEffectAssetDocumentWindow(ezAssetDocument* pDocument);
  ~ezQtParticleEffectAssetDocumentWindow();

  ezParticleEffectAssetDocument* GetParticleDocument();

protected:
  virtual void InternalRedraw() override;

private:
  void SendRedrawMsg();
  void RestoreResource();
  void SendLiveResourcePreview();
  void PropertyEventHandler(const ezDocumentObjectPropertyEvent& e);
  void StructureEventHandler(const ezDocumentObjectStructureEvent& e);
  void ParticleEventHandler(const ezParticleEffectAssetEvent& e);

  ezParticleEffectAssetDocument* m_pAssetDoc;

  ezEngineViewConfig m_ViewConfig;
  ezQtOrbitCamViewWidget* m_pViewWidget;

  ezQtParticleEffectGraphScene* m_pScene = nullptr;
  ezQtVisualGraphView* m_pView = nullptr;

  bool m_bDoLiveResourceUpdate = true;
};
