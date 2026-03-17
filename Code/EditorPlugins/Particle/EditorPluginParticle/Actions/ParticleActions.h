#pragma once

#include <EditorPluginParticle/EditorPluginParticleDLL.h>
#include <GuiFoundation/Action/BaseActions.h>
#include <GuiFoundation/GuiFoundationDLL.h>

class ezParticleEffectAssetDocument;
struct ezParticleEffectAssetEvent;

class ezParticleActions
{
public:
  static void RegisterActions();
  static void UnregisterActions();

  static void MapActions(ezStringView sMapping);

  static ezActionDescriptorHandle s_hCategory;
  static ezActionDescriptorHandle s_hPauseEffect;
  static ezActionDescriptorHandle s_hRestartEffect;
  static ezActionDescriptorHandle s_hAutoRestart;
  static ezActionDescriptorHandle s_hSimulationSpeedMenu;
  static ezActionDescriptorHandle s_hSimulationSpeed[10];
  static ezActionDescriptorHandle s_hRenderVisualizers;
  static ezActionDescriptorHandle s_hConvertToGPU;

  static void MapMenuBarActions(ezStringView sMapping);
};

class ezParticleAction : public ezButtonAction
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleAction, ezButtonAction);

public:
  enum class ActionType
  {
    PauseEffect,
    RestartEffect,
    AutoRestart,
    SimulationSpeed,
    RenderVisualizers,
    ConvertToGPU,
  };

  ezParticleAction(const ezActionContext& context, const char* szName, ActionType type, float fSimSpeed = 1.0f);
  ~ezParticleAction();

  virtual void Execute(const ezVariant& value) override;

private:
  void EffectEventHandler(const ezParticleEffectAssetEvent& e);
  void UpdateState();

  ezParticleEffectAssetDocument* m_pEffectDocument;
  ActionType m_Type;
  float m_fSimSpeed;
};

class ezParticleConvertToGPUAction : public ezButtonAction
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleConvertToGPUAction, ezButtonAction);

public:
  ezParticleConvertToGPUAction(const ezActionContext& context, const char* szName);

  virtual void Execute(const ezVariant& value) override;
};
