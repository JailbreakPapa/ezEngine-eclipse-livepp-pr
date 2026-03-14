#pragma once

#include <Core/ResourceManager/ResourceHandle.h>
#include <EditorFramework/Assets/AssetDocument.h>
#include <EditorFramework/Assets/SimpleAssetDocument.h>
#include <Foundation/Containers/HashTable.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphPins.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphResource.h>
#include <ToolsFoundation/Command/Command.h>
#include <ToolsFoundation/VisualGraph/VisualGraphObjectManager.h>

using ezAnimationClipResourceHandle = ezTypedResourceHandle<class ezAnimationClipResource>;

class ezAnimGraphInstance;
class ezAnimGraphNode;
class ezAnimGraphStateNodeBase;
class ezAnimGraphStateNode;
class ezAnimGraphAnyStateNode;
class ezStateMachineAnimNode;

/// Entry in a breadcrumb navigation path.
struct ezAnimGraphBreadcrumbEntry
{
  ezUuid m_Guid;
  ezString m_sDisplayName;
};

/// Visual graph pin for animation graph nodes.
///
/// Stores animation-specific pin metadata such as the animation data type and whether it supports multiple inputs.
class ezAnimationGraphNodePin : public ezVisualGraphPin
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAnimationGraphNodePin, ezVisualGraphPin);

public:
  ezAnimationGraphNodePin(Type type, const char* szName, const ezColorGammaUB& color, const ezDocumentObject* pObject);
  ~ezAnimationGraphNodePin();

  bool m_bMultiInputPin = false;
  ezAnimGraphPin::Type m_DataType = ezAnimGraphPin::Invalid;
};

/// Object manager for animation graphs with hierarchical scope support.
///
/// Manages animation graph nodes and their connections. Supports hierarchical editing through
/// scope tracking: nodes are stored flat but tagged with a scope UUID indicating which state machine
/// or state they belong to. The editor filters which nodes are visible based on the current scope.
///
/// Scope levels:
/// - Root (invalid UUID): top-level blend tree nodes and state machine nodes
/// - State machine scope (SM node UUID): states and any-state nodes within that SM
/// - State scope (state node UUID): blend tree nodes within that state
class ezAnimationGraphNodeManager : public ezVisualGraphObjectManager
{
public:
  virtual bool InternalIsNode(const ezDocumentObject* pObject) const override;
  virtual void InternalCreatePins(const ezDocumentObject* pObject, NodeInternal& ref_node) override;
  virtual void GetCreateableTypes(ezHybridArray<const ezRTTI*, 32>& ref_types) const override;

  virtual ezStatus InternalCanConnect(const ezVisualGraphPin& source, const ezVisualGraphPin& target, CanConnectResult& out_result) const override;

  virtual const ezRTTI* GetConnectionType() const override;

  //////////////////////////////////////////////////////////////////////////
  // Hierarchical scope support

  /// Sets the currently viewed scope. Invalid UUID = root level.
  void SetCurrentViewScope(const ezUuid& scopeGuid);

  /// Returns the currently viewed scope.
  const ezUuid& GetCurrentViewScope() const { return m_CurrentViewScope; }

  /// Returns whether the current scope is a state machine view (showing states)
  /// or a blend tree view (showing regular AnimGraph nodes).
  bool IsCurrentScopeStateMachine() const;

  /// Builds the breadcrumb path from root to the current scope.
  /// Each entry is (UUID, display name). Root is always the first entry with an invalid UUID.
  void GetBreadcrumbPath(ezDynamicArray<ezAnimGraphBreadcrumbEntry>& out_path) const;

  /// Returns all document object nodes that belong to the given scope.
  void GetNodesInScope(const ezUuid& scopeGuid, ezDynamicArray<const ezDocumentObject*>& out_nodes) const;

  /// Returns the scope UUID a node belongs to. Invalid UUID means root level.
  ezUuid GetNodeScope(const ezDocumentObject* pObject) const;

  /// Sets the scope a node belongs to, stored as metadata.
  void SetNodeScope(const ezDocumentObject* pObject, const ezUuid& scopeGuid);

  /// Returns true if the given document object is a state machine AnimGraph node.
  bool IsStateMachineNode(const ezDocumentObject* pObject) const;

  /// Returns true if the given document object is an AnimGraph state node.
  bool IsStateNode(const ezDocumentObject* pObject) const;

  /// Returns true if the given document object is an AnimGraph "any state" node.
  bool IsAnyStateNode(const ezDocumentObject* pObject) const;

  /// Returns true if the given document object is an AnimGraph transition connection.
  bool IsTransitionConnection(const ezDocumentObject* pObject) const;

  /// Returns the initial state node within the given state machine, or nullptr if none.
  const ezDocumentObject* GetInitialState(const ezUuid& stateMachineGuid) const;

private:
  virtual bool InternalIsDynamicPinProperty(const ezDocumentObject* pObject, const ezAbstractProperty* pProp) const override;

  void CreateBlendTreePins(const ezDocumentObject* pObject, NodeInternal& ref_node);
  void CreateStatePins(const ezDocumentObject* pObject, NodeInternal& ref_node, bool bIsAnyState);
  void CreateStateMachineNodePins(const ezDocumentObject* pObject, NodeInternal& ref_node);

  ezUuid m_CurrentViewScope;
  ezHashTable<ezUuid, ezUuid> m_NodeToScope; ///< Maps node guid -> scope guid
};

/// Undoable command that sets a state as the initial state within a state machine scope.
///
/// Ensures exactly one state per state machine is marked as initial. Clears the flag on the
/// previously marked initial state (if any) and sets it on the new one.
class ezSetAnimGraphInitialStateCommand : public ezCommand
{
  EZ_ADD_DYNAMIC_REFLECTION(ezSetAnimGraphInitialStateCommand, ezCommand);

public:
  ezSetAnimGraphInitialStateCommand();

public:
  ezUuid m_NewInitialStateObject;
  ezUuid m_StateMachineScope;

private:
  virtual ezStatus DoInternal(bool bRedo) override;
  virtual ezStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override {}

  ezDocumentObject* m_pOldInitialStateObject = nullptr;
  ezDocumentObject* m_pNewInitialStateObject = nullptr;
};

class ezAnimationGraphAssetProperties : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAnimationGraphAssetProperties, ezReflectedClass);

public:
  ezDynamicArray<ezString> m_IncludeGraphs;
  ezDynamicArray<ezAnimationClipMapping> m_AnimationClipMapping;
};

class ezAnimationGraphAssetDocument : public ezSimpleAssetDocument<ezAnimationGraphAssetProperties>
{
  EZ_ADD_DYNAMIC_REFLECTION(ezAnimationGraphAssetDocument, ezSimpleAssetDocument<ezAnimationGraphAssetProperties>);

public:
  ezAnimationGraphAssetDocument(ezStringView sDocumentPath);

protected:
  struct PinCount
  {
    ezUInt16 m_uiInputCount = 0;
    ezUInt16 m_uiInputIdx = 0;
    ezUInt16 m_uiOutputCount = 0;
    ezUInt16 m_uiOutputIdx = 0;
  };

  virtual ezTransformStatus InternalTransformAsset(ezStreamWriter& stream, ezStringView sOutputTag, const ezPlatformProfile* pAssetProfile, const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags) override;

  virtual void GetSupportedMimeTypesForPasting(ezHybridArray<ezString, 4>& out_MimeTypes) const override;
  virtual bool CopySelectedObjects(ezAbstractObjectGraph& out_objectGraph, ezStringBuilder& out_MimeType) const override;
  virtual bool Paste(const ezArrayPtr<PasteInfo>& info, const ezAbstractObjectGraph& objectGraph, bool bAllowPickedPosition, ezStringView sMimeType) override;

  virtual void InternalGetMetaDataHash(const ezDocumentObject* pObject, ezUInt64& inout_uiHash) const override;
  virtual void AttachMetaDataBeforeSaving(ezAbstractObjectGraph& graph) const override;
  virtual void RestoreMetaDataAfterLoading(const ezAbstractObjectGraph& graph, bool bUndoable) override;
};
