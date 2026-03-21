#include <RazorPlugin/RazorPluginPCH.h>

#include <RazorPlugin/Components/RazorCanvasComponentBase.h>
#include <RazorPlugin/RazorSystem.h>
#include <RazorPlugin/Resources/RazorDocumentResource.h>
#include <RazorCore/DOM/RazorDocument.h>
#include <RazorCore/Style/RazorStyleSheet.h>

#include <Core/ResourceManager/ResourceManager.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/IO/MemoryStream.h>

// clang-format off
EZ_BEGIN_ABSTRACT_COMPONENT_TYPE(ezRazorCanvasComponentBase, 1)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_RESOURCE_ACCESSOR_PROPERTY("RazorResource", GetRazorResource, SetRazorResource)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Razor_Document")),
  }
  EZ_END_PROPERTIES;

  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezMsgExtractRenderData, OnMsgExtractRenderData),
  }
  EZ_END_MESSAGEHANDLERS;
}
EZ_END_ABSTRACT_COMPONENT_TYPE;
// clang-format on

ezRazorCanvasComponentBase::ezRazorCanvasComponentBase() = default;
ezRazorCanvasComponentBase::~ezRazorCanvasComponentBase() = default;
ezRazorCanvasComponentBase& ezRazorCanvasComponentBase::operator=(ezRazorCanvasComponentBase&& rhs) = default;

void ezRazorCanvasComponentBase::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  auto& s = inout_stream.GetStream();

  s << m_hResource;
}

void ezRazorCanvasComponentBase::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  auto& s = inout_stream.GetStream();

  s >> m_hResource;
}

void ezRazorCanvasComponentBase::Initialize()
{
  SUPER::Initialize();
  UpdateCachedResource();
}

void ezRazorCanvasComponentBase::Deinitialize()
{
  if (ezRazorSystem* pSystem = ezRazorSystem::GetSingleton())
  {
    if (m_pDocument != nullptr)
    {
      pSystem->DestroyDocument(m_pDocument);
      m_pDocument = nullptr;
    }

    if (m_pStyleSheet != nullptr)
    {
      pSystem->DestroyStyleSheet(m_pStyleSheet);
      m_pStyleSheet = nullptr;
    }
  }

  SUPER::Deinitialize();
}

void ezRazorCanvasComponentBase::Update()
{
  if (m_pDocument != nullptr)
  {
    m_pDocument->UpdateLayout(ezVec2(static_cast<float>(m_vSize.x), static_cast<float>(m_vSize.y)));
  }
}

void ezRazorCanvasComponentBase::SetRazorResource(const ezRazorDocumentResourceHandle& hResource)
{
  m_hResource = hResource;
  UpdateCachedResource();
}

ezResult ezRazorCanvasComponentBase::GetLocalBounds(ezBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, ezMsgUpdateLocalBounds& ref_msg)
{
  // 2D UI is always visible (screen-space). 3D overrides with real bounds.
  ref_bAlwaysVisible = true;
  return EZ_SUCCESS;
}

void ezRazorCanvasComponentBase::UpdateCachedResource()
{
  if (!m_hResource.IsValid())
  {
    ezLog::Debug("RazorCanvas: No resource handle set");
    return;
  }

  // Clean up old document
  if (m_pDocument != nullptr)
  {
    if (ezRazorSystem* pSystem = ezRazorSystem::GetSingleton())
    {
      pSystem->DestroyDocument(m_pDocument);
    }
    m_pDocument = nullptr;
    m_pStyleSheet = nullptr;
  }

  // Acquire resource and load compiled data
  ezResourceLock<ezRazorDocumentResource> pResource(m_hResource, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
  if (pResource.GetAcquireResult() != ezResourceAcquireResult::Final)
  {
    ezLog::Warning("Failed to load Razor resource");
    return;
  }

  const ezDataBuffer& compiledData = pResource->GetCompiledData();
  if (compiledData.IsEmpty())
  {
    ezLog::Warning("Razor resource has no compiled data (size={})", compiledData.GetCount());
    return;
  }

  ezLog::Debug("RazorCanvas: Loading resource with {} bytes of compiled data", compiledData.GetCount());

  // Deserialize DOM and stylesheet from compiled data
  ezRazorSystem* pSystem = ezRazorSystem::GetSingleton();
  if (pSystem == nullptr)
  {
    ezLog::Error("RazorSystem not available");
    return;
  }

  m_pDocument = pSystem->CreateDocument();
  m_pStyleSheet = pSystem->CreateStyleSheet();

  ezMemoryStreamContainerWrapperStorage<const ezDataBuffer> storage(&compiledData);
  ezMemoryStreamReader reader(&storage);

  ezUInt32 uiVersion = 0;
  reader >> uiVersion;

  if (uiVersion != 1)
  {
    ezLog::Error("Unknown Razor compiled data version: {}", uiVersion);
    pSystem->DestroyDocument(m_pDocument);
    pSystem->DestroyStyleSheet(m_pStyleSheet);
    m_pDocument = nullptr;
    m_pStyleSheet = nullptr;
    return;
  }

  if (m_pDocument->Deserialize(reader).Failed())
  {
    ezLog::Error("Failed to deserialize Razor document");
    pSystem->DestroyDocument(m_pDocument);
    pSystem->DestroyStyleSheet(m_pStyleSheet);
    m_pDocument = nullptr;
    m_pStyleSheet = nullptr;
    return;
  }

  if (m_pStyleSheet->Deserialize(reader).Failed())
  {
    ezLog::Error("Failed to deserialize Razor stylesheet");
    pSystem->DestroyDocument(m_pDocument);
    pSystem->DestroyStyleSheet(m_pStyleSheet);
    m_pDocument = nullptr;
    m_pStyleSheet = nullptr;
    return;
  }

  ezLog::Debug("RazorCanvas: Loaded document with root={}, stylesheet with {} rules",
               m_pDocument->GetRootElement().IsValid() ? "valid" : "invalid",
               m_pStyleSheet->GetRules().GetCount());

  m_pDocument->AddStyleSheet(m_pStyleSheet);
  m_pDocument->InvalidateAll();
}

EZ_STATICLINK_FILE(RazorPlugin, RazorPlugin_Components_RazorCanvasComponentBase);
