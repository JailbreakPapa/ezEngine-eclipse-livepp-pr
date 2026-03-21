#pragma once

#include <RazorPlugin/RazorPluginDLL.h>

#include <Core/ResourceManager/ResourceHandle.h>
#include <RendererCore/Pipeline/Renderer.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>

using ezShaderResourceHandle = ezTypedResourceHandle<class ezShaderResource>;

/// GPU renderer that processes ezRazorRenderData batches.
///
/// Binds the Razor UI shader, uploads vertex/index data, and issues draw calls
/// with per-draw scissor rects and shader permutation switching (solid, textured,
/// SDF text, alpha text).
class EZ_RAZORPLUGIN_DLL ezRazorRenderer : public ezRenderer
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorRenderer, ezRenderer);
  EZ_DISALLOW_COPY_AND_ASSIGN(ezRazorRenderer);

public:
  ezRazorRenderer();
  ~ezRazorRenderer();

  virtual void GetSupportedRenderDataTypes(ezDynamicArray<const ezRTTI*>& out_types) const override;
  virtual void RenderBatch(const ezRenderViewContext& renderViewContext, const ezRenderPipelinePass* pPass, const ezRenderDataBatch& batch) const override;

private:
  ezShaderResourceHandle m_hShader;
  ezConstantBufferStorageHandle m_hConstantBuffer;
};
