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

  ezGameObject* m_pGameObject = nullptr;
};
