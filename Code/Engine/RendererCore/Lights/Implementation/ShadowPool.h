#pragma once

#include <RendererCore/Declarations.h>

class ezDirectionalLightComponent;
class ezPointLightComponent;
class ezSpotLightComponent;
class ezLightComponent;
class ezGALTextureHandle;
class ezGALBufferHandle;
class ezView;
struct ezRenderWorldExtractionEvent;
struct ezRenderWorldRenderEvent;

class EZ_RENDERERCORE_DLL ezShadowPool
{
public:
  static ezUInt32 AddDirectionalLight(const ezDirectionalLightComponent* pDirLight, const ezView* pReferenceView);
  static ezUInt32 AddPointLight(const ezPointLightComponent* pPointLight, float fScreenSpaceSize, const ezView* pReferenceView);
  static ezUInt32 AddSpotLight(const ezSpotLightComponent* pSpotLight, float fScreenSpaceSize, const ezView* pReferenceView);

  /// Adds point-light-style cube map shadows for an area light (rect or tube).
  /// Uses the light's position as shadow origin and the given effective range as the far plane.
  static ezUInt32 AddAreaLightAsPointLight(const ezLightComponent* pLight, float fEffectiveRange, float fScreenSpaceSize, const ezView* pReferenceView);

  static ezGALTextureHandle GetShadowAtlasTexture();
  static ezGALBufferHandle GetShadowDataBuffer();

  /// \brief All exclude tags on this white list are copied from the reference views to the shadow views.
  static void AddExcludeTagToWhiteList(const ezTag& tag);

private:
  EZ_MAKE_SUBSYSTEM_STARTUP_FRIEND(RendererCore, ShadowPool);

  static void OnEngineStartup();
  static void OnEngineShutdown();

  static void OnExtractionEvent(const ezRenderWorldExtractionEvent& e);
  static void OnRenderEvent(const ezRenderWorldRenderEvent& e);

  struct Data;
  static Data* s_pData;
};
