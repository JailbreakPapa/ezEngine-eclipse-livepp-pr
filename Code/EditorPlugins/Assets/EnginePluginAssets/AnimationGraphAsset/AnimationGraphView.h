#pragma once

#include <EditorEngineProcessFramework/EngineProcess/EngineProcessViewContext.h>

class ezAnimationGraphContext;

class ezAnimationGraphViewContext : public ezEngineProcessViewContext
{
public:
  ezAnimationGraphViewContext(ezAnimationGraphContext* pContext);
  ~ezAnimationGraphViewContext();

  bool UpdateThumbnailCamera(const ezBoundingBoxSphere& bounds);

protected:
  virtual ezViewHandle CreateView() override;
  virtual void SetCamera(const ezViewRedrawMsgToEngine* pMsg) override;

  ezAnimationGraphContext* m_pContext = nullptr;
};
