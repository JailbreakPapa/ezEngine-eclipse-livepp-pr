#include <RazorPlugin/RazorPluginPCH.h>

#include <RazorPlugin/Components/RazorCanvas2DComponent.h>
#include <RazorPlugin/Rendering/RazorGeometryBuilder.h>
#include <RazorPlugin/Rendering/RazorRenderData.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Pipeline/RenderData.h>
#include <RendererCore/Pipeline/RenderDataManager.h>
#include <RendererCore/Pipeline/View.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezRazorCanvas2DComponent, 1, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("Offset", GetOffset, SetOffset),
    EZ_ACCESSOR_PROPERTY("Size", GetSize, SetSize)->AddAttributes(new ezDefaultValueAttribute(ezVec2U32(1920, 1080))),
    EZ_ACCESSOR_PROPERTY("PassInput", GetPassInput, SetPassInput)->AddAttributes(new ezDefaultValueAttribute(true)),
  }
  EZ_END_PROPERTIES;
}
EZ_END_COMPONENT_TYPE;
// clang-format on

ezRazorCanvas2DComponent::ezRazorCanvas2DComponent() = default;
ezRazorCanvas2DComponent::~ezRazorCanvas2DComponent() = default;

ezRazorCanvas2DComponent& ezRazorCanvas2DComponent::operator=(ezRazorCanvas2DComponent&& rhs)
{
  SUPER::operator=(std::move(rhs));
  m_vOffset = rhs.m_vOffset;
  m_bPassInput = rhs.m_bPassInput;
  m_bPaintDirty = true;
  return *this;
}

void ezRazorCanvas2DComponent::OnActivated()
{
  SUPER::OnActivated();
}

void ezRazorCanvas2DComponent::Update()
{
  SUPER::Update();

  // After layout update from base class, mark paint as dirty.
  m_bPaintDirty = true;
}

void ezRazorCanvas2DComponent::SetOffset(const ezVec2I32& vOffset)
{
  m_vOffset = vOffset;
}

void ezRazorCanvas2DComponent::SetSize(const ezVec2U32& vSize)
{
  m_vSize = vSize;
}

void ezRazorCanvas2DComponent::SetPassInput(bool bPassInput)
{
  m_bPassInput = bPassInput;
}

void ezRazorCanvas2DComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  auto& s = inout_stream.GetStream();

  s << m_vOffset;
  s << m_vSize;
  s << m_bPassInput;
}

void ezRazorCanvas2DComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  auto& s = inout_stream.GetStream();

  s >> m_vOffset;
  s >> m_vSize;
  s >> m_bPassInput;
}

void ezRazorCanvas2DComponent::OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const
{
  if (msg.m_pView->GetCameraUsageHint() != ezCameraUsageHint::MainView &&
      msg.m_pView->GetCameraUsageHint() != ezCameraUsageHint::EditorView &&
      msg.m_pView->GetCameraUsageHint() != ezCameraUsageHint::Thumbnail)
    return;

  // Don't extract for selection override categories.
  if (msg.m_OverrideCategory != ezInvalidRenderDataCategory)
    return;

  if (m_pDocument == nullptr)
  {
    ezLog::Debug("RazorCanvas2D: m_pDocument is null");
    return;
  }

  // Rebuild paint commands if dirty.
  if (m_bPaintDirty)
  {
    const_cast<ezRazorPaintGenerator&>(m_PaintGenerator).Generate(*m_pDocument, m_RenderBatch);
    m_bPaintDirty = false;

    ezLog::Debug("RazorCanvas2D: Generated {} paint commands", m_RenderBatch.GetCommands().GetCount());
  }

  if (m_RenderBatch.GetCommands().IsEmpty())
  {
    ezLog::Debug("RazorCanvas2D: No paint commands to render");
    return;
  }

  // Convert paint commands to GPU vertex/index data.
  auto pRenderData = msg.m_pRenderDataManager->CreateRenderDataForThisFrame<ezRazorRenderData>(GetOwner());

  ezRazorGeometryBuilder builder;
  builder.Build(m_RenderBatch, *pRenderData);

  ezLog::Debug("RazorCanvas2D: Built {} vertices, {} draw calls", pRenderData->m_Vertices.GetCount(), pRenderData->m_DrawCalls.GetCount());

  if (!pRenderData->m_Vertices.IsEmpty())
  {
    msg.AddRenderData(pRenderData, ezDefaultRenderDataCategories::GUI, ezRenderData::Caching::Never);
  }
}

EZ_STATICLINK_FILE(RazorPlugin, RazorPlugin_Components_RazorCanvas2DComponent);
