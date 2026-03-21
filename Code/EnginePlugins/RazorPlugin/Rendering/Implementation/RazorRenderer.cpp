#include <RazorPlugin/RazorPluginPCH.h>

#include <RazorPlugin/Rendering/RazorRenderData.h>
#include <RazorPlugin/Rendering/RazorRenderer.h>

#include <RendererCore/Pipeline/RenderDataBatch.h>
#include <RendererCore/Pipeline/ViewData.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererFoundation/CommandEncoder/CommandEncoder.h>
#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Resources/Buffer.h>
#include <RendererFoundation/Resources/Texture.h>
#include <cstddef>

#include <Shaders/RazorConstants.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorRenderData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorRenderer, 1, ezRTTIDefaultAllocator<ezRazorRenderer>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezUInt32 ezRazorVertex::PackColor(const ezColor& color)
{
  ezColorLinearUB packed = color;
  return static_cast<ezUInt32>(packed.r) |
         (static_cast<ezUInt32>(packed.g) << 8) |
         (static_cast<ezUInt32>(packed.b) << 16) |
         (static_cast<ezUInt32>(packed.a) << 24);
}

ezRazorRenderer::ezRazorRenderer()
{
  m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/RazorUI.ezShader");
  m_hConstantBuffer = ezRenderContext::CreateConstantBufferStorage<ezRazorConstants>();
}

ezRazorRenderer::~ezRazorRenderer()
{
  ezRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
}

void ezRazorRenderer::GetSupportedRenderDataTypes(ezDynamicArray<const ezRTTI*>& out_types) const
{
  out_types.PushBack(ezGetStaticRTTI<ezRazorRenderData>());
}

void ezRazorRenderer::RenderBatch(const ezRenderViewContext& renderViewContext, const ezRenderPipelinePass* pPass, const ezRenderDataBatch& batch) const
{
  EZ_IGNORE_UNUSED(pPass);

  auto RenderSingleBatch = [this](ezRenderContext* pRenderContext, const ezRazorRenderData* pRenderData, float fViewportW, float fViewportH)
  {
    pRenderContext->BindShader(m_hShader);

    ezRazorConstants* pConstants = pRenderContext->GetConstantBufferData<ezRazorConstants>(m_hConstantBuffer);
    pConstants->RazorViewportSize = ezVec2(fViewportW, fViewportH);
    pConstants->RazorAtlasSize = ezVec2(1024, 1024); // TODO: Updated per-texture when text is rendered.

    ezBindGroupBuilder& bindGroup = pRenderContext->GetBindGroup();
    bindGroup.BindBuffer("ezRazorConstants", m_hConstantBuffer);

    // Create transient vertex and index buffers.
    ezGALBufferCreationDescription vbDesc;
    vbDesc.m_uiStructSize = sizeof(ezRazorVertex);
    vbDesc.m_uiTotalSize = pRenderData->m_Vertices.GetCount() * sizeof(ezRazorVertex);
    vbDesc.m_BufferFlags = ezGALBufferUsageFlags::VertexBuffer;

    ezGALBufferHandle hVB = ezGALDevice::GetDefaultDevice()->CreateBuffer(
      vbDesc, pRenderData->m_Vertices.GetByteArrayPtr());

    ezGALBufferCreationDescription ibDesc;
    ibDesc.m_uiStructSize = sizeof(ezUInt16);
    ibDesc.m_uiTotalSize = pRenderData->m_Indices.GetCount() * sizeof(ezUInt16);
    ibDesc.m_BufferFlags = ezGALBufferUsageFlags::IndexBuffer;

    ezGALBufferHandle hIB = ezGALDevice::GetDefaultDevice()->CreateBuffer(
      ibDesc, pRenderData->m_Indices.GetByteArrayPtr());

    ezGALVertexAttribute vertexAttributes[] = {
      ezGALVertexAttribute(ezGALVertexAttributeSemantic::Position, ezGALResourceFormat::RGFloat, offsetof(ezRazorVertex, m_fPosX), 0),
      ezGALVertexAttribute(ezGALVertexAttributeSemantic::TexCoord0, ezGALResourceFormat::RGFloat, offsetof(ezRazorVertex, m_fTexU), 0),
      ezGALVertexAttribute(ezGALVertexAttributeSemantic::Color0, ezGALResourceFormat::RGBAUByteNormalized, offsetof(ezRazorVertex, m_uiColor), 0)};

    ezGALBufferHandle vertexBuffers[] = {hVB};
    const ezUInt32 uiTotalPrimitives = pRenderData->m_Indices.GetCount() / 3;
    pRenderContext->BindMeshBuffer(
      ezArrayPtr<const ezGALBufferHandle>(vertexBuffers),
      hIB,
      ezArrayPtr<const ezGALVertexAttribute>(vertexAttributes),
      ezGALPrimitiveTopology::Triangles,
      uiTotalPrimitives);

    // Process draw calls.
    for (const ezRazorDrawCall& dc : pRenderData->m_DrawCalls)
    {
      switch (dc.m_Mode)
      {
        case ezRazorDrawCall::Mode::Solid:
          pRenderContext->SetShaderPermutationVariable("RAZOR_MODE", ezTempHashedString("RAZOR_MODE_SOLID"));
          break;
        case ezRazorDrawCall::Mode::Textured:
          pRenderContext->SetShaderPermutationVariable("RAZOR_MODE", ezTempHashedString("RAZOR_MODE_TEXTURED"));
          if (!dc.m_hTexture.IsInvalidated())
            bindGroup.BindTexture("BaseTexture", dc.m_hTexture);
          break;
        case ezRazorDrawCall::Mode::SDFText:
          pRenderContext->SetShaderPermutationVariable("RAZOR_MODE", ezTempHashedString("RAZOR_MODE_SDF_TEXT"));
          if (!dc.m_hTexture.IsInvalidated())
            bindGroup.BindTexture("BaseTexture", dc.m_hTexture);
          break;
        case ezRazorDrawCall::Mode::AlphaText:
          pRenderContext->SetShaderPermutationVariable("RAZOR_MODE", ezTempHashedString("RAZOR_MODE_ALPHA_TEXT"));
          if (!dc.m_hTexture.IsInvalidated())
            bindGroup.BindTexture("BaseTexture", dc.m_hTexture);
          break;
      }

      if (dc.m_bHasScissor)
      {
        const ezRectU32 scissorRect(
          static_cast<ezUInt32>(ezMath::Max(0.0f, dc.m_ScissorRect.x)),
          static_cast<ezUInt32>(ezMath::Max(0.0f, dc.m_ScissorRect.y)),
          static_cast<ezUInt32>(ezMath::Max(0.0f, dc.m_ScissorRect.width)),
          static_cast<ezUInt32>(ezMath::Max(0.0f, dc.m_ScissorRect.height)));
        pRenderContext->GetCommandEncoder()->SetScissorRect(scissorRect);
      }

      const ezUInt32 uiPrimitiveCount = dc.m_uiIndexCount / 3;
      const ezUInt32 uiFirstPrimitive = dc.m_uiFirstIndex / 3;
      pRenderContext->DrawMeshBuffer(uiPrimitiveCount, uiFirstPrimitive).IgnoreResult();
    }

    ezGALDevice::GetDefaultDevice()->DestroyBuffer(hVB);
    ezGALDevice::GetDefaultDevice()->DestroyBuffer(hIB);
  };

  for (auto it = batch.GetIterator<ezRazorRenderData>(); it.IsValid(); ++it)
  {
    const ezRazorRenderData* pRenderData = it;

    if (pRenderData->m_Vertices.IsEmpty() || pRenderData->m_DrawCalls.IsEmpty())
      continue;

    // World-space 3D canvas path: render this batch into a dedicated target texture.
    if (!pRenderData->m_hTargetTexture.IsInvalidated())
    {
      ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
      const ezGALTexture* pTargetTexture = pDevice->GetTexture(pRenderData->m_hTargetTexture);
      if (pTargetTexture == nullptr)
        continue;

      const auto& textureDesc = pTargetTexture->GetDescription();

      ezGALCommandEncoder* pCommandEncoder = pDevice->BeginCommands("RazorUI Offscreen");
      ezRenderContext* pOffscreenContext = ezRenderContext::CreateInstance(pCommandEncoder);

      ezGALRenderingSetup renderingSetup;
      renderingSetup.SetColorTarget(0, pDevice->GetDefaultRenderTargetView(pRenderData->m_hTargetTexture));
      renderingSetup.SetClearColor(0, ezColor::MakeZero());

      const ezRectFloat viewport(static_cast<float>(textureDesc.m_uiWidth), static_cast<float>(textureDesc.m_uiHeight));
      pOffscreenContext->BeginRendering(renderingSetup, viewport, "RazorUI Offscreen Pass", false);

      RenderSingleBatch(pOffscreenContext, pRenderData, viewport.width, viewport.height);

      pOffscreenContext->EndRendering();

      if (textureDesc.m_uiMipLevelCount > 1 && textureDesc.m_TextureFlags.IsSet(ezGALTextureUsageFlags::DynamicMipGeneration))
      {
        pCommandEncoder->GenerateMipMaps(pRenderData->m_hTargetTexture, ezGALTextureRange::MakeFromMipRange());
      }

      ezRenderContext::DestroyInstance(pOffscreenContext);
      pDevice->EndCommands(pCommandEncoder);

      continue;
    }

    // 2D path: render into the currently bound render pipeline target.
    RenderSingleBatch(renderViewContext.m_pRenderContext, pRenderData,
      renderViewContext.m_pViewData->m_ViewPortRect.width,
      renderViewContext.m_pViewData->m_ViewPortRect.height);
  }
}

EZ_STATICLINK_FILE(RazorPlugin, RazorPlugin_Rendering_RazorRenderer);
