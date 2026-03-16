#pragma once

#include <EditorEngineProcessFramework/EngineProcess/EngineProcessDocumentContext.h>
#include <EnginePluginAssets/EnginePluginAssetsDLL.h>

class EZ_ENGINEPLUGINASSETS_DLL ezAnimationGraphContext : public ezEngineProcessDocumentContext
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAnimationGraphContext, ezEngineProcessDocumentContext);

public:
  ezAnimationGraphContext();

  virtual void HandleMessage(const ezEditorEngineDocumentMsg* pMsg) override;

  bool m_bDisplayGrid = true;

protected:
  virtual void OnInitialize() override;

  virtual ezEngineProcessViewContext* CreateViewContext() override;
  virtual void DestroyViewContext(ezEngineProcessViewContext* pContext) override;
  virtual bool UpdateThumbnailViewContext(ezEngineProcessViewContext* pThumbnailViewContext) override;

private:
  void QuerySelectionBBox(const ezEditorEngineDocumentMsg* pMsg);
  void SendDebugState(const ezEditorEngineDocumentMsg* pMsg);
  void ApplyBlackboardParams();

  ezGameObject* m_pGameObject = nullptr;
  ezString m_sAnimatedMeshToUse;
  ezString m_sBlackboardParamsPayload; ///< Cached "name|type;..." for deferred registration
  ezComponentHandle m_hAnimMeshComponent;
  ezComponentHandle m_hSkeletonComponent;
  ezComponentHandle m_hBlackboardComponent;  ///< Blackboard for animation parameters
  ezComponentHandle m_hAnimControllerComponent;
  ezComponentHandle m_hSimpleAnimComponent; ///< Used for isolated clip playback
  float m_fSimulationSpeed = 1.0f;
  bool m_bPlayingIsolatedClip = false;
};
