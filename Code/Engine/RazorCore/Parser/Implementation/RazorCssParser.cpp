#include <RazorCore/RazorCorePCH.h>

#include <RazorCore/Parser/RazorCssParser.h>
#include <RazorCore/Style/RazorComputedStyle.h>
#include <RazorCore/Style/RazorStyleSheet.h>

//////////////////////////////////////////////////////////////////////////
// Named CSS colors lookup table
//////////////////////////////////////////////////////////////////////////

struct CssNamedColor
{
  const char* name;
  ezUInt8 r, g, b;
};

// CSS3 named colors (147 colors)
static const CssNamedColor s_NamedColors[] = {
  {"aliceblue", 240, 248, 255},
  {"antiquewhite", 250, 235, 215},
  {"aqua", 0, 255, 255},
  {"aquamarine", 127, 255, 212},
  {"azure", 240, 255, 255},
  {"beige", 245, 245, 220},
  {"bisque", 255, 228, 196},
  {"black", 0, 0, 0},
  {"blanchedalmond", 255, 235, 205},
  {"blue", 0, 0, 255},
  {"blueviolet", 138, 43, 226},
  {"brown", 165, 42, 42},
  {"burlywood", 222, 184, 135},
  {"cadetblue", 95, 158, 160},
  {"chartreuse", 127, 255, 0},
  {"chocolate", 210, 105, 30},
  {"coral", 255, 127, 80},
  {"cornflowerblue", 100, 149, 237},
  {"cornsilk", 255, 248, 220},
  {"crimson", 220, 20, 60},
  {"cyan", 0, 255, 255},
  {"darkblue", 0, 0, 139},
  {"darkcyan", 0, 139, 139},
  {"darkgoldenrod", 184, 134, 11},
  {"darkgray", 169, 169, 169},
  {"darkgreen", 0, 100, 0},
  {"darkgrey", 169, 169, 169},
  {"darkkhaki", 189, 183, 107},
  {"darkmagenta", 139, 0, 139},
  {"darkolivegreen", 85, 107, 47},
  {"darkorange", 255, 140, 0},
  {"darkorchid", 153, 50, 204},
  {"darkred", 139, 0, 0},
  {"darksalmon", 233, 150, 122},
  {"darkseagreen", 143, 188, 143},
  {"darkslateblue", 72, 61, 139},
  {"darkslategray", 47, 79, 79},
  {"darkslategrey", 47, 79, 79},
  {"darkturquoise", 0, 206, 209},
  {"darkviolet", 148, 0, 211},
  {"deeppink", 255, 20, 147},
  {"deepskyblue", 0, 191, 255},
  {"dimgray", 105, 105, 105},
  {"dimgrey", 105, 105, 105},
  {"dodgerblue", 30, 144, 255},
  {"firebrick", 178, 34, 34},
  {"floralwhite", 255, 250, 240},
  {"forestgreen", 34, 139, 34},
  {"fuchsia", 255, 0, 255},
  {"gainsboro", 220, 220, 220},
  {"ghostwhite", 248, 248, 255},
  {"gold", 255, 215, 0},
  {"goldenrod", 218, 165, 32},
  {"gray", 128, 128, 128},
  {"green", 0, 128, 0},
  {"greenyellow", 173, 255, 47},
  {"grey", 128, 128, 128},
  {"honeydew", 240, 255, 240},
  {"hotpink", 255, 105, 180},
  {"indianred", 205, 92, 92},
  {"indigo", 75, 0, 130},
  {"ivory", 255, 255, 240},
  {"khaki", 240, 230, 140},
  {"lavender", 230, 230, 250},
  {"lavenderblush", 255, 240, 245},
  {"lawngreen", 124, 252, 0},
  {"lemonchiffon", 255, 250, 205},
  {"lightblue", 173, 216, 230},
  {"lightcoral", 240, 128, 128},
  {"lightcyan", 224, 255, 255},
  {"lightgoldenrodyellow", 250, 250, 210},
  {"lightgray", 211, 211, 211},
  {"lightgreen", 144, 238, 144},
  {"lightgrey", 211, 211, 211},
  {"lightpink", 255, 182, 193},
  {"lightsalmon", 255, 160, 122},
  {"lightseagreen", 32, 178, 170},
  {"lightskyblue", 135, 206, 250},
  {"lightslategray", 119, 136, 153},
  {"lightslategrey", 119, 136, 153},
  {"lightsteelblue", 176, 196, 222},
  {"lightyellow", 255, 255, 224},
  {"lime", 0, 255, 0},
  {"limegreen", 50, 205, 50},
  {"linen", 250, 240, 230},
  {"magenta", 255, 0, 255},
  {"maroon", 128, 0, 0},
  {"mediumaquamarine", 102, 205, 170},
  {"mediumblue", 0, 0, 205},
  {"mediumorchid", 186, 85, 211},
  {"mediumpurple", 147, 112, 219},
  {"mediumseagreen", 60, 179, 113},
  {"mediumslateblue", 123, 104, 238},
  {"mediumspringgreen", 0, 250, 154},
  {"mediumturquoise", 72, 209, 204},
  {"mediumvioletred", 199, 21, 133},
  {"midnightblue", 25, 25, 112},
  {"mintcream", 245, 255, 250},
  {"mistyrose", 255, 228, 225},
  {"moccasin", 255, 228, 181},
  {"navajowhite", 255, 222, 173},
  {"navy", 0, 0, 128},
  {"oldlace", 253, 245, 230},
  {"olive", 128, 128, 0},
  {"olivedrab", 107, 142, 35},
  {"orange", 255, 165, 0},
  {"orangered", 255, 69, 0},
  {"orchid", 218, 112, 214},
  {"palegoldenrod", 238, 232, 170},
  {"palegreen", 152, 251, 152},
  {"paleturquoise", 175, 238, 238},
  {"palevioletred", 219, 112, 147},
  {"papayawhip", 255, 239, 213},
  {"peachpuff", 255, 218, 185},
  {"peru", 205, 133, 63},
  {"pink", 255, 192, 203},
  {"plum", 221, 160, 221},
  {"powderblue", 176, 224, 230},
  {"purple", 128, 0, 128},
  {"rebeccapurple", 102, 51, 153},
  {"red", 255, 0, 0},
  {"rosybrown", 188, 143, 143},
  {"royalblue", 65, 105, 225},
  {"saddlebrown", 139, 69, 19},
  {"salmon", 250, 128, 114},
  {"sandybrown", 244, 164, 96},
  {"seagreen", 46, 139, 87},
  {"seashell", 255, 245, 238},
  {"sienna", 160, 82, 45},
  {"silver", 192, 192, 192},
  {"skyblue", 135, 206, 235},
  {"slateblue", 106, 90, 205},
  {"slategray", 112, 128, 144},
  {"slategrey", 112, 128, 144},
  {"snow", 255, 250, 250},
  {"springgreen", 0, 255, 127},
  {"steelblue", 70, 130, 180},
  {"tan", 210, 180, 140},
  {"teal", 0, 128, 128},
  {"thistle", 216, 191, 216},
  {"tomato", 255, 99, 71},
  {"turquoise", 64, 224, 208},
  {"violet", 238, 130, 238},
  {"wheat", 245, 222, 179},
  {"white", 255, 255, 255},
  {"whitesmoke", 245, 245, 245},
  {"yellow", 255, 255, 0},
  {"yellowgreen", 154, 205, 50},
};

//////////////////////////////////////////////////////////////////////////
// Main Entry Point
//////////////////////////////////////////////////////////////////////////

ezResult ezRazorCssParser::Parse(ezStringView sCss, ezRazorStyleSheet& out_stylesheet, const ParseOptions& options)
{
  ParseContext ctx;
  ctx.m_pStyleSheet = &out_stylesheet;
  ctx.m_pCurrent = sCss.GetStartPointer();
  ctx.m_pEnd = sCss.GetEndPointer();
  ctx.m_bIgnoreUnknown = options.m_bIgnoreUnknownProperties;

  while (ctx.m_pCurrent < ctx.m_pEnd)
  {
    SkipWhitespaceAndComments(ctx);

    if (ctx.m_pCurrent >= ctx.m_pEnd)
      break;

    EZ_SUCCEED_OR_RETURN(ParseRule(ctx));
  }

  return EZ_SUCCESS;
}

ezResult ezRazorCssParser::ParseColorValue(ezStringView sColor, ezColor& out_color)
{
  ParseContext ctx;
  ctx.m_pCurrent = sColor.GetStartPointer();
  ctx.m_pEnd = sColor.GetEndPointer();

  SkipWhitespaceAndComments(ctx);

  if (ctx.m_pCurrent >= ctx.m_pEnd)
    return EZ_FAILURE;

  // Try hex color
  if (*ctx.m_pCurrent == '#')
  {
    return ParseColor(ctx, out_color);
  }

  // Try rgb/rgba
  ezStringView ident = ReadIdentifier(ctx);
  if (ident == "rgb" || ident == "rgba")
  {
    // Reset and parse
    ctx.m_pCurrent = sColor.GetStartPointer();
    SkipWhitespaceAndComments(ctx);
    ReadIdentifier(ctx);  // skip rgb/rgba
    return ParseRgbColor(ctx, out_color);
  }

  // Try hsl/hsla
  if (ident == "hsl" || ident == "hsla")
  {
    ctx.m_pCurrent = sColor.GetStartPointer();
    SkipWhitespaceAndComments(ctx);
    ReadIdentifier(ctx);  // skip hsl/hsla
    return ParseHslColor(ctx, out_color);
  }

  // Try named color
  return ParseNamedColor(ident, out_color);
}

ezResult ezRazorCssParser::ParseRule(ParseContext& ctx)
{
  // Parse selector list (comma-separated selectors)
  ezDynamicArray<ezRazorSelector> selectors;
  EZ_SUCCEED_OR_RETURN(ParseSelectorList(ctx, selectors));

  SkipWhitespaceAndComments(ctx);

  // Expect '{'
  if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != '{')
  {
    ezLog::Error("Expected '{{' after selector in CSS");
    return EZ_FAILURE;
  }
  ++ctx.m_pCurrent;

  // Parse declarations into a temporary rule
  ezRazorStyleRule tempRule;
  EZ_SUCCEED_OR_RETURN(ParseDeclarations(ctx, tempRule));

  // Expect '}'
  if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != '}')
  {
    ezLog::Error("Expected '}}' at end of rule in CSS");
    return EZ_FAILURE;
  }
  ++ctx.m_pCurrent;

  // Create a rule for each selector in the list
  for (auto& selector : selectors)
  {
    ezRazorStyleRule rule;
    rule.m_Selector = std::move(selector);
    rule.m_Declarations = tempRule.m_Declarations;  // Copy declarations
    ctx.m_pStyleSheet->AddRule(std::move(rule));
  }

  return EZ_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// Selector Parsing (CSS3)
//////////////////////////////////////////////////////////////////////////

ezResult ezRazorCssParser::ParseSelectorList(ParseContext& ctx, ezDynamicArray<ezRazorSelector>& out_selectors)
{
  out_selectors.Clear();

  while (true)
  {
    SkipWhitespaceAndComments(ctx);

    ezRazorSelector selector;
    EZ_SUCCEED_OR_RETURN(ParseComplexSelector(ctx, selector));
    out_selectors.PushBack(std::move(selector));

    SkipWhitespaceAndComments(ctx);

    // Check for comma (selector list)
    if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == ',')
    {
      ++ctx.m_pCurrent;
      continue;
    }

    // End of selector list
    break;
  }

  return out_selectors.IsEmpty() ? EZ_FAILURE : EZ_SUCCESS;
}

ezResult ezRazorCssParser::ParseComplexSelector(ParseContext& ctx, ezRazorSelector& out_selector)
{
  out_selector.m_Parts.Clear();

  while (true)
  {
    SkipWhitespaceAndComments(ctx);

    if (ctx.m_pCurrent >= ctx.m_pEnd)
      break;

    char c = *ctx.m_pCurrent;

    // End of selector?
    if (c == '{' || c == ',')
      break;

    // Parse combinator if we already have at least one part
    ezRazorCombinator::Enum combinator = ezRazorCombinator::None;

    if (!out_selector.m_Parts.IsEmpty())
    {
      // Check for explicit combinator
      if (c == '>')
      {
        combinator = ezRazorCombinator::Child;
        ++ctx.m_pCurrent;
        SkipWhitespaceAndComments(ctx);
      }
      else if (c == '+')
      {
        combinator = ezRazorCombinator::AdjacentSibling;
        ++ctx.m_pCurrent;
        SkipWhitespaceAndComments(ctx);
      }
      else if (c == '~')
      {
        combinator = ezRazorCombinator::GeneralSibling;
        ++ctx.m_pCurrent;
        SkipWhitespaceAndComments(ctx);
      }
      else
      {
        // Implicit descendant combinator (whitespace was already skipped)
        combinator = ezRazorCombinator::Descendant;
      }
    }

    // Parse compound selector
    ezRazorSelectorPart part;
    part.m_Combinator = combinator;

    if (ParseCompoundSelector(ctx, part.m_Selector).Failed())
    {
      if (out_selector.m_Parts.IsEmpty())
        return EZ_FAILURE;
      break;
    }

    out_selector.m_Parts.PushBack(std::move(part));
  }

  // Also fill in legacy simple selector fields for backward compatibility
  if (out_selector.m_Parts.GetCount() == 1 && out_selector.m_Parts[0].m_Combinator == ezRazorCombinator::None)
  {
    const auto& simple = out_selector.m_Parts[0].m_Selector;
    out_selector.m_sTag = simple.m_sTag;
    out_selector.m_sId = simple.m_sId;
    out_selector.m_Classes = simple.m_Classes;
  }

  return out_selector.m_Parts.IsEmpty() ? EZ_FAILURE : EZ_SUCCESS;
}

ezResult ezRazorCssParser::ParseCompoundSelector(ParseContext& ctx, ezRazorSimpleSelector& out_selector)
{
  SkipWhitespaceAndComments(ctx);

  bool bParsedSomething = false;

  while (ctx.m_pCurrent < ctx.m_pEnd)
  {
    char c = *ctx.m_pCurrent;

    // End of compound selector?
    if (c == '{' || c == ',' || c == '>' || c == '+' || c == '~' || IsWhitespace(c))
      break;

    if (c == '.')
    {
      // Class selector
      ++ctx.m_pCurrent;
      ezStringView className = ReadIdentifier(ctx);
      if (className.IsEmpty())
      {
        ezLog::Error("Expected class name after '.' in CSS selector");
        return EZ_FAILURE;
      }
      ezHashedString hs;
      hs.Assign(className);
      out_selector.m_Classes.PushBack(hs);
      bParsedSomething = true;
    }
    else if (c == '#')
    {
      // ID selector
      ++ctx.m_pCurrent;
      ezStringView idName = ReadIdentifier(ctx);
      if (idName.IsEmpty())
      {
        ezLog::Error("Expected ID name after '#' in CSS selector");
        return EZ_FAILURE;
      }
      out_selector.m_sId.Assign(idName);
      bParsedSomething = true;
    }
    else if (c == ':')
    {
      ++ctx.m_pCurrent;

      // Check for pseudo-element (::)
      if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == ':')
      {
        ++ctx.m_pCurrent;
        EZ_SUCCEED_OR_RETURN(ParsePseudoElement(ctx, out_selector));
      }
      else
      {
        EZ_SUCCEED_OR_RETURN(ParsePseudoClass(ctx, out_selector));
      }
      bParsedSomething = true;
    }
    else if (c == '[')
    {
      // Attribute selector
      EZ_SUCCEED_OR_RETURN(ParseAttributeSelector(ctx, out_selector));
      bParsedSomething = true;
    }
    else if (c == '*')
    {
      // Universal selector
      ++ctx.m_pCurrent;
      // m_sTag left empty = matches any tag
      bParsedSomething = true;
    }
    else if (IsIdentStartChar(c))
    {
      // Type selector
      ezStringView tagName = ReadIdentifier(ctx);
      out_selector.m_sTag.Assign(tagName);
      bParsedSomething = true;
    }
    else
    {
      // Unknown character
      break;
    }
  }

  return bParsedSomething ? EZ_SUCCESS : EZ_FAILURE;
}

ezResult ezRazorCssParser::ParsePseudoClass(ParseContext& ctx, ezRazorSimpleSelector& out_selector)
{
  ezStringView pseudoName = ReadIdentifier(ctx);
  if (pseudoName.IsEmpty())
  {
    ezLog::Error("Expected pseudo-class name after ':' in CSS selector");
    return EZ_FAILURE;
  }

  // Simple pseudo-classes (no arguments)
  if (pseudoName == "hover")
    out_selector.m_PseudoClasses.Add(ezRazorPseudoClass::Hover);
  else if (pseudoName == "active")
    out_selector.m_PseudoClasses.Add(ezRazorPseudoClass::Active);
  else if (pseudoName == "focus")
    out_selector.m_PseudoClasses.Add(ezRazorPseudoClass::Focus);
  else if (pseudoName == "focus-within")
    out_selector.m_PseudoClasses.Add(ezRazorPseudoClass::FocusWithin);
  else if (pseudoName == "visited")
    out_selector.m_PseudoClasses.Add(ezRazorPseudoClass::Visited);
  else if (pseudoName == "first-child")
    out_selector.m_PseudoClasses.Add(ezRazorPseudoClass::FirstChild);
  else if (pseudoName == "last-child")
    out_selector.m_PseudoClasses.Add(ezRazorPseudoClass::LastChild);
  else if (pseudoName == "only-child")
    out_selector.m_PseudoClasses.Add(ezRazorPseudoClass::OnlyChild);
  else if (pseudoName == "empty")
    out_selector.m_PseudoClasses.Add(ezRazorPseudoClass::Empty);
  else if (pseudoName == "disabled")
    out_selector.m_PseudoClasses.Add(ezRazorPseudoClass::Disabled);
  else if (pseudoName == "enabled")
    out_selector.m_PseudoClasses.Add(ezRazorPseudoClass::Enabled);
  else if (pseudoName == "checked")
    out_selector.m_PseudoClasses.Add(ezRazorPseudoClass::Checked);
  else if (pseudoName == "nth-child" || pseudoName == "nth-of-type")
  {
    // Functional pseudo-class: :nth-child(an+b)
    SkipWhitespaceAndComments(ctx);
    if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != '(')
    {
      ezLog::Error("Expected '(' after :{}()", ezStringBuilder(pseudoName));
      return EZ_FAILURE;
    }
    ++ctx.m_pCurrent;

    // Parse the expression (simplified: handle "odd", "even", or "n" expressions)
    SkipWhitespaceAndComments(ctx);

    ezStringBuilder expr;
    while (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent != ')')
    {
      expr.Append(*ctx.m_pCurrent);
      ++ctx.m_pCurrent;
    }
    expr.Trim(" \t\n\r");

    if (expr == "odd")
    {
      out_selector.m_iNthChildA = 2;
      out_selector.m_iNthChildB = 1;
    }
    else if (expr == "even")
    {
      out_selector.m_iNthChildA = 2;
      out_selector.m_iNthChildB = 0;
    }
    else
    {
      // Parse "an+b" or "n" or just "b"
      // Simplified parsing
      const char* p = expr.GetData();
      bool bNegativeA = false;
      bool bNegativeB = false;

      if (*p == '-')
      {
        bNegativeA = true;
        ++p;
      }

      if (*p == 'n')
      {
        out_selector.m_iNthChildA = bNegativeA ? -1 : 1;
        ++p;
      }
      else if (IsDigit(*p))
      {
        ezInt32 num = 0;
        while (IsDigit(*p))
        {
          num = num * 10 + (*p - '0');
          ++p;
        }

        if (*p == 'n')
        {
          out_selector.m_iNthChildA = bNegativeA ? -num : num;
          ++p;
        }
        else
        {
          out_selector.m_iNthChildB = bNegativeA ? -num : num;
        }
      }

      // Look for +/- followed by b
      while (*p == ' ') ++p;
      if (*p == '+' || *p == '-')
      {
        bNegativeB = (*p == '-');
        ++p;
        while (*p == ' ') ++p;

        if (IsDigit(*p))
        {
          ezInt32 b = 0;
          while (IsDigit(*p))
          {
            b = b * 10 + (*p - '0');
            ++p;
          }
          out_selector.m_iNthChildB = bNegativeB ? -b : b;
        }
      }
    }

    out_selector.m_bHasNthChild = true;

    if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != ')')
    {
      ezLog::Error("Expected ')' after :{}() expression", ezStringBuilder(pseudoName));
      return EZ_FAILURE;
    }
    ++ctx.m_pCurrent;
  }
  else if (pseudoName == "not")
  {
    // :not() pseudo-class
    SkipWhitespaceAndComments(ctx);
    if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != '(')
    {
      ezLog::Error("Expected '(' after :not()");
      return EZ_FAILURE;
    }
    ++ctx.m_pCurrent;

    // Read until closing paren (simple - doesn't handle nested parens)
    ezStringBuilder notContent;
    ezInt32 parenDepth = 1;
    while (ctx.m_pCurrent < ctx.m_pEnd && parenDepth > 0)
    {
      if (*ctx.m_pCurrent == '(')
        ++parenDepth;
      else if (*ctx.m_pCurrent == ')')
        --parenDepth;

      if (parenDepth > 0)
      {
        notContent.Append(*ctx.m_pCurrent);
      }
      ++ctx.m_pCurrent;
    }

    out_selector.m_sNotSelector = notContent;
  }
  else
  {
    // Unknown pseudo-class - ignore it silently
    // Skip any arguments
    if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == '(')
    {
      ++ctx.m_pCurrent;
      ezInt32 depth = 1;
      while (ctx.m_pCurrent < ctx.m_pEnd && depth > 0)
      {
        if (*ctx.m_pCurrent == '(') ++depth;
        else if (*ctx.m_pCurrent == ')') --depth;
        ++ctx.m_pCurrent;
      }
    }
  }

  return EZ_SUCCESS;
}

ezResult ezRazorCssParser::ParsePseudoElement(ParseContext& ctx, ezRazorSimpleSelector& out_selector)
{
  ezStringView pseudoName = ReadIdentifier(ctx);
  if (pseudoName.IsEmpty())
  {
    ezLog::Error("Expected pseudo-element name after '::' in CSS selector");
    return EZ_FAILURE;
  }

  out_selector.m_sPseudoElement.Assign(pseudoName);
  return EZ_SUCCESS;
}

ezResult ezRazorCssParser::ParseAttributeSelector(ParseContext& ctx, ezRazorSimpleSelector& out_selector)
{
  // Expect '['
  if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != '[')
    return EZ_FAILURE;
  ++ctx.m_pCurrent;

  SkipWhitespaceAndComments(ctx);

  ezRazorAttrSelector attr;

  // Read attribute name
  ezStringView attrName = ReadIdentifier(ctx);
  if (attrName.IsEmpty())
  {
    ezLog::Error("Expected attribute name in attribute selector");
    return EZ_FAILURE;
  }
  attr.m_sAttr.Assign(attrName);

  SkipWhitespaceAndComments(ctx);

  // Check for operator and value
  if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent != ']')
  {
    char c1 = *ctx.m_pCurrent;
    char c2 = (ctx.m_pCurrent + 1 < ctx.m_pEnd) ? *(ctx.m_pCurrent + 1) : '\0';

    if (c1 == '=' && c2 != '=')
    {
      attr.m_Op = ezRazorAttrOp::Equals;
      ++ctx.m_pCurrent;
    }
    else if (c1 == '~' && c2 == '=')
    {
      attr.m_Op = ezRazorAttrOp::Contains;
      ctx.m_pCurrent += 2;
    }
    else if (c1 == '^' && c2 == '=')
    {
      attr.m_Op = ezRazorAttrOp::StartsWith;
      ctx.m_pCurrent += 2;
    }
    else if (c1 == '$' && c2 == '=')
    {
      attr.m_Op = ezRazorAttrOp::EndsWith;
      ctx.m_pCurrent += 2;
    }
    else if (c1 == '*' && c2 == '=')
    {
      attr.m_Op = ezRazorAttrOp::Substring;
      ctx.m_pCurrent += 2;
    }
    else if (c1 == '|' && c2 == '=')
    {
      attr.m_Op = ezRazorAttrOp::DashMatch;
      ctx.m_pCurrent += 2;
    }
    else
    {
      ezLog::Error("Unknown attribute selector operator");
      return EZ_FAILURE;
    }

    SkipWhitespaceAndComments(ctx);

    // Read value (quoted string or identifier)
    if (ctx.m_pCurrent < ctx.m_pEnd)
    {
      if (*ctx.m_pCurrent == '"' || *ctx.m_pCurrent == '\'')
      {
        ezStringView str = ReadString(ctx);
        attr.m_sValue = str;
      }
      else
      {
        ezStringView ident = ReadIdentifier(ctx);
        attr.m_sValue = ident;
      }
    }

    SkipWhitespaceAndComments(ctx);

    // Check for case insensitivity flag 'i'
    if (ctx.m_pCurrent < ctx.m_pEnd && (*ctx.m_pCurrent == 'i' || *ctx.m_pCurrent == 'I'))
    {
      attr.m_bCaseInsensitive = true;
      ++ctx.m_pCurrent;
      SkipWhitespaceAndComments(ctx);
    }
  }
  else
  {
    attr.m_Op = ezRazorAttrOp::Exists;
  }

  // Expect ']'
  if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != ']')
  {
    ezLog::Error("Expected ']' at end of attribute selector");
    return EZ_FAILURE;
  }
  ++ctx.m_pCurrent;

  out_selector.m_AttrSelectors.PushBack(std::move(attr));
  return EZ_SUCCESS;
}

// Legacy selector parsing for backward compatibility
ezResult ezRazorCssParser::ParseSelector(ParseContext& ctx, ezRazorSelector& out_selector)
{
  return ParseComplexSelector(ctx, out_selector);
}

//////////////////////////////////////////////////////////////////////////
// Declaration Parsing
//////////////////////////////////////////////////////////////////////////

ezResult ezRazorCssParser::ParseDeclarations(ParseContext& ctx, ezRazorStyleRule& out_rule)
{
  while (ctx.m_pCurrent < ctx.m_pEnd)
  {
    SkipWhitespaceAndComments(ctx);

    if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent == '}')
      break;

    ezRazorDeclaration decl;
    if (ParseDeclaration(ctx, decl).Succeeded())
    {
      out_rule.m_Declarations.PushBack(std::move(decl));
    }
  }

  return EZ_SUCCESS;
}

ezResult ezRazorCssParser::ParseDeclaration(ParseContext& ctx, ezRazorDeclaration& out_decl)
{
  SkipWhitespaceAndComments(ctx);

  // Check for CSS custom property (--var-name)
  bool bIsCustomProperty = false;
  if (ctx.m_pCurrent + 2 < ctx.m_pEnd && ctx.m_pCurrent[0] == '-' && ctx.m_pCurrent[1] == '-')
  {
    bIsCustomProperty = true;
  }

  // Read property name
  ezStringView propName = ReadIdentifier(ctx);
  if (propName.IsEmpty())
    return EZ_FAILURE;

  out_decl.m_sProperty.Assign(propName);

  SkipWhitespaceAndComments(ctx);

  // Expect ':'
  if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != ':')
  {
    ezLog::Error("Expected ':' after property name '{}'", ezStringBuilder(propName));
    return EZ_FAILURE;
  }
  ++ctx.m_pCurrent;

  SkipWhitespaceAndComments(ctx);

  // Parse value using enhanced CSS3 value parser
  EZ_SUCCEED_OR_RETURN(ParsePropertyValue(ctx, out_decl.m_PropertyValue));

  // Also fill legacy single-value fields for backward compatibility
  if (!out_decl.m_PropertyValue.IsEmpty())
  {
    const auto& firstVal = out_decl.m_PropertyValue[0];
    if (firstVal.m_Type == ezRazorCssValue::Type::Length)
    {
      out_decl.m_Value = firstVal.m_LengthValue;
      out_decl.m_bIsColor = false;
    }
    else if (firstVal.m_Type == ezRazorCssValue::Type::Color)
    {
      out_decl.m_ColorValue = firstVal.m_ColorValue;
      out_decl.m_bIsColor = true;
    }
    else if (firstVal.m_Type == ezRazorCssValue::Type::Keyword)
    {
      if (firstVal.m_sStringValue == "auto")
        out_decl.m_Value = ezRazorStyleValue::MakeAuto();
      out_decl.m_bIsColor = false;
    }
  }

  SkipWhitespaceAndComments(ctx);

  // Check for !important
  if (ctx.m_pCurrent + 10 < ctx.m_pEnd && ctx.m_pCurrent[0] == '!')
  {
    ++ctx.m_pCurrent;
    SkipWhitespaceAndComments(ctx);
    ezStringView imp = ReadIdentifier(ctx);
    if (imp == "important")
    {
      out_decl.m_bImportant = true;
    }
  }

  SkipWhitespaceAndComments(ctx);

  // Expect ';' or '}'
  if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == ';')
  {
    ++ctx.m_pCurrent;
  }

  // Store custom properties for var() resolution
  if (bIsCustomProperty)
  {
    ezStringBuilder varValue;
    for (const auto& v : out_decl.m_PropertyValue.m_Values)
    {
      if (v.m_Type == ezRazorCssValue::Type::String || v.m_Type == ezRazorCssValue::Type::Keyword)
        varValue.Append(v.m_sStringValue);
    }
    ctx.m_CustomProperties[propName] = varValue;
  }

  return EZ_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// CSS3 Value Parsing
//////////////////////////////////////////////////////////////////////////

ezResult ezRazorCssParser::ParsePropertyValue(ParseContext& ctx, ezRazorCssPropertyValue& out_value)
{
  out_value.m_Values.Clear();

  while (ctx.m_pCurrent < ctx.m_pEnd)
  {
    SkipWhitespaceAndComments(ctx);

    if (ctx.m_pCurrent >= ctx.m_pEnd)
      break;

    char c = *ctx.m_pCurrent;

    // End of value?
    if (c == ';' || c == '}' || c == '!')
      break;

    // Handle comma-separated values (for things like font-family)
    if (c == ',')
    {
      ++ctx.m_pCurrent;
      continue;
    }

    ezRazorCssValue val;
    if (ParseSingleValue(ctx, val).Succeeded())
    {
      out_value.m_Values.PushBack(std::move(val));
    }
    else
    {
      // Skip unknown token
      while (ctx.m_pCurrent < ctx.m_pEnd && !IsWhitespace(*ctx.m_pCurrent) &&
             *ctx.m_pCurrent != ';' && *ctx.m_pCurrent != '}' && *ctx.m_pCurrent != '!')
      {
        ++ctx.m_pCurrent;
      }
    }
  }

  return out_value.IsEmpty() ? EZ_FAILURE : EZ_SUCCESS;
}

ezResult ezRazorCssParser::ParseSingleValue(ParseContext& ctx, ezRazorCssValue& out_value)
{
  SkipWhitespaceAndComments(ctx);

  if (ctx.m_pCurrent >= ctx.m_pEnd)
    return EZ_FAILURE;

  char c = *ctx.m_pCurrent;

  // Hex color
  if (c == '#')
  {
    ezColor color;
    EZ_SUCCEED_OR_RETURN(ParseColor(ctx, color));
    out_value = ezRazorCssValue::MakeColor(color);
    return EZ_SUCCESS;
  }

  // Quoted string
  if (c == '"' || c == '\'')
  {
    ezStringView str = ReadString(ctx);
    out_value.m_Type = ezRazorCssValue::Type::String;
    out_value.m_sStringValue = str;
    return EZ_SUCCESS;
  }

  // Number (possibly with unit)
  if (c == '-' || c == '+' || c == '.' || IsDigit(c))
  {
    float fValue = 0.0f;
    EZ_SUCCEED_OR_RETURN(ReadNumber(ctx, fValue));

    // Check for % unit first (not a valid identifier character)
    if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == '%')
    {
      ++ctx.m_pCurrent;
      out_value = ezRazorCssValue::MakeLength({fValue, ezRazorStyleValue::Unit::Percent});
    }
    // Check for other units (identifier-based)
    else
    {
      ezStringView unit = ReadIdentifier(ctx);
      if (unit.IsEmpty())
      {
        out_value = ezRazorCssValue::MakeLength({fValue, ezRazorStyleValue::Unit::Pixels});
      }
      else if (unit == "px")
        out_value = ezRazorCssValue::MakeLength(ezRazorStyleValue::MakePixels(fValue));
      else if (unit == "em")
        out_value = ezRazorCssValue::MakeLength(ezRazorStyleValue::MakeEm(fValue));
      else if (unit == "rem")
        out_value = ezRazorCssValue::MakeLength(ezRazorStyleValue::MakeRem(fValue));
      else if (unit == "vh")
        out_value = ezRazorCssValue::MakeLength(ezRazorStyleValue::MakeVh(fValue));
      else if (unit == "vw")
        out_value = ezRazorCssValue::MakeLength(ezRazorStyleValue::MakeVw(fValue));
      else if (unit == "vmin")
        out_value = ezRazorCssValue::MakeLength({fValue, ezRazorStyleValue::Unit::Vmin});
      else if (unit == "vmax")
        out_value = ezRazorCssValue::MakeLength({fValue, ezRazorStyleValue::Unit::Vmax});
      else if (unit == "ch")
        out_value = ezRazorCssValue::MakeLength({fValue, ezRazorStyleValue::Unit::Ch});
      else if (unit == "ex")
        out_value = ezRazorCssValue::MakeLength({fValue, ezRazorStyleValue::Unit::Ex});
      else if (unit == "cm")
        out_value = ezRazorCssValue::MakeLength({fValue, ezRazorStyleValue::Unit::Cm});
      else if (unit == "mm")
        out_value = ezRazorCssValue::MakeLength({fValue, ezRazorStyleValue::Unit::Mm});
      else if (unit == "in")
        out_value = ezRazorCssValue::MakeLength({fValue, ezRazorStyleValue::Unit::In});
      else if (unit == "pt")
        out_value = ezRazorCssValue::MakeLength({fValue, ezRazorStyleValue::Unit::Pt});
      else if (unit == "pc")
        out_value = ezRazorCssValue::MakeLength({fValue, ezRazorStyleValue::Unit::Pc});
      else if (unit == "deg" || unit == "rad" || unit == "grad" || unit == "turn")
      {
        out_value.m_Type = ezRazorCssValue::Type::Angle;
        out_value.m_fNumberValue = fValue;
        out_value.m_sStringValue = unit;
      }
      else if (unit == "s" || unit == "ms")
      {
        out_value.m_Type = ezRazorCssValue::Type::Time;
        out_value.m_fNumberValue = (unit == "ms") ? fValue / 1000.0f : fValue;
      }
      else
      {
        // Unknown unit, treat as pixels
        out_value = ezRazorCssValue::MakeLength(ezRazorStyleValue::MakePixels(fValue));
      }
    }

    return EZ_SUCCESS;
  }

  // Identifier (keyword or function)
  if (IsIdentStartChar(c))
  {
    ezStringView ident = ReadIdentifier(ctx);

    // Check for function call
    SkipWhitespaceAndComments(ctx);
    if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == '(')
    {
      ++ctx.m_pCurrent;

      if (ident == "rgb" || ident == "rgba")
      {
        ezColor color;
        EZ_SUCCEED_OR_RETURN(ParseRgbColor(ctx, color));
        out_value = ezRazorCssValue::MakeColor(color);
        return EZ_SUCCESS;
      }
      else if (ident == "hsl" || ident == "hsla")
      {
        ezColor color;
        EZ_SUCCEED_OR_RETURN(ParseHslColor(ctx, color));
        out_value = ezRazorCssValue::MakeColor(color);
        return EZ_SUCCESS;
      }
      else if (ident == "calc")
      {
        return ParseCalcExpression(ctx, out_value);
      }
      else if (ident == "var")
      {
        return ParseVarReference(ctx, out_value);
      }
      else if (ident == "url")
      {
        // Parse URL
        SkipWhitespaceAndComments(ctx);
        ezStringBuilder url;
        if (ctx.m_pCurrent < ctx.m_pEnd && (*ctx.m_pCurrent == '"' || *ctx.m_pCurrent == '\''))
        {
          url = ReadString(ctx);
        }
        else
        {
          while (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent != ')')
          {
            url.Append(*ctx.m_pCurrent);
            ++ctx.m_pCurrent;
          }
          url.Trim(" \t\n\r");
        }
        SkipWhitespaceAndComments(ctx);
        if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == ')')
          ++ctx.m_pCurrent;

        out_value.m_Type = ezRazorCssValue::Type::Url;
        out_value.m_sStringValue = url;
        return EZ_SUCCESS;
      }
      else
      {
        // Generic function - store content for later evaluation
        ezStringBuilder funcContent;
        funcContent.Append(ident);
        funcContent.Append("(");
        ezInt32 parenDepth = 1;
        while (ctx.m_pCurrent < ctx.m_pEnd && parenDepth > 0)
        {
          if (*ctx.m_pCurrent == '(') ++parenDepth;
          else if (*ctx.m_pCurrent == ')') --parenDepth;

          if (parenDepth > 0)
            funcContent.Append(*ctx.m_pCurrent);
          ++ctx.m_pCurrent;
        }
        funcContent.Append(")");

        out_value.m_Type = ezRazorCssValue::Type::Function;
        out_value.m_sStringValue = funcContent;
        return EZ_SUCCESS;
      }
    }

    // Try named color first
    ezColor namedColor;
    if (ParseNamedColor(ident, namedColor).Succeeded())
    {
      out_value = ezRazorCssValue::MakeColor(namedColor);
      return EZ_SUCCESS;
    }

    // It's a keyword
    out_value = ezRazorCssValue::MakeKeyword(ident);
    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

//////////////////////////////////////////////////////////////////////////
// Color Parsing
//////////////////////////////////////////////////////////////////////////

ezResult ezRazorCssParser::ParseRgbColor(ParseContext& ctx, ezColor& out_color)
{
  // Already past the opening '('
  SkipWhitespaceAndComments(ctx);

  float r = 0, g = 0, b = 0, a = 1.0f;

  // Red
  if (ReadNumber(ctx, r).Failed())
    return EZ_FAILURE;

  // Check if it's percentage
  bool bIsPercent = false;
  if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == '%')
  {
    bIsPercent = true;
    ++ctx.m_pCurrent;
  }

  SkipWhitespaceAndComments(ctx);

  // Separator can be comma or space (CSS4 syntax)
  bool bUseComma = false;
  if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == ',')
  {
    bUseComma = true;
    ++ctx.m_pCurrent;
    SkipWhitespaceAndComments(ctx);
  }

  // Green
  if (ReadNumber(ctx, g).Failed())
    return EZ_FAILURE;
  if (bIsPercent && ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == '%')
    ++ctx.m_pCurrent;

  SkipWhitespaceAndComments(ctx);
  if (bUseComma && ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == ',')
  {
    ++ctx.m_pCurrent;
    SkipWhitespaceAndComments(ctx);
  }

  // Blue
  if (ReadNumber(ctx, b).Failed())
    return EZ_FAILURE;
  if (bIsPercent && ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == '%')
    ++ctx.m_pCurrent;

  SkipWhitespaceAndComments(ctx);

  // Check for alpha (rgba or css4 rgb with /)
  if (ctx.m_pCurrent < ctx.m_pEnd)
  {
    if (*ctx.m_pCurrent == ',')
    {
      ++ctx.m_pCurrent;
      SkipWhitespaceAndComments(ctx);
      if (ReadNumber(ctx, a).Failed())
        a = 1.0f;
      if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == '%')
      {
        a /= 100.0f;
        ++ctx.m_pCurrent;
      }
    }
    else if (*ctx.m_pCurrent == '/')
    {
      ++ctx.m_pCurrent;
      SkipWhitespaceAndComments(ctx);
      if (ReadNumber(ctx, a).Failed())
        a = 1.0f;
      if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == '%')
      {
        a /= 100.0f;
        ++ctx.m_pCurrent;
      }
    }
  }

  SkipWhitespaceAndComments(ctx);

  // Expect ')'
  if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != ')')
  {
    ezLog::Error("Expected ')' at end of rgb()/rgba() color");
    return EZ_FAILURE;
  }
  ++ctx.m_pCurrent;

  // Normalize values
  if (bIsPercent)
  {
    r /= 100.0f;
    g /= 100.0f;
    b /= 100.0f;
  }
  else
  {
    r /= 255.0f;
    g /= 255.0f;
    b /= 255.0f;
  }

  out_color = ezColor(
    ezMath::Clamp(r, 0.0f, 1.0f),
    ezMath::Clamp(g, 0.0f, 1.0f),
    ezMath::Clamp(b, 0.0f, 1.0f),
    ezMath::Clamp(a, 0.0f, 1.0f));

  return EZ_SUCCESS;
}

ezResult ezRazorCssParser::ParseHslColor(ParseContext& ctx, ezColor& out_color)
{
  // Already past the opening '('
  SkipWhitespaceAndComments(ctx);

  float h = 0, s = 0, l = 0, a = 1.0f;

  // Hue (degrees)
  if (ReadNumber(ctx, h).Failed())
    return EZ_FAILURE;

  // Skip optional 'deg' unit
  ezStringView unit = ReadIdentifier(ctx);
  if (unit == "rad")
    h = h * 180.0f / 3.14159265f;
  else if (unit == "turn")
    h = h * 360.0f;
  else if (unit == "grad")
    h = h * 0.9f;

  SkipWhitespaceAndComments(ctx);

  // Separator
  bool bUseComma = false;
  if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == ',')
  {
    bUseComma = true;
    ++ctx.m_pCurrent;
    SkipWhitespaceAndComments(ctx);
  }

  // Saturation (percentage)
  if (ReadNumber(ctx, s).Failed())
    return EZ_FAILURE;
  if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == '%')
    ++ctx.m_pCurrent;
  s /= 100.0f;

  SkipWhitespaceAndComments(ctx);
  if (bUseComma && ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == ',')
  {
    ++ctx.m_pCurrent;
    SkipWhitespaceAndComments(ctx);
  }

  // Lightness (percentage)
  if (ReadNumber(ctx, l).Failed())
    return EZ_FAILURE;
  if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == '%')
    ++ctx.m_pCurrent;
  l /= 100.0f;

  SkipWhitespaceAndComments(ctx);

  // Check for alpha
  if (ctx.m_pCurrent < ctx.m_pEnd)
  {
    if (*ctx.m_pCurrent == ',' || *ctx.m_pCurrent == '/')
    {
      ++ctx.m_pCurrent;
      SkipWhitespaceAndComments(ctx);
      if (ReadNumber(ctx, a).Failed())
        a = 1.0f;
      if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == '%')
      {
        a /= 100.0f;
        ++ctx.m_pCurrent;
      }
    }
  }

  SkipWhitespaceAndComments(ctx);

  // Expect ')'
  if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != ')')
  {
    ezLog::Error("Expected ')' at end of hsl()/hsla() color");
    return EZ_FAILURE;
  }
  ++ctx.m_pCurrent;

  // Convert HSL to RGB
  h = ezMath::Mod(h, 360.0f);
  if (h < 0) h += 360.0f;
  h /= 360.0f;

  s = ezMath::Clamp(s, 0.0f, 1.0f);
  l = ezMath::Clamp(l, 0.0f, 1.0f);

  float r, g, b;
  if (s == 0)
  {
    r = g = b = l;
  }
  else
  {
    auto hue2rgb = [](float p, float q, float t) -> float
    {
      if (t < 0) t += 1;
      if (t > 1) t -= 1;
      if (t < 1.0f/6.0f) return p + (q - p) * 6 * t;
      if (t < 1.0f/2.0f) return q;
      if (t < 2.0f/3.0f) return p + (q - p) * (2.0f/3.0f - t) * 6;
      return p;
    };

    float q = l < 0.5f ? l * (1 + s) : l + s - l * s;
    float p = 2 * l - q;
    r = hue2rgb(p, q, h + 1.0f/3.0f);
    g = hue2rgb(p, q, h);
    b = hue2rgb(p, q, h - 1.0f/3.0f);
  }

  out_color = ezColor(r, g, b, ezMath::Clamp(a, 0.0f, 1.0f));
  return EZ_SUCCESS;
}

ezResult ezRazorCssParser::ParseCalcExpression(ParseContext& ctx, ezRazorCssValue& out_value)
{
  // Already past the opening '('
  // Store the calc expression as a string for later evaluation
  ezStringBuilder calcExpr;
  ezInt32 parenDepth = 1;

  while (ctx.m_pCurrent < ctx.m_pEnd && parenDepth > 0)
  {
    if (*ctx.m_pCurrent == '(') ++parenDepth;
    else if (*ctx.m_pCurrent == ')') --parenDepth;

    if (parenDepth > 0)
    {
      calcExpr.Append(*ctx.m_pCurrent);
    }
    ++ctx.m_pCurrent;
  }

  out_value.m_Type = ezRazorCssValue::Type::Calc;
  out_value.m_sStringValue = calcExpr;
  return EZ_SUCCESS;
}

ezResult ezRazorCssParser::ParseVarReference(ParseContext& ctx, ezRazorCssValue& out_value)
{
  // Already past the opening '('
  SkipWhitespaceAndComments(ctx);

  ezStringBuilder varName;
  ezStringBuilder fallback;

  // Read variable name (starts with --)
  while (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent != ',' && *ctx.m_pCurrent != ')')
  {
    varName.Append(*ctx.m_pCurrent);
    ++ctx.m_pCurrent;
  }
  varName.Trim(" \t\n\r");

  // Check for fallback value
  if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == ',')
  {
    ++ctx.m_pCurrent;
    SkipWhitespaceAndComments(ctx);

    // Read fallback (can contain nested parens)
    ezInt32 parenDepth = 1;
    while (ctx.m_pCurrent < ctx.m_pEnd && parenDepth > 0)
    {
      if (*ctx.m_pCurrent == '(') ++parenDepth;
      else if (*ctx.m_pCurrent == ')') --parenDepth;

      if (parenDepth > 0)
      {
        fallback.Append(*ctx.m_pCurrent);
      }
      ++ctx.m_pCurrent;
    }
    fallback.Trim(" \t\n\r");
  }
  else
  {
    // Skip closing paren
    if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == ')')
      ++ctx.m_pCurrent;
  }

  out_value.m_Type = ezRazorCssValue::Type::Var;
  out_value.m_sStringValue = varName;
  // Fallback is stored separately if needed (could extend ezRazorCssValue)

  return EZ_SUCCESS;
}

ezResult ezRazorCssParser::ParseNamedColor(ezStringView sName, ezColor& out_color)
{
  // Handle special values
  if (sName == "transparent")
  {
    out_color = ezColor::MakeZero();
    return EZ_SUCCESS;
  }
  if (sName == "currentcolor" || sName == "currentColor")
  {
    // Would need to be resolved at compute time
    out_color = ezColor::Black;
    return EZ_SUCCESS;
  }

  // Binary search through named colors (they're sorted alphabetically)
  ezStringBuilder lower(sName);
  lower.ToLower();

  const int count = EZ_ARRAY_SIZE(s_NamedColors);
  int lo = 0, hi = count - 1;

  while (lo <= hi)
  {
    int mid = (lo + hi) / 2;
    int cmp = lower.Compare(s_NamedColors[mid].name);
    if (cmp == 0)
    {
      out_color = ezColor(
        s_NamedColors[mid].r / 255.0f,
        s_NamedColors[mid].g / 255.0f,
        s_NamedColors[mid].b / 255.0f,
        1.0f);
      return EZ_SUCCESS;
    }
    else if (cmp < 0)
    {
      hi = mid - 1;
    }
    else
    {
      lo = mid + 1;
    }
  }

  return EZ_FAILURE;
}

//////////////////////////////////////////////////////////////////////////
// Legacy Value Parsing (backward compatibility)
//////////////////////////////////////////////////////////////////////////

ezResult ezRazorCssParser::ParseValue(ParseContext& ctx, ezRazorStyleValue& out_value, ezColor& out_color, bool& out_bIsColor)
{
  out_bIsColor = false;

  // Use new CSS3 value parser
  ezRazorCssValue cssValue;
  if (ParseSingleValue(ctx, cssValue).Failed())
    return EZ_FAILURE;

  // Convert to legacy format
  switch (cssValue.m_Type)
  {
    case ezRazorCssValue::Type::Length:
      out_value = cssValue.m_LengthValue;
      out_bIsColor = false;
      break;

    case ezRazorCssValue::Type::Color:
      out_color = cssValue.m_ColorValue;
      out_bIsColor = true;
      break;

    case ezRazorCssValue::Type::Keyword:
      if (cssValue.m_sStringValue == "auto")
        out_value = ezRazorStyleValue::MakeAuto();
      else if (cssValue.m_sStringValue == "none")
        out_value = ezRazorStyleValue::MakeUndefined();
      else
        out_value = ezRazorStyleValue::MakeAuto();
      out_bIsColor = false;
      break;

    default:
      out_value = ezRazorStyleValue::MakeAuto();
      out_bIsColor = false;
      break;
  }

  return EZ_SUCCESS;
}

ezResult ezRazorCssParser::ParseColor(ParseContext& ctx, ezColor& out_color)
{
  // Expect '#'
  if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != '#')
    return EZ_FAILURE;

  ++ctx.m_pCurrent;

  // Read hex digits
  ezStringBuilder hexStr;
  while (ctx.m_pCurrent < ctx.m_pEnd && IsHexDigit(*ctx.m_pCurrent))
  {
    hexStr.Append(*ctx.m_pCurrent);
    ++ctx.m_pCurrent;
  }

  if (hexStr.GetElementCount() == 3)
  {
    // Short form: #RGB -> #RRGGBB
    ezUInt8 r = HexDigitValue(hexStr[0]);
    ezUInt8 g = HexDigitValue(hexStr[1]);
    ezUInt8 b = HexDigitValue(hexStr[2]);
    out_color = ezColor(
      (r * 17) / 255.0f,
      (g * 17) / 255.0f,
      (b * 17) / 255.0f,
      1.0f);
  }
  else if (hexStr.GetElementCount() == 4)
  {
    // Short form with alpha: #RGBA
    ezUInt8 r = HexDigitValue(hexStr[0]);
    ezUInt8 g = HexDigitValue(hexStr[1]);
    ezUInt8 b = HexDigitValue(hexStr[2]);
    ezUInt8 a = HexDigitValue(hexStr[3]);
    out_color = ezColor(
      (r * 17) / 255.0f,
      (g * 17) / 255.0f,
      (b * 17) / 255.0f,
      (a * 17) / 255.0f);
  }
  else if (hexStr.GetElementCount() == 6)
  {
    // Full form: #RRGGBB
    ezUInt8 r = (HexDigitValue(hexStr[0]) << 4) | HexDigitValue(hexStr[1]);
    ezUInt8 g = (HexDigitValue(hexStr[2]) << 4) | HexDigitValue(hexStr[3]);
    ezUInt8 b = (HexDigitValue(hexStr[4]) << 4) | HexDigitValue(hexStr[5]);
    out_color = ezColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
  }
  else if (hexStr.GetElementCount() == 8)
  {
    // Full form with alpha: #RRGGBBAA
    ezUInt8 r = (HexDigitValue(hexStr[0]) << 4) | HexDigitValue(hexStr[1]);
    ezUInt8 g = (HexDigitValue(hexStr[2]) << 4) | HexDigitValue(hexStr[3]);
    ezUInt8 b = (HexDigitValue(hexStr[4]) << 4) | HexDigitValue(hexStr[5]);
    ezUInt8 a = (HexDigitValue(hexStr[6]) << 4) | HexDigitValue(hexStr[7]);
    out_color = ezColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
  }
  else
  {
    ezLog::Error("Invalid hex color format: #{}", hexStr);
    return EZ_FAILURE;
  }

  return EZ_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// Utility Functions
//////////////////////////////////////////////////////////////////////////

void ezRazorCssParser::SkipWhitespace(ParseContext& ctx)
{
  while (ctx.m_pCurrent < ctx.m_pEnd && IsWhitespace(*ctx.m_pCurrent))
  {
    ++ctx.m_pCurrent;
  }
}

void ezRazorCssParser::SkipComment(ParseContext& ctx)
{
  // C-style comments: /* ... */
  if (ctx.m_pCurrent + 2 <= ctx.m_pEnd && ctx.m_pCurrent[0] == '/' && ctx.m_pCurrent[1] == '*')
  {
    ctx.m_pCurrent += 2;
    while (ctx.m_pCurrent + 2 <= ctx.m_pEnd)
    {
      if (ctx.m_pCurrent[0] == '*' && ctx.m_pCurrent[1] == '/')
      {
        ctx.m_pCurrent += 2;
        return;
      }
      ++ctx.m_pCurrent;
    }
    // Unterminated comment
    ctx.m_pCurrent = ctx.m_pEnd;
  }
}

void ezRazorCssParser::SkipWhitespaceAndComments(ParseContext& ctx)
{
  while (ctx.m_pCurrent < ctx.m_pEnd)
  {
    if (IsWhitespace(*ctx.m_pCurrent))
    {
      SkipWhitespace(ctx);
    }
    else if (ctx.m_pCurrent + 2 <= ctx.m_pEnd && ctx.m_pCurrent[0] == '/' && ctx.m_pCurrent[1] == '*')
    {
      SkipComment(ctx);
    }
    else
    {
      break;
    }
  }
}

ezStringView ezRazorCssParser::ReadIdentifier(ParseContext& ctx)
{
  const char* pStart = ctx.m_pCurrent;

  // CSS identifiers can start with -- for custom properties
  if (ctx.m_pCurrent + 2 <= ctx.m_pEnd && ctx.m_pCurrent[0] == '-' && ctx.m_pCurrent[1] == '-')
  {
    ctx.m_pCurrent += 2;
    while (ctx.m_pCurrent < ctx.m_pEnd && IsIdentChar(*ctx.m_pCurrent))
    {
      ++ctx.m_pCurrent;
    }
    return ezStringView(pStart, ctx.m_pCurrent);
  }

  if (ctx.m_pCurrent >= ctx.m_pEnd || !IsIdentStartChar(*ctx.m_pCurrent))
    return ezStringView();

  ++ctx.m_pCurrent;
  while (ctx.m_pCurrent < ctx.m_pEnd && IsIdentChar(*ctx.m_pCurrent))
  {
    ++ctx.m_pCurrent;
  }

  return ezStringView(pStart, ctx.m_pCurrent);
}

ezStringView ezRazorCssParser::ReadString(ParseContext& ctx)
{
  if (ctx.m_pCurrent >= ctx.m_pEnd)
    return ezStringView();

  char quote = *ctx.m_pCurrent;
  if (quote != '"' && quote != '\'')
    return ezStringView();

  ++ctx.m_pCurrent;
  const char* pStart = ctx.m_pCurrent;

  while (ctx.m_pCurrent < ctx.m_pEnd)
  {
    if (*ctx.m_pCurrent == quote)
    {
      ezStringView result(pStart, ctx.m_pCurrent);
      ++ctx.m_pCurrent;
      return result;
    }
    // Handle escape sequences
    if (*ctx.m_pCurrent == '\\' && ctx.m_pCurrent + 1 < ctx.m_pEnd)
    {
      ctx.m_pCurrent += 2;
    }
    else
    {
      ++ctx.m_pCurrent;
    }
  }

  // Unterminated string
  return ezStringView(pStart, ctx.m_pCurrent);
}

ezResult ezRazorCssParser::ReadNumber(ParseContext& ctx, float& out_fValue)
{
  SkipWhitespaceAndComments(ctx);

  if (ctx.m_pCurrent >= ctx.m_pEnd)
    return EZ_FAILURE;

  const char* pStart = ctx.m_pCurrent;
  bool bNegative = false;
  bool bHasDigits = false;

  if (*ctx.m_pCurrent == '-')
  {
    bNegative = true;
    ++ctx.m_pCurrent;
  }
  else if (*ctx.m_pCurrent == '+')
  {
    ++ctx.m_pCurrent;
  }

  float fValue = 0.0f;
  bool bHasDecimal = false;
  float fDecimalMult = 0.1f;

  while (ctx.m_pCurrent < ctx.m_pEnd)
  {
    char c = *ctx.m_pCurrent;
    if (c >= '0' && c <= '9')
    {
      bHasDigits = true;
      if (bHasDecimal)
      {
        fValue += (c - '0') * fDecimalMult;
        fDecimalMult *= 0.1f;
      }
      else
      {
        fValue = fValue * 10.0f + (c - '0');
      }
      ++ctx.m_pCurrent;
    }
    else if (c == '.' && !bHasDecimal)
    {
      bHasDecimal = true;
      ++ctx.m_pCurrent;
    }
    else
    {
      break;
    }
  }

  if (!bHasDigits)
  {
    ctx.m_pCurrent = pStart;
    return EZ_FAILURE;
  }

  out_fValue = bNegative ? -fValue : fValue;
  return EZ_SUCCESS;
}

bool ezRazorCssParser::TryReadChar(ParseContext& ctx, char c)
{
  SkipWhitespaceAndComments(ctx);
  if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == c)
  {
    ++ctx.m_pCurrent;
    return true;
  }
  return false;
}

bool ezRazorCssParser::PeekChar(ParseContext& ctx, char c)
{
  SkipWhitespaceAndComments(ctx);
  return ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == c;
}

bool ezRazorCssParser::IsWhitespace(char c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool ezRazorCssParser::IsIdentStartChar(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '-';
}

bool ezRazorCssParser::IsIdentChar(char c)
{
  return IsIdentStartChar(c) || (c >= '0' && c <= '9');
}

bool ezRazorCssParser::IsHexDigit(char c)
{
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool ezRazorCssParser::IsDigit(char c)
{
  return c >= '0' && c <= '9';
}

ezUInt8 ezRazorCssParser::HexDigitValue(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return 10 + (c - 'a');
  if (c >= 'A' && c <= 'F')
    return 10 + (c - 'A');
  return 0;
}

EZ_STATICLINK_FILE(RazorCore, RazorCore_Parser_RazorCssParser);
