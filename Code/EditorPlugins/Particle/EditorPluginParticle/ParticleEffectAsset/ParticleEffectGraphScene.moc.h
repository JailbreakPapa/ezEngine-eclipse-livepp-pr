#pragma once

#include <Foundation/Basics.h>
#include <GuiFoundation/VisualGraph/Scene.moc.h>

class ezQtParticleEffectGraphScene : public ezQtVisualGraphScene
{
  Q_OBJECT

public:
  ezQtParticleEffectGraphScene(QObject* pParent = nullptr);
  ~ezQtParticleEffectGraphScene();
};
