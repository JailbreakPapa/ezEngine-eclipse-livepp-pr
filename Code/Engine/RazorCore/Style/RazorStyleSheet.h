#pragma once

#include <RazorCore/RazorCoreDLL.h>
#include <RazorCore/Style/RazorComputedStyle.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Strings/HashedString.h>

/// CSS selector combinator types.
struct ezRazorCombinator
{
  using StorageType = ezUInt8;

  enum Enum : ezUInt8
  {
    None,           ///< No combinator (single selector or start of chain)
    Descendant,     ///< Space - matches any descendant
    Child,          ///< > - matches direct children only
    AdjacentSibling,///< + - matches immediately following sibling
    GeneralSibling, ///< ~ - matches any following sibling

    Default = None
  };
};

/// CSS pseudo-class types.
struct ezRazorPseudoClass
{
  using StorageType = ezUInt16;

  enum Enum : ezUInt16
  {
    None = 0,
    Hover = EZ_BIT(0),
    Active = EZ_BIT(1),
    Focus = EZ_BIT(2),
    FocusWithin = EZ_BIT(3),
    Visited = EZ_BIT(4),
    FirstChild = EZ_BIT(5),
    LastChild = EZ_BIT(6),
    OnlyChild = EZ_BIT(7),
    Empty = EZ_BIT(8),
    Disabled = EZ_BIT(9),
    Enabled = EZ_BIT(10),
    Checked = EZ_BIT(11),
    // NthChild handled separately with parameter

    Default = None
  };

  struct Bits
  {
    StorageType Hover : 1;
    StorageType Active : 1;
    StorageType Focus : 1;
    StorageType FocusWithin : 1;
    StorageType Visited : 1;
    StorageType FirstChild : 1;
    StorageType LastChild : 1;
    StorageType OnlyChild : 1;
    StorageType Empty : 1;
    StorageType Disabled : 1;
    StorageType Enabled : 1;
    StorageType Checked : 1;
  };
};

EZ_DECLARE_FLAGS_OPERATORS(ezRazorPseudoClass);

/// CSS attribute selector operator.
struct ezRazorAttrOp
{
  using StorageType = ezUInt8;

  enum Enum : ezUInt8
  {
    Exists,         ///< [attr] - has attribute
    Equals,         ///< [attr=value] - exact match
    Contains,       ///< [attr~=value] - contains word
    StartsWith,     ///< [attr^=value] - starts with
    EndsWith,       ///< [attr$=value] - ends with
    Substring,      ///< [attr*=value] - contains substring
    DashMatch,      ///< [attr|=value] - starts with value or value-
  };
};

/// An attribute selector condition.
struct EZ_RAZORCORE_DLL ezRazorAttrSelector
{
  ezHashedString m_sAttr;
  ezString m_sValue;
  ezEnum<ezRazorAttrOp> m_Op = ezRazorAttrOp::Exists;
  bool m_bCaseInsensitive = false;
};

/// A single simple selector (one part of a compound/complex selector).
struct EZ_RAZORCORE_DLL ezRazorSimpleSelector
{
  ezHashedString m_sTag;                          ///< Empty = matches any tag (or *)
  ezHashedString m_sId;                           ///< Empty = not constrained by ID.
  ezHybridArray<ezHashedString, 2> m_Classes;     ///< All must match.
  ezBitflags<ezRazorPseudoClass> m_PseudoClasses; ///< Pseudo-class flags.
  ezHybridArray<ezRazorAttrSelector, 1> m_AttrSelectors; ///< Attribute selectors.

  // :nth-child(an+b) parameters
  ezInt32 m_iNthChildA = 0;
  ezInt32 m_iNthChildB = 0;
  bool m_bHasNthChild = false;

  // :not() contains nested selector(s)
  ezString m_sNotSelector;  ///< Raw content of :not() for later parsing

  // ::before, ::after pseudo-elements
  ezHashedString m_sPseudoElement;
};

/// A compound selector is a sequence of simple selectors (e.g., div.class#id:hover).
/// A complex selector is compound selectors joined by combinators.
struct EZ_RAZORCORE_DLL ezRazorSelectorPart
{
  ezRazorSimpleSelector m_Selector;
  ezEnum<ezRazorCombinator> m_Combinator = ezRazorCombinator::None; ///< Combinator before this part
};

/// A CSS-like selector that can be simple, compound, or complex.
///
/// Supports:
/// - Type selectors: `div`, `Box`, `*`
/// - Class selectors: `.container`
/// - ID selectors: `#header`
/// - Attribute selectors: `[attr]`, `[attr=value]`, etc.
/// - Pseudo-classes: `:hover`, `:first-child`, `:nth-child(2n+1)`, etc.
/// - Pseudo-elements: `::before`, `::after`
/// - Combinators: descendant (space), child (`>`), adjacent sibling (`+`), general sibling (`~`)
struct EZ_RAZORCORE_DLL ezRazorSelector
{
  ezDynamicArray<ezRazorSelectorPart> m_Parts;

  // Legacy simple selector interface for backward compatibility
  ezHashedString m_sTag;
  ezHashedString m_sId;
  ezHybridArray<ezHashedString, 2> m_Classes;

  ezUInt32 GetSpecificity() const;

  /// Returns true if this selector uses only simple tag/class/id matching (legacy mode).
  bool IsSimple() const { return m_Parts.IsEmpty(); }
};

/// A single property declaration, e.g. "width: 100px".
struct EZ_RAZORCORE_DLL ezRazorDeclaration
{
  ezHashedString m_sProperty;
  ezRazorCssPropertyValue m_PropertyValue;  ///< New multi-value support

  // Legacy single-value interface for backward compatibility
  ezRazorStyleValue m_Value;
  ezColor m_ColorValue;
  bool m_bIsColor = false;
  bool m_bImportant = false;  ///< !important flag
};

/// A selector-declaration pair (one CSS rule).
struct EZ_RAZORCORE_DLL ezRazorStyleRule
{
  ezRazorSelector m_Selector;
  ezDynamicArray<ezRazorDeclaration> m_Declarations;
};

/// A compiled stylesheet containing ordered rules for selector matching and cascade.
///
/// Typically compiled from CSS source at editor/asset time. At runtime, documents
/// reference one or more stylesheets to resolve element styles.
class EZ_RAZORCORE_DLL ezRazorStyleSheet
{
public:
  ezRazorStyleSheet();
  ~ezRazorStyleSheet();

  void AddRule(ezRazorStyleRule&& rule);
  const ezDynamicArray<ezRazorStyleRule>& GetRules() const { return m_Rules; }

  void Clear();

  ezResult Serialize(ezStreamWriter& inout_stream) const;
  ezResult Deserialize(ezStreamReader& inout_stream);

private:
  ezDynamicArray<ezRazorStyleRule> m_Rules;
};
