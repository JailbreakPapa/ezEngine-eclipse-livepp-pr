#include <EnginePluginRazor/EnginePluginRazorPCH.h>

#include <EditorEngineProcessFramework/EngineProcess/EngineProcessViewContext.h>
#include <EnginePluginRazor/RazorAsset/RazorDocumentContext.h>
#include <EnginePluginRazor/RazorAsset/RazorViewContext.h>
#include <RazorPlugin/Components/RazorCanvas2DComponent.h>
#include <RazorPlugin/Resources/RazorDocumentResource.h>
#include <SharedPluginRazor/Common/Messages.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorDocumentContext, 1, ezRTTIDefaultAllocator<ezRazorDocumentContext>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_CONSTANT_PROPERTY("DocumentType", (const char*) "Razor Document"),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezRazorDocumentContext::ezRazorDocumentContext()
  : ezEngineProcessDocumentContext(ezEngineProcessDocumentContextFlags::CreateWorld)
{
}

ezRazorDocumentContext::~ezRazorDocumentContext() = default;

void ezRazorDocumentContext::SetHighlightedElement(ezUInt32 uiElementId)
{
  m_uiHighlightedElementId = uiElementId;
  // TODO: Update highlight overlay rendering
}

void ezRazorDocumentContext::SetSelectedElement(ezUInt32 uiElementId)
{
  m_uiSelectedElementId = uiElementId;
  // TODO: Update selection highlight rendering
}

void ezRazorDocumentContext::OnInitialize()
{
  auto pWorld = m_pWorld;
  EZ_LOCK(pWorld->GetWriteMarker());

  // Preview object
  {
    ezGameObjectDesc obj;
    obj.m_sName.Assign("RazorPreview");
    obj.m_bDynamic = true;
    pWorld->CreateObject(obj, m_pMainObject);

    ezRazorCanvas2DComponent::CreateComponent(m_pMainObject, m_pCanvasComponent);

    m_pCanvasComponent->SetPassInput(false);
    m_pCanvasComponent->SetSize(ezVec2U32(1024, 768)); // Default preview size

    ezStringBuilder sResourceGuid;
    ezConversionUtils::ToString(GetDocumentGuid(), sResourceGuid);
    m_hMainResource = ezResourceManager::LoadResource<ezRazorDocumentResource>(sResourceGuid);

    m_pCanvasComponent->SetRazorResource(m_hMainResource);
  }
}

ezEngineProcessViewContext* ezRazorDocumentContext::CreateViewContext()
{
  return EZ_DEFAULT_NEW(ezRazorViewContext, this);
}

void ezRazorDocumentContext::DestroyViewContext(ezEngineProcessViewContext* pContext)
{
  EZ_DEFAULT_DELETE(pContext);
}

bool ezRazorDocumentContext::UpdateThumbnailViewContext(ezEngineProcessViewContext* pThumbnailViewContext)
{
  // Thumbnail generation not yet implemented.
  // The asset manager has SupportsThumbnail disabled.
  return false;
}

void ezRazorDocumentContext::HandleMessage(const ezEditorEngineDocumentMsg* pMsg)
{
  if (pMsg->GetDynamicRTTI()->IsDerivedFrom<ezRazorGetHierarchyMsgToEngine>())
  {
    HandleGetHierarchy(static_cast<const ezRazorGetHierarchyMsgToEngine*>(pMsg));
  }
  else if (pMsg->GetDynamicRTTI()->IsDerivedFrom<ezRazorSelectElementMsgToEngine>())
  {
    HandleSelectElement(static_cast<const ezRazorSelectElementMsgToEngine*>(pMsg));
  }
  else if (pMsg->GetDynamicRTTI()->IsDerivedFrom<ezRazorGetComputedStyleMsgToEngine>())
  {
    HandleGetComputedStyle(static_cast<const ezRazorGetComputedStyleMsgToEngine*>(pMsg));
  }
  else if (pMsg->GetDynamicRTTI()->IsDerivedFrom<ezRazorGetLayoutMsgToEngine>())
  {
    HandleGetLayout(static_cast<const ezRazorGetLayoutMsgToEngine*>(pMsg));
  }
  else if (pMsg->GetDynamicRTTI()->IsDerivedFrom<ezRazorHighlightElementMsgToEngine>())
  {
    HandleHighlightElement(static_cast<const ezRazorHighlightElementMsgToEngine*>(pMsg));
  }
  else if (pMsg->GetDynamicRTTI()->IsDerivedFrom<ezRazorHotReloadMsgToEngine>())
  {
    HandleHotReload(static_cast<const ezRazorHotReloadMsgToEngine*>(pMsg));
  }
  else
  {
    ezEngineProcessDocumentContext::HandleMessage(pMsg);
  }
}

void ezRazorDocumentContext::HandleGetHierarchy(const ezRazorGetHierarchyMsgToEngine* pMsg)
{
  ezRazorHierarchyMsgToEditor response;
  response.m_DocumentGuid = GetDocumentGuid();

  // TODO: Extract DOM hierarchy from canvas component
  // For now, send empty hierarchy
  // Once RazorCore integration is complete, iterate through DOM and build element list

  SendProcessMessage(&response);
}

void ezRazorDocumentContext::HandleSelectElement(const ezRazorSelectElementMsgToEngine* pMsg)
{
  SetSelectedElement(pMsg->m_uiElementId);
}

void ezRazorDocumentContext::HandleGetComputedStyle(const ezRazorGetComputedStyleMsgToEngine* pMsg)
{
  ezRazorComputedStyleMsgToEditor response;
  response.m_DocumentGuid = GetDocumentGuid();
  response.m_uiElementId = pMsg->m_uiElementId;

  // TODO: Extract computed style from RazorCore document
  // For now, send empty properties

  SendProcessMessage(&response);
}

void ezRazorDocumentContext::HandleGetLayout(const ezRazorGetLayoutMsgToEngine* pMsg)
{
  ezRazorLayoutMsgToEditor response;
  response.m_DocumentGuid = GetDocumentGuid();
  response.m_Layout.m_uiElementId = pMsg->m_uiElementId;

  // TODO: Extract layout rectangles from RazorCore document
  // For now, send empty layout

  SendProcessMessage(&response);
}

void ezRazorDocumentContext::HandleHighlightElement(const ezRazorHighlightElementMsgToEngine* pMsg)
{
  if (pMsg->m_bHighlight)
  {
    SetHighlightedElement(pMsg->m_uiElementId);
  }
  else
  {
    SetHighlightedElement(0);
  }
}

void ezRazorDocumentContext::HandleHotReload(const ezRazorHotReloadMsgToEngine* pMsg)
{
  EZ_IGNORE_UNUSED(pMsg);

  // Reload the resource
  if (m_hMainResource.IsValid())
  {
    ezResourceManager::ReloadResource(m_hMainResource, false);
  }
}

EZ_STATICLINK_FILE(EnginePluginRazor, EnginePluginRazor_RazorAsset_RazorDocumentContext);
