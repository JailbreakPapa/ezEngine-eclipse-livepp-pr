#pragma once

#include <ToolsFoundation/VisualGraph/VisualGraphObjectManager.h>

/// Object manager for particle effect visual graphs.
///
/// Manages nodes representing particle systems (layers), emitters, initializers, behaviors,
/// renderers, and event reactions. Validates connections so that only compatible pin categories
/// can be linked (e.g., an emitter output can only connect to a system's emitter input).
class ezParticleEffectNodeManager : public ezVisualGraphObjectManager
{
public:
  virtual bool InternalIsNode(const ezDocumentObject* pObject) const override;
  virtual void InternalCreatePins(const ezDocumentObject* pObject, NodeInternal& ref_node) override;
  virtual void GetCreateableTypes(ezHybridArray<const ezRTTI*, 32>& ref_types) const override;
  virtual ezStatus InternalCanConnect(const ezVisualGraphPin& source, const ezVisualGraphPin& target, CanConnectResult& out_result) const override;
};
