#include <SharedPluginRazor/SharedPluginRazorPCH.h>

#include <SharedPluginRazor/Common/Messages.h>

// clang-format off

//////////////////////////////////////////////////////////////////////////
// ezRazorElementInfo
//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorElementInfo, 1, ezRTTIDefaultAllocator<ezRazorElementInfo>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ElementId", m_uiElementId),
    EZ_MEMBER_PROPERTY("ParentId", m_uiParentId),
    EZ_MEMBER_PROPERTY("TagName", m_sTagName),
    EZ_MEMBER_PROPERTY("Id", m_sId),
    EZ_ARRAY_MEMBER_PROPERTY("Classes", m_Classes),
    EZ_MEMBER_PROPERTY("HasChildren", m_bHasChildren),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////
// ezRazorStyleProperty
//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorStyleProperty, 1, ezRTTIDefaultAllocator<ezRazorStyleProperty>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Name", m_sName),
    EZ_MEMBER_PROPERTY("Value", m_sValue),
    EZ_MEMBER_PROPERTY("Source", m_sSource),
    EZ_MEMBER_PROPERTY("Inherited", m_bInherited),
    EZ_MEMBER_PROPERTY("Overridden", m_bOverridden),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////
// ezRazorLayoutInfo
//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorLayoutInfo, 1, ezRTTIDefaultAllocator<ezRazorLayoutInfo>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ElementId", m_uiElementId),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////
// ezRazorMatchedRule
//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorMatchedRule, 1, ezRTTIDefaultAllocator<ezRazorMatchedRule>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Selector", m_sSelector),
    EZ_MEMBER_PROPERTY("SourceFile", m_sSourceFile),
    EZ_MEMBER_PROPERTY("SourceLine", m_uiSourceLine),
    EZ_MEMBER_PROPERTY("Specificity", m_uiSpecificity),
    EZ_ARRAY_MEMBER_PROPERTY("Properties", m_Properties),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////
// Messages: Editor -> Engine
//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorGetHierarchyMsgToEngine, 1, ezRTTIDefaultAllocator<ezRazorGetHierarchyMsgToEngine>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorSelectElementMsgToEngine, 1, ezRTTIDefaultAllocator<ezRazorSelectElementMsgToEngine>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ElementId", m_uiElementId),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorGetComputedStyleMsgToEngine, 1, ezRTTIDefaultAllocator<ezRazorGetComputedStyleMsgToEngine>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ElementId", m_uiElementId),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorGetMatchedRulesMsgToEngine, 1, ezRTTIDefaultAllocator<ezRazorGetMatchedRulesMsgToEngine>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ElementId", m_uiElementId),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorGetLayoutMsgToEngine, 1, ezRTTIDefaultAllocator<ezRazorGetLayoutMsgToEngine>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ElementId", m_uiElementId),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorHotReloadMsgToEngine, 1, ezRTTIDefaultAllocator<ezRazorHotReloadMsgToEngine>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ReloadXml", m_bReloadXml),
    EZ_MEMBER_PROPERTY("ReloadCss", m_bReloadCss),
    EZ_MEMBER_PROPERTY("ReloadScripts", m_bReloadScripts),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorHighlightElementMsgToEngine, 1, ezRTTIDefaultAllocator<ezRazorHighlightElementMsgToEngine>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ElementId", m_uiElementId),
    EZ_MEMBER_PROPERTY("Highlight", m_bHighlight),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////
// Messages: Engine -> Editor
//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorHierarchyMsgToEditor, 1, ezRTTIDefaultAllocator<ezRazorHierarchyMsgToEditor>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ARRAY_MEMBER_PROPERTY("Elements", m_Elements),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorComputedStyleMsgToEditor, 1, ezRTTIDefaultAllocator<ezRazorComputedStyleMsgToEditor>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ElementId", m_uiElementId),
    EZ_ARRAY_MEMBER_PROPERTY("Properties", m_Properties),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorMatchedRulesMsgToEditor, 1, ezRTTIDefaultAllocator<ezRazorMatchedRulesMsgToEditor>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ElementId", m_uiElementId),
    EZ_ARRAY_MEMBER_PROPERTY("Rules", m_Rules),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorLayoutMsgToEditor, 1, ezRTTIDefaultAllocator<ezRazorLayoutMsgToEditor>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Layout", m_Layout),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorElementSelectedMsgToEditor, 1, ezRTTIDefaultAllocator<ezRazorElementSelectedMsgToEditor>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ElementId", m_uiElementId),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezRazorDiagnosticMsgToEditor, 1, ezRTTIDefaultAllocator<ezRazorDiagnosticMsgToEditor>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ENUM_MEMBER_PROPERTY("Severity", ezRazorDiagnosticSeverity, m_Severity),
    EZ_MEMBER_PROPERTY("Message", m_sMessage),
    EZ_MEMBER_PROPERTY("SourceFile", m_sSourceFile),
    EZ_MEMBER_PROPERTY("SourceLine", m_uiSourceLine),
    EZ_MEMBER_PROPERTY("SourceColumn", m_uiSourceColumn),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_STATIC_REFLECTED_ENUM(ezRazorDiagnosticSeverity, 1)
  EZ_ENUM_CONSTANT(ezRazorDiagnosticSeverity::Info),
  EZ_ENUM_CONSTANT(ezRazorDiagnosticSeverity::Warning),
  EZ_ENUM_CONSTANT(ezRazorDiagnosticSeverity::Error),
EZ_END_STATIC_REFLECTED_ENUM;

// clang-format on

EZ_STATICLINK_FILE(SharedPluginRazor, SharedPluginRazor_Common_Messages);
