#pragma once

#include <RazorCore/RazorCoreDLL.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Strings/String.h>

class ezRazorDocument;
struct ezRazorElementId;

/// Parses Razor XML markup into a document.
///
/// Razor XML is a simplified subset of XML where element tag names map to UI components
/// and attributes map to properties. The parser creates DOM elements for each XML element.
///
/// Example:
/// ```xml
/// <Root>
///   <Box class="container">
///     <Text id="title">Hello World</Text>
///   </Box>
/// </Root>
/// ```
class EZ_RAZORCORE_DLL ezRazorXmlParser
{
public:
  struct ParseOptions
  {
    bool m_bStripWhitespace = true; ///< Ignore whitespace-only text nodes.
  };

  /// Parses XML content and builds the document tree.
  /// Returns EZ_FAILURE if the XML is malformed.
  static ezResult Parse(ezStringView sXml, ezRazorDocument& out_document, const ParseOptions& options = {});

private:
  struct ParseContext
  {
    ezRazorDocument* m_pDocument = nullptr;
    const char* m_pCurrent = nullptr;
    const char* m_pEnd = nullptr;
    bool m_bStripWhitespace = true;
  };

  static ezResult ParseElement(ParseContext& ctx, ezRazorElementId parent);
  static ezResult ParseAttributes(ParseContext& ctx, ezRazorElementId element);
  static ezResult ParseTextContent(ParseContext& ctx, ezRazorElementId parent);
  static void SkipWhitespace(ParseContext& ctx);
  static bool SkipComment(ParseContext& ctx);
  static ezStringView ReadIdentifier(ParseContext& ctx);
  static ezResult ReadQuotedString(ParseContext& ctx, ezStringBuilder& out_value);
  static bool IsWhitespace(char c);
  static bool IsNameChar(char c);
  static bool IsNameStartChar(char c);
};
