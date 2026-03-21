#pragma once

#include <RazorCore/RazorCoreDLL.h>

#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>

class ezRazorStyleSheet;
struct ezRazorSelector;
struct ezRazorStyleRule;
struct ezRazorDeclaration;
struct ezRazorStyleValue;
struct ezRazorCssValue;
struct ezRazorCssPropertyValue;
struct ezRazorSimpleSelector;
struct ezRazorSelectorPart;

/// Parses CSS3 syntax into a stylesheet.
///
/// Supports comprehensive CSS3 features:
///
/// **Selectors:**
/// - Type selectors: `div`, `Box`, `*`
/// - Class selectors: `.container`
/// - ID selectors: `#header`
/// - Attribute selectors: `[attr]`, `[attr=value]`, `[attr^=value]`, etc.
/// - Pseudo-classes: `:hover`, `:focus`, `:active`, `:first-child`, `:last-child`,
///   `:nth-child()`, `:nth-of-type()`, `:not()`, `:empty`, `:disabled`, etc.
/// - Pseudo-elements: `::before`, `::after`, `::first-line`, `::first-letter`
/// - Combinators: descendant (space), child (`>`), adjacent sibling (`+`), general sibling (`~`)
/// - Selector lists: `.a, .b, .c`
///
/// **Values:**
/// - Lengths: px, %, em, rem, vh, vw, vmin, vmax, ch, ex, cm, mm, in, pt, pc
/// - Colors: hex (#RGB, #RRGGBB, #RGBA, #RRGGBBAA), rgb(), rgba(), hsl(), hsla(), named colors
/// - Functions: calc(), var(), url(), linear-gradient(), radial-gradient()
/// - Keywords: auto, inherit, initial, unset, none, etc.
/// - Multiple values: `margin: 10px 20px;`
/// - !important flag
///
/// Example:
/// ```css
/// .container {
///   width: calc(100% - 20px);
///   background-color: rgba(255, 255, 255, 0.9);
///   margin: 10px 20px;
/// }
///
/// .button:hover {
///   background-color: var(--primary-color, #3498db);
/// }
/// ```
class EZ_RAZORCORE_DLL ezRazorCssParser
{
public:
  struct ParseOptions
  {
    bool m_bIgnoreUnknownProperties = true;
  };

  /// Parses CSS content into a stylesheet.
  static ezResult Parse(ezStringView sCss, ezRazorStyleSheet& out_stylesheet, const ParseOptions& options = {});

  /// Parses a single CSS color value (hex, rgb, hsl, or named).
  static ezResult ParseColorValue(ezStringView sColor, ezColor& out_color);

private:
  struct ParseContext
  {
    ezRazorStyleSheet* m_pStyleSheet = nullptr;
    const char* m_pCurrent = nullptr;
    const char* m_pEnd = nullptr;
    bool m_bIgnoreUnknown = true;
    ezMap<ezString, ezString> m_CustomProperties;  ///< CSS custom properties (--var-name)
  };

  // Rule parsing
  static ezResult ParseRule(ParseContext& ctx);

  // Complex selector parsing (CSS3)
  static ezResult ParseSelectorList(ParseContext& ctx, ezDynamicArray<ezRazorSelector>& out_selectors);
  static ezResult ParseComplexSelector(ParseContext& ctx, ezRazorSelector& out_selector);
  static ezResult ParseCompoundSelector(ParseContext& ctx, ezRazorSimpleSelector& out_selector);
  static ezResult ParsePseudoClass(ParseContext& ctx, ezRazorSimpleSelector& out_selector);
  static ezResult ParsePseudoElement(ParseContext& ctx, ezRazorSimpleSelector& out_selector);
  static ezResult ParseAttributeSelector(ParseContext& ctx, ezRazorSimpleSelector& out_selector);

  // Legacy simple selector (backward compat)
  static ezResult ParseSelector(ParseContext& ctx, ezRazorSelector& out_selector);

  // Declaration parsing
  static ezResult ParseDeclarations(ParseContext& ctx, ezRazorStyleRule& out_rule);
  static ezResult ParseDeclaration(ParseContext& ctx, ezRazorDeclaration& out_decl);

  // CSS3 value parsing
  static ezResult ParsePropertyValue(ParseContext& ctx, ezRazorCssPropertyValue& out_value);
  static ezResult ParseSingleValue(ParseContext& ctx, ezRazorCssValue& out_value);
  static ezResult ParseRgbColor(ParseContext& ctx, ezColor& out_color);
  static ezResult ParseHslColor(ParseContext& ctx, ezColor& out_color);
  static ezResult ParseCalcExpression(ParseContext& ctx, ezRazorCssValue& out_value);
  static ezResult ParseVarReference(ParseContext& ctx, ezRazorCssValue& out_value);
  static ezResult ParseNamedColor(ezStringView sName, ezColor& out_color);

  // Legacy value parsing (backward compat)
  static ezResult ParseValue(ParseContext& ctx, ezRazorStyleValue& out_value, ezColor& out_color, bool& out_bIsColor);
  static ezResult ParseColor(ParseContext& ctx, ezColor& out_color);

  // Utility functions
  static void SkipWhitespace(ParseContext& ctx);
  static void SkipComment(ParseContext& ctx);
  static void SkipWhitespaceAndComments(ParseContext& ctx);
  static ezStringView ReadIdentifier(ParseContext& ctx);
  static ezStringView ReadString(ParseContext& ctx);
  static ezResult ReadNumber(ParseContext& ctx, float& out_fValue);
  static bool TryReadChar(ParseContext& ctx, char c);
  static bool PeekChar(ParseContext& ctx, char c);
  static bool IsWhitespace(char c);
  static bool IsIdentChar(char c);
  static bool IsIdentStartChar(char c);
  static bool IsHexDigit(char c);
  static bool IsDigit(char c);
  static ezUInt8 HexDigitValue(char c);
};
