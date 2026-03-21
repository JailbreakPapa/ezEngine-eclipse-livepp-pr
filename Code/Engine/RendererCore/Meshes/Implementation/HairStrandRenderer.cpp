#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Meshes/HairStrandRenderer.h>
#include <RendererCore/Meshes/HairStrandResource.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Pipeline/RenderData.h>
#include <RendererCore/Pipeline/RenderDataBatch.h>
#include <RendererFoundation/CommandEncoder/CommandEncoder.h>
#include <RendererFoundation/Device/Device.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezHairStrandRenderData, 1, ezRTTIDefaultAllocator<ezHairStrandRenderData>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezHairStrandRenderer, 1, ezRTTIDefaultAllocator<ezHairStrandRenderer>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezHairStrandRenderer::ezHairStrandRenderer() = default;
ezHairStrandRenderer::~ezHairStrandRenderer() = default;

void ezHairStrandRenderer::GetSupportedRenderDataTypes(ezDynamicArray<const ezRTTI*>& out_types) const
{
  out_types.PushBack(ezGetStaticRTTI<ezHairStrandRenderData>());
}

void ezHairStrandRenderer::RenderBatch(
  const ezRenderViewContext& renderViewContext,
  const ezRenderPipelinePass* pPass,
  const ezRenderDataBatch& batch) const
{
  ezRenderContext* pRenderContext = renderViewContext.m_pRenderContext;

  for (auto it = batch.GetIterator<ezRenderData>(); it.IsValid(); ++it)
  {
    if (!it->IsInstanceOf<ezHairStrandRenderData>())
      continue;

    const ezHairStrandRenderData* pRenderData = static_cast<const ezHairStrandRenderData*>((const ezRenderData*)it);

    if (!pRenderData->m_hStrandResource.IsValid())
      continue;

    ezResourceLock<ezHairStrandResource> pStrandResource(pRenderData->m_hStrandResource, ezResourceAcquireMode::AllowLoadingFallback);
    if (pStrandResource.GetAcquireResult() != ezResourceAcquireResult::Final)
      continue;

    if (pStrandResource->GetTotalStrandCount() == 0)
      continue;

    // Bind hair material (provides vertex + pixel shaders)
    if (pRenderData->m_hMaterial.IsValid())
    {
      pRenderContext->BindMaterial(pRenderData->m_hMaterial);
    }
    else
    {
      continue; // No material = nothing to render
    }

    // Bind tessellated vertex buffer for the vertex shader to read via SV_VertexID
    ezBindGroupBuilder& bindGroupDraw = pRenderContext->GetBindGroup(EZ_GAL_BIND_GROUP_DRAW_CALL);
    bindGroupDraw.BindBuffer("hairTessVerticesRead", pStrandResource->GetTessVertexBuffer());

    // Draw each strand group
    for (const auto& group : pStrandResource->GetStrandGroups())
    {
      if (group.m_uiStrandCount == 0)
        continue;

      const ezUInt32 uiPointsPerStrand = group.m_uiPointCount / group.m_uiStrandCount;
      if (uiPointsPerStrand < 2)
        continue;

      const ezUInt32 uiSegmentsPerStrand = uiPointsPerStrand - 1;
      const ezUInt32 uiTotalSegments = group.m_uiStrandCount * uiSegmentsPerStrand;
      const ezUInt32 uiTriangles = uiTotalSegments * 2;

      pRenderContext->BindNullMeshBuffer(ezGALPrimitiveTopology::Triangles, uiTriangles);
      pRenderContext->DrawMeshBuffer(uiTriangles).IgnoreResult();
    }
  }
}

EZ_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_HairStrandRenderer);
