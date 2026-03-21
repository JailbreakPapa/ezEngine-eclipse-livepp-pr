#pragma once

#include <EditorEngineProcessFramework/EngineProcess/EngineProcessMessages.h>
#include <SharedPluginRazor/SharedPluginRazorDLL.h>

#include <Foundation/Math/Rect.h>
#include <Foundation/Strings/HashedString.h>

//////////////////////////////////////////////////////////////////////////
// Element Info Structures
//////////////////////////////////////////////////////////////////////////

/// Simplified element info for hierarchy display in the editor.
struct EZ_SHAREDPLUGINRAZOR_DLL ezRazorElementInfo : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorElementInfo, ezReflectedClass);

public:
  ezUInt32 m_uiElementId = 0;
  ezUInt32 m_uiParentId = 0;
  ezString m_sTagName;
  ezString m_sId;
  ezHybridArray<ezString, 4> m_Classes;
  bool m_bHasChildren = false;
};

/// Computed style property for debugging display.
struct EZ_SHAREDPLUGINRAZOR_DLL ezRazorStyleProperty : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorStyleProperty, ezReflectedClass);

public:
  ezString m_sName;
  ezString m_sValue;
  ezString m_sSource;    ///< Where this value came from (e.g., "style.rcss:42")
  bool m_bInherited = false;
  bool m_bOverridden = false;
};

/// Layout rectangle info for an element.
struct EZ_SHAREDPLUGINRAZOR_DLL ezRazorLayoutInfo : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorLayoutInfo, ezReflectedClass);

public:
  ezUInt32 m_uiElementId = 0;
  ezRectFloat m_ContentBox;
  ezRectFloat m_PaddingBox;
  ezRectFloat m_BorderBox;
  ezRectFloat m_MarginBox;
};

/// Matched CSS rule for style debugging.
struct EZ_SHAREDPLUGINRAZOR_DLL ezRazorMatchedRule : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorMatchedRule, ezReflectedClass);

public:
  ezString m_sSelector;
  ezString m_sSourceFile;
  ezUInt32 m_uiSourceLine = 0;
  ezUInt32 m_uiSpecificity = 0;
  ezDynamicArray<ezRazorStyleProperty> m_Properties;
};

//////////////////////////////////////////////////////////////////////////
// Messages: Editor -> Engine
//////////////////////////////////////////////////////////////////////////

/// Request the DOM hierarchy for display in the editor's hierarchy panel.
class EZ_SHAREDPLUGINRAZOR_DLL ezRazorGetHierarchyMsgToEngine : public ezEditorEngineDocumentMsg
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorGetHierarchyMsgToEngine, ezEditorEngineDocumentMsg);
};

/// Notify the engine that an element was selected in the editor.
class EZ_SHAREDPLUGINRAZOR_DLL ezRazorSelectElementMsgToEngine : public ezEditorEngineDocumentMsg
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorSelectElementMsgToEngine, ezEditorEngineDocumentMsg);

public:
  ezUInt32 m_uiElementId = 0;
};

/// Request computed style for a specific element.
class EZ_SHAREDPLUGINRAZOR_DLL ezRazorGetComputedStyleMsgToEngine : public ezEditorEngineDocumentMsg
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorGetComputedStyleMsgToEngine, ezEditorEngineDocumentMsg);

public:
  ezUInt32 m_uiElementId = 0;
};

/// Request matched rules for a specific element (for CSS debugging).
class EZ_SHAREDPLUGINRAZOR_DLL ezRazorGetMatchedRulesMsgToEngine : public ezEditorEngineDocumentMsg
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorGetMatchedRulesMsgToEngine, ezEditorEngineDocumentMsg);

public:
  ezUInt32 m_uiElementId = 0;
};

/// Request layout info for a specific element.
class EZ_SHAREDPLUGINRAZOR_DLL ezRazorGetLayoutMsgToEngine : public ezEditorEngineDocumentMsg
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorGetLayoutMsgToEngine, ezEditorEngineDocumentMsg);

public:
  ezUInt32 m_uiElementId = 0;
};

/// Request hot reload of the document from source files.
class EZ_SHAREDPLUGINRAZOR_DLL ezRazorHotReloadMsgToEngine : public ezEditorEngineDocumentMsg
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorHotReloadMsgToEngine, ezEditorEngineDocumentMsg);

public:
  bool m_bReloadXml = true;
  bool m_bReloadCss = true;
  bool m_bReloadScripts = true;
};

/// Highlight a specific element in the preview (e.g., on hover in hierarchy).
class EZ_SHAREDPLUGINRAZOR_DLL ezRazorHighlightElementMsgToEngine : public ezEditorEngineDocumentMsg
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorHighlightElementMsgToEngine, ezEditorEngineDocumentMsg);

public:
  ezUInt32 m_uiElementId = 0;
  bool m_bHighlight = true;
};

//////////////////////////////////////////////////////////////////////////
// Messages: Engine -> Editor
//////////////////////////////////////////////////////////////////////////

/// Response containing the DOM hierarchy.
class EZ_SHAREDPLUGINRAZOR_DLL ezRazorHierarchyMsgToEditor : public ezEditorEngineDocumentMsg
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorHierarchyMsgToEditor, ezEditorEngineDocumentMsg);

public:
  ezDynamicArray<ezRazorElementInfo> m_Elements;
};

/// Response containing computed style properties for an element.
class EZ_SHAREDPLUGINRAZOR_DLL ezRazorComputedStyleMsgToEditor : public ezEditorEngineDocumentMsg
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorComputedStyleMsgToEditor, ezEditorEngineDocumentMsg);

public:
  ezUInt32 m_uiElementId = 0;
  ezDynamicArray<ezRazorStyleProperty> m_Properties;
};

/// Response containing matched CSS rules for an element.
class EZ_SHAREDPLUGINRAZOR_DLL ezRazorMatchedRulesMsgToEditor : public ezEditorEngineDocumentMsg
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorMatchedRulesMsgToEditor, ezEditorEngineDocumentMsg);

public:
  ezUInt32 m_uiElementId = 0;
  ezDynamicArray<ezRazorMatchedRule> m_Rules;
};

/// Response containing layout info for an element.
class EZ_SHAREDPLUGINRAZOR_DLL ezRazorLayoutMsgToEditor : public ezEditorEngineDocumentMsg
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorLayoutMsgToEditor, ezEditorEngineDocumentMsg);

public:
  ezRazorLayoutInfo m_Layout;
};

/// Notification that the engine-side element selection changed (e.g., from clicking in preview).
class EZ_SHAREDPLUGINRAZOR_DLL ezRazorElementSelectedMsgToEditor : public ezEditorEngineDocumentMsg
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorElementSelectedMsgToEditor, ezEditorEngineDocumentMsg);

public:
  ezUInt32 m_uiElementId = 0;
};

/// Severity level for diagnostic messages.
struct EZ_SHAREDPLUGINRAZOR_DLL ezRazorDiagnosticSeverity
{
  using StorageType = ezUInt8;

  enum Enum : ezUInt8
  {
    Info = 0,
    Warning = 1,
    Error = 2,
    Default = Info
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_SHAREDPLUGINRAZOR_DLL, ezRazorDiagnosticSeverity);

/// Diagnostic message from the engine (parse errors, runtime warnings, etc.).
class EZ_SHAREDPLUGINRAZOR_DLL ezRazorDiagnosticMsgToEditor : public ezEditorEngineDocumentMsg
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRazorDiagnosticMsgToEditor, ezEditorEngineDocumentMsg);

public:
  ezEnum<ezRazorDiagnosticSeverity> m_Severity;
  ezString m_sMessage;
  ezString m_sSourceFile;
  ezUInt32 m_uiSourceLine = 0;
  ezUInt32 m_uiSourceColumn = 0;
};
