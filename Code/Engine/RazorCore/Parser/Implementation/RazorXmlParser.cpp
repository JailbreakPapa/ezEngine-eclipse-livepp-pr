#include <RazorCore/RazorCorePCH.h>

#include <RazorCore/Parser/RazorXmlParser.h>
#include <RazorCore/DOM/RazorDocument.h>
#include <RazorCore/DOM/RazorElement.h>

ezResult ezRazorXmlParser::Parse(ezStringView sXml, ezRazorDocument& out_document, const ParseOptions& options)
{
  ParseContext ctx;
  ctx.m_pDocument = &out_document;
  ctx.m_pCurrent = sXml.GetStartPointer();
  ctx.m_pEnd = sXml.GetEndPointer();
  ctx.m_bStripWhitespace = options.m_bStripWhitespace;

  SkipWhitespace(ctx);

  // Skip XML declaration if present: <?xml ... ?>
  if (ctx.m_pCurrent + 2 <= ctx.m_pEnd && ctx.m_pCurrent[0] == '<' && ctx.m_pCurrent[1] == '?')
  {
    while (ctx.m_pCurrent + 2 <= ctx.m_pEnd)
    {
      if (ctx.m_pCurrent[0] == '?' && ctx.m_pCurrent[1] == '>')
      {
        ctx.m_pCurrent += 2;
        break;
      }
      ++ctx.m_pCurrent;
    }
    SkipWhitespace(ctx);
  }

  // Skip doctype if present: <!DOCTYPE ... >
  if (ctx.m_pCurrent + 9 <= ctx.m_pEnd && ezStringUtils::StartsWith(ctx.m_pCurrent, "<!DOCTYPE"))
  {
    while (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent != '>')
      ++ctx.m_pCurrent;
    if (ctx.m_pCurrent < ctx.m_pEnd)
      ++ctx.m_pCurrent;
    SkipWhitespace(ctx);
  }

  // Parse root element
  if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != '<')
  {
    ezLog::Error("Expected root element in Razor XML");
    return EZ_FAILURE;
  }

  ezRazorElementId invalidParent;
  return ParseElement(ctx, invalidParent);
}

ezResult ezRazorXmlParser::ParseElement(ParseContext& ctx, ezRazorElementId parent)
{
  // Expect '<'
  if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != '<')
    return EZ_FAILURE;

  ++ctx.m_pCurrent;

  // Check for comments
  if (SkipComment(ctx))
  {
    return EZ_SUCCESS;
  }

  // Read tag name
  ezStringView tagName = ReadIdentifier(ctx);
  if (tagName.IsEmpty())
  {
    ezLog::Error("Expected tag name in Razor XML");
    return EZ_FAILURE;
  }

  // Create element
  ezStringBuilder tagNameStr(tagName);
  ezRazorElementId elementId = ctx.m_pDocument->CreateElement(tagNameStr);
  ezRazorElement* pElement = ctx.m_pDocument->GetElement(elementId);
  if (pElement == nullptr)
  {
    ezLog::Error("Failed to create element '{}'", tagNameStr);
    return EZ_FAILURE;
  }

  // Set root if no parent
  if (!parent.IsValid())
  {
    ctx.m_pDocument->SetRootElement(elementId);
  }
  else
  {
    ctx.m_pDocument->AppendChild(parent, elementId);
  }

  // Parse attributes
  EZ_SUCCEED_OR_RETURN(ParseAttributes(ctx, elementId));

  SkipWhitespace(ctx);

  // Self-closing tag?
  if (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent == '/')
  {
    ++ctx.m_pCurrent;
    if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != '>')
    {
      ezLog::Error("Expected '>' after '/' in self-closing tag");
      return EZ_FAILURE;
    }
    ++ctx.m_pCurrent;
    return EZ_SUCCESS;
  }

  // Expect '>'
  if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != '>')
  {
    ezLog::Error("Expected '>' at end of opening tag");
    return EZ_FAILURE;
  }
  ++ctx.m_pCurrent;

  // Parse children and text content
  while (ctx.m_pCurrent < ctx.m_pEnd)
  {
    SkipWhitespace(ctx);

    if (ctx.m_pCurrent >= ctx.m_pEnd)
      break;

    // Check for closing tag
    if (ctx.m_pCurrent + 2 <= ctx.m_pEnd && ctx.m_pCurrent[0] == '<' && ctx.m_pCurrent[1] == '/')
    {
      ctx.m_pCurrent += 2;
      ezStringView closingTag = ReadIdentifier(ctx);
      if (closingTag != tagName)
      {
        ezLog::Error("Mismatched closing tag: expected '</{}>' but found '</{}>'",
                     tagNameStr, ezStringBuilder(closingTag));
        return EZ_FAILURE;
      }
      SkipWhitespace(ctx);
      if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != '>')
      {
        ezLog::Error("Expected '>' after closing tag name");
        return EZ_FAILURE;
      }
      ++ctx.m_pCurrent;
      return EZ_SUCCESS;
    }

    // Child element?
    if (*ctx.m_pCurrent == '<')
    {
      // Could be comment
      if (ctx.m_pCurrent + 4 <= ctx.m_pEnd && ctx.m_pCurrent[1] == '!' && ctx.m_pCurrent[2] == '-' && ctx.m_pCurrent[3] == '-')
      {
        ++ctx.m_pCurrent; // move past '<'
        SkipComment(ctx);
        continue;
      }

      EZ_SUCCEED_OR_RETURN(ParseElement(ctx, elementId));
      continue;
    }

    // Text content
    EZ_SUCCEED_OR_RETURN(ParseTextContent(ctx, elementId));
  }

  ezLog::Error("Unexpected end of XML while parsing element '{}'", tagNameStr);
  return EZ_FAILURE;
}

ezResult ezRazorXmlParser::ParseAttributes(ParseContext& ctx, ezRazorElementId element)
{
  ezRazorElement* pElement = ctx.m_pDocument->GetElement(element);
  if (pElement == nullptr)
    return EZ_FAILURE;

  while (ctx.m_pCurrent < ctx.m_pEnd)
  {
    SkipWhitespace(ctx);

    if (ctx.m_pCurrent >= ctx.m_pEnd)
      break;

    // End of attributes?
    if (*ctx.m_pCurrent == '>' || *ctx.m_pCurrent == '/')
      break;

    // Read attribute name
    ezStringView attrName = ReadIdentifier(ctx);
    if (attrName.IsEmpty())
    {
      ezLog::Error("Expected attribute name");
      return EZ_FAILURE;
    }

    SkipWhitespace(ctx);

    // Expect '='
    if (ctx.m_pCurrent >= ctx.m_pEnd || *ctx.m_pCurrent != '=')
    {
      ezLog::Error("Expected '=' after attribute name");
      return EZ_FAILURE;
    }
    ++ctx.m_pCurrent;

    SkipWhitespace(ctx);

    // Read quoted value
    ezStringBuilder attrValue;
    EZ_SUCCEED_OR_RETURN(ReadQuotedString(ctx, attrValue));

    // Store attribute
    ezStringBuilder attrNameStr(attrName);
    if (attrNameStr == "id")
    {
      pElement->m_sId.Assign(attrValue);
    }
    else if (attrNameStr == "class")
    {
      // Parse space-separated class names
      ezStringBuilder remaining = attrValue;
      while (!remaining.IsEmpty())
      {
        const char* pStart = remaining.GetData();
        while (*pStart && IsWhitespace(*pStart))
          ++pStart;
        if (!*pStart)
          break;

        const char* pEnd = pStart;
        while (*pEnd && !IsWhitespace(*pEnd))
          ++pEnd;

        if (pEnd > pStart)
        {
          ezHashedString className;
          className.Assign(ezStringView(pStart, pEnd));
          pElement->m_Classes.PushBack(className);
        }

        remaining = ezStringView(pEnd, remaining.GetData() + remaining.GetElementCount());
      }
    }
    else
    {
      // Generic attributes stored in element (would need an attribute map if needed)
      // For now, skip unknown attributes
    }
  }

  return EZ_SUCCESS;
}

ezResult ezRazorXmlParser::ParseTextContent(ParseContext& ctx, ezRazorElementId parent)
{
  const char* pStart = ctx.m_pCurrent;

  while (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent != '<')
  {
    ++ctx.m_pCurrent;
  }

  ezStringView textContent(pStart, ctx.m_pCurrent);

  // Handle whitespace stripping
  if (ctx.m_bStripWhitespace)
  {
    // Trim whitespace
    const char* pTrimStart = textContent.GetStartPointer();
    const char* pTrimEnd = textContent.GetEndPointer();

    while (pTrimStart < pTrimEnd && IsWhitespace(*pTrimStart))
      ++pTrimStart;
    while (pTrimEnd > pTrimStart && IsWhitespace(*(pTrimEnd - 1)))
      --pTrimEnd;

    if (pTrimStart >= pTrimEnd)
      return EZ_SUCCESS; // Empty text after trimming, skip

    textContent = ezStringView(pTrimStart, pTrimEnd);
  }

  if (textContent.IsEmpty())
    return EZ_SUCCESS;

  // Create a #text element for the text content
  ezRazorElementId textId = ctx.m_pDocument->CreateElement("#text");
  ezRazorElement* pTextElement = ctx.m_pDocument->GetElement(textId);
  if (pTextElement != nullptr)
  {
    pTextElement->m_sTextContent = ezStringBuilder(textContent);
    ctx.m_pDocument->AppendChild(parent, textId);
  }

  return EZ_SUCCESS;
}

void ezRazorXmlParser::SkipWhitespace(ParseContext& ctx)
{
  while (ctx.m_pCurrent < ctx.m_pEnd && IsWhitespace(*ctx.m_pCurrent))
  {
    ++ctx.m_pCurrent;
  }
}

bool ezRazorXmlParser::SkipComment(ParseContext& ctx)
{
  // Check for <!-- ... -->
  if (ctx.m_pCurrent + 3 <= ctx.m_pEnd &&
      ctx.m_pCurrent[0] == '!' && ctx.m_pCurrent[1] == '-' && ctx.m_pCurrent[2] == '-')
  {
    ctx.m_pCurrent += 3;
    while (ctx.m_pCurrent + 3 <= ctx.m_pEnd)
    {
      if (ctx.m_pCurrent[0] == '-' && ctx.m_pCurrent[1] == '-' && ctx.m_pCurrent[2] == '>')
      {
        ctx.m_pCurrent += 3;
        return true;
      }
      ++ctx.m_pCurrent;
    }
    // Unterminated comment - consume rest
    ctx.m_pCurrent = ctx.m_pEnd;
    return true;
  }
  return false;
}

ezStringView ezRazorXmlParser::ReadIdentifier(ParseContext& ctx)
{
  const char* pStart = ctx.m_pCurrent;

  if (ctx.m_pCurrent >= ctx.m_pEnd || !IsNameStartChar(*ctx.m_pCurrent))
    return ezStringView();

  ++ctx.m_pCurrent;
  while (ctx.m_pCurrent < ctx.m_pEnd && IsNameChar(*ctx.m_pCurrent))
  {
    ++ctx.m_pCurrent;
  }

  return ezStringView(pStart, ctx.m_pCurrent);
}

ezResult ezRazorXmlParser::ReadQuotedString(ParseContext& ctx, ezStringBuilder& out_value)
{
  if (ctx.m_pCurrent >= ctx.m_pEnd)
    return EZ_FAILURE;

  char quote = *ctx.m_pCurrent;
  if (quote != '"' && quote != '\'')
  {
    ezLog::Error("Expected quoted string");
    return EZ_FAILURE;
  }

  ++ctx.m_pCurrent;
  const char* pStart = ctx.m_pCurrent;

  while (ctx.m_pCurrent < ctx.m_pEnd && *ctx.m_pCurrent != quote)
  {
    ++ctx.m_pCurrent;
  }

  out_value = ezStringView(pStart, ctx.m_pCurrent);

  if (ctx.m_pCurrent < ctx.m_pEnd)
    ++ctx.m_pCurrent; // Skip closing quote

  return EZ_SUCCESS;
}

bool ezRazorXmlParser::IsWhitespace(char c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool ezRazorXmlParser::IsNameStartChar(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '#';
}

bool ezRazorXmlParser::IsNameChar(char c)
{
  return IsNameStartChar(c) || (c >= '0' && c <= '9') || c == '-' || c == '.';
}

EZ_STATICLINK_FILE(RazorCore, RazorCore_Parser_RazorXmlParser);
