#pragma once

#include <RazorPlugin/RazorPluginDLL.h>
#include <RazorPlugin/Resources/RazorDocumentResource.h>

#include <Core/ResourceManager/ResourceHandle.h>
#include <RendererCore/Components/RenderComponent.h>
#include <RendererFoundation/RendererFoundationDLL.h>

struct ezMsgExtractRenderData;
class ezRazorDocument;
class ezRazorStyleSheet;

/// Base class for Razor UI canvas components.
///
/// Manages the link between an ezEngine entity component and a live Razor document:
/// resource loading, document instantiation, input routing, and render data extraction.
/// Derived classes provide concrete 2D (screen-space) or 3D (world-space) behavior.
class EZ_RAZORPLUGIN_DLL ezRazorCanvasComponentBase : public ezRenderComponent
{
  EZ_DECLARE_ABSTRACT_COMPONENT_TYPE(ezRazorCanvasComponentBase, ezRenderComponent);

public:
  ezRazorCanvasComponentBase();
  ~ezRazorCanvasComponentBase();

  ezRazorCanvasComponentBase& operator=(ezRazorCanvasComponentBase&& rhs);

  virtual void SerializeComponent(ezWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(ezWorldReader& inout_stream) override;

  virtual void Initialize() override;
  virtual void Deinitialize() override;

  virtual void Update();

  void SetRazorResource(const ezRazorDocumentResourceHandle& hResource); // [ property ]
  const ezRazorDocumentResourceHandle& GetRazorResource() const { return m_hResource; } // [ property ]

  ezRazorDocument* GetDocument() { return m_pDocument; }

  virtual ezResult GetLocalBounds(ezBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, ezMsgUpdateLocalBounds& ref_msg) override;

protected:
  virtual void OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const = 0; // [ msg handler ]

  void UpdateCachedResource();

  ezRazorDocumentResourceHandle m_hResource;
  ezEvent<const ezResourceEvent&, ezMutex>::Unsubscriber m_ResourceEventUnsubscriber;

  ezVec2U32 m_vSize = ezVec2U32(1920, 1080);
  ezRazorDocument* m_pDocument = nullptr;
  ezRazorStyleSheet* m_pStyleSheet = nullptr;
};
