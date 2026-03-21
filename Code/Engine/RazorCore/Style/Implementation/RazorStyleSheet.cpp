#include <RazorCore/RazorCorePCH.h>

#include <RazorCore/Style/RazorStyleSheet.h>

#include <Foundation/IO/Stream.h>

// --- Selector ---

ezUInt32 ezRazorSelector::GetSpecificity() const
{
  // Simplified specificity: IDs = 100, classes = 10 each, tags = 1.
  ezUInt32 spec = 0;

  if (!m_sId.IsEmpty())
    spec += 100;

  spec += m_Classes.GetCount() * 10;

  if (!m_sTag.IsEmpty())
    spec += 1;

  return spec;
}

// --- StyleSheet ---

ezRazorStyleSheet::ezRazorStyleSheet() = default;
ezRazorStyleSheet::~ezRazorStyleSheet() = default;

void ezRazorStyleSheet::AddRule(ezRazorStyleRule&& rule)
{
  m_Rules.PushBack(std::move(rule));
}

void ezRazorStyleSheet::Clear()
{
  m_Rules.Clear();
}

ezResult ezRazorStyleSheet::Serialize(ezStreamWriter& inout_stream) const
{
  const ezUInt32 uiVersion = 1;
  inout_stream << uiVersion;
  inout_stream << m_Rules.GetCount();

  for (const auto& rule : m_Rules)
  {
    inout_stream << rule.m_Selector.m_sTag;
    inout_stream << rule.m_Selector.m_sId;

    inout_stream << rule.m_Selector.m_Classes.GetCount();
    for (const auto& cls : rule.m_Selector.m_Classes)
    {
      inout_stream << cls;
    }

    inout_stream << rule.m_Declarations.GetCount();
    for (const auto& decl : rule.m_Declarations)
    {
      inout_stream << decl.m_sProperty;
      inout_stream << decl.m_Value.m_fValue;
      inout_stream << static_cast<ezUInt8>(decl.m_Value.m_Unit);
      inout_stream << decl.m_bIsColor;
      if (decl.m_bIsColor)
      {
        inout_stream << decl.m_ColorValue;
      }
    }
  }

  return EZ_SUCCESS;
}

ezResult ezRazorStyleSheet::Deserialize(ezStreamReader& inout_stream)
{
  ezUInt32 uiVersion = 0;
  inout_stream >> uiVersion;

  if (uiVersion != 1)
    return EZ_FAILURE;

  ezUInt32 uiRuleCount = 0;
  inout_stream >> uiRuleCount;

  m_Rules.Clear();
  m_Rules.Reserve(uiRuleCount);

  for (ezUInt32 r = 0; r < uiRuleCount; ++r)
  {
    ezRazorStyleRule rule;
    inout_stream >> rule.m_Selector.m_sTag;
    inout_stream >> rule.m_Selector.m_sId;

    ezUInt32 uiClassCount = 0;
    inout_stream >> uiClassCount;
    rule.m_Selector.m_Classes.SetCount(uiClassCount);
    for (ezUInt32 c = 0; c < uiClassCount; ++c)
    {
      inout_stream >> rule.m_Selector.m_Classes[c];
    }

    ezUInt32 uiDeclCount = 0;
    inout_stream >> uiDeclCount;
    rule.m_Declarations.Reserve(uiDeclCount);
    for (ezUInt32 d = 0; d < uiDeclCount; ++d)
    {
      ezRazorDeclaration decl;
      inout_stream >> decl.m_sProperty;
      inout_stream >> decl.m_Value.m_fValue;

      ezUInt8 uiUnit = 0;
      inout_stream >> uiUnit;
      decl.m_Value.m_Unit = static_cast<ezRazorStyleValue::Unit>(uiUnit);

      inout_stream >> decl.m_bIsColor;
      if (decl.m_bIsColor)
      {
        inout_stream >> decl.m_ColorValue;
      }

      rule.m_Declarations.PushBack(std::move(decl));
    }

    m_Rules.PushBack(std::move(rule));
  }

  return EZ_SUCCESS;
}

EZ_STATICLINK_FILE(RazorCore, RazorCore_Style_RazorStyleSheet);
