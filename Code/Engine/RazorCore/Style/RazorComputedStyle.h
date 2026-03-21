#pragma once

#include <RazorCore/RazorCoreDLL.h>

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Math/Color.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/Enum.h>
#include <Foundation/Types/Variant.h>

/// CSS-like length/size value with unit information.
struct EZ_RAZORCORE_DLL ezRazorStyleValue
{
  EZ_DECLARE_POD_TYPE();

  enum class Unit : ezUInt8
  {
    Undefined,
    Auto,
    Pixels,
    Percent,
    Em,
    Rem,
    Vh,        ///< Viewport height percentage
    Vw,        ///< Viewport width percentage
    Vmin,      ///< Smaller of vw or vh
    Vmax,      ///< Larger of vw or vh
    Ch,        ///< Character width (0)
    Ex,        ///< x-height
    Cm,        ///< Centimeters
    Mm,        ///< Millimeters
    In,        ///< Inches
    Pt,        ///< Points (1/72 inch)
    Pc,        ///< Picas (12 points)
  };

  float m_fValue = 0.0f;
  Unit m_Unit = Unit::Undefined;

  static ezRazorStyleValue MakeUndefined() { return {0.0f, Unit::Undefined}; }
  static ezRazorStyleValue MakeAuto() { return {0.0f, Unit::Auto}; }
  static ezRazorStyleValue MakePixels(float fPx) { return {fPx, Unit::Pixels}; }
  static ezRazorStyleValue MakePercent(float fPct) { return {fPct, Unit::Percent}; }
  static ezRazorStyleValue MakeEm(float fEm) { return {fEm, Unit::Em}; }
  static ezRazorStyleValue MakeRem(float fRem) { return {fRem, Unit::Rem}; }
  static ezRazorStyleValue MakeVh(float fVh) { return {fVh, Unit::Vh}; }
  static ezRazorStyleValue MakeVw(float fVw) { return {fVw, Unit::Vw}; }

  bool IsUndefined() const { return m_Unit == Unit::Undefined; }
};

/// CSS calc() expression node types.
struct ezRazorCalcOp
{
  using StorageType = ezUInt8;

  enum Enum : ezUInt8
  {
    Value,    ///< Leaf node: a single value
    Add,      ///< +
    Subtract, ///< -
    Multiply, ///< *
    Divide,   ///< /
  };
};

/// A CSS value that can hold simple values, colors, calc expressions, or var() references.
struct EZ_RAZORCORE_DLL ezRazorCssValue
{
  enum class Type : ezUInt8
  {
    None,
    Length,       ///< A length value with unit (px, %, em, etc.)
    Color,        ///< A color value
    Keyword,      ///< A keyword like "auto", "inherit", "flex-start"
    String,       ///< A quoted string
    Url,          ///< url() function
    Calc,         ///< calc() expression (stored as string for evaluation)
    Var,          ///< var(--custom-property) reference
    Number,       ///< A unitless number
    Integer,      ///< An integer number
    Angle,        ///< An angle (deg, rad, grad, turn)
    Time,         ///< A time duration (s, ms)
    Function,     ///< Generic CSS function
  };

  Type m_Type = Type::None;
  ezRazorStyleValue m_LengthValue;
  ezColor m_ColorValue;
  ezString m_sStringValue;       ///< For Keyword, String, Url, Calc, Var, Function
  float m_fNumberValue = 0.0f;

  static ezRazorCssValue MakeNone() { return {}; }
  static ezRazorCssValue MakeLength(const ezRazorStyleValue& v) { ezRazorCssValue r; r.m_Type = Type::Length; r.m_LengthValue = v; return r; }
  static ezRazorCssValue MakeColor(const ezColor& c) { ezRazorCssValue r; r.m_Type = Type::Color; r.m_ColorValue = c; return r; }
  static ezRazorCssValue MakeKeyword(ezStringView s) { ezRazorCssValue r; r.m_Type = Type::Keyword; r.m_sStringValue = s; return r; }
  static ezRazorCssValue MakeNumber(float f) { ezRazorCssValue r; r.m_Type = Type::Number; r.m_fNumberValue = f; return r; }
};

/// A CSS property value that may have multiple parts (e.g., "10px 20px 10px 20px" for margin).
struct EZ_RAZORCORE_DLL ezRazorCssPropertyValue
{
  ezHybridArray<ezRazorCssValue, 4> m_Values;

  bool IsEmpty() const { return m_Values.IsEmpty(); }
  ezUInt32 GetCount() const { return m_Values.GetCount(); }
  const ezRazorCssValue& operator[](ezUInt32 i) const { return m_Values[i]; }
};

/// Flex direction for Yoga-backed layout.
struct ezRazorFlexDirection
{
  using StorageType = ezUInt8;

  enum Enum : ezUInt8
  {
    Row,
    Column,
    RowReverse,
    ColumnReverse,

    Default = Row
  };
};

/// Alignment along the cross axis.
struct ezRazorAlignItems
{
  using StorageType = ezUInt8;

  enum Enum : ezUInt8
  {
    Stretch,
    FlexStart,
    FlexEnd,
    Center,
    Baseline,

    Default = Stretch
  };
};

/// Content justification along the main axis.
struct ezRazorJustifyContent
{
  using StorageType = ezUInt8;

  enum Enum : ezUInt8
  {
    FlexStart,
    FlexEnd,
    Center,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly,

    Default = FlexStart
  };
};

/// Flex wrap mode.
struct ezRazorFlexWrap
{
  using StorageType = ezUInt8;

  enum Enum : ezUInt8
  {
    NoWrap,
    Wrap,
    WrapReverse,

    Default = NoWrap
  };
};

/// Overflow behavior.
struct ezRazorOverflow
{
  using StorageType = ezUInt8;

  enum Enum : ezUInt8
  {
    Visible,
    Hidden,
    Scroll,

    Default = Visible
  };
};

/// Position type (static, relative, or absolute).
struct ezRazorPosition
{
  using StorageType = ezUInt8;

  enum Enum : ezUInt8
  {
    Static,
    Relative,
    Absolute,

    Default = Static
  };
};

/// Display mode.
struct ezRazorDisplay
{
  using StorageType = ezUInt8;

  enum Enum : ezUInt8
  {
    Flex,
    None,

    Default = Flex
  };
};

/// The resolved final style for a single element after cascade and inheritance.
///
/// Closely aligned with what Yoga can consume, plus visual properties
/// that Razor handles directly (color, background, border rendering, opacity, etc.).
struct EZ_RAZORCORE_DLL ezRazorComputedStyle
{
  // --- Layout (Yoga-mapped) ---
  ezEnum<ezRazorDisplay> m_Display;
  ezEnum<ezRazorPosition> m_Position;
  ezEnum<ezRazorFlexDirection> m_FlexDirection;
  ezEnum<ezRazorFlexWrap> m_FlexWrap;
  ezEnum<ezRazorJustifyContent> m_JustifyContent;
  ezEnum<ezRazorAlignItems> m_AlignItems;
  ezEnum<ezRazorOverflow> m_Overflow;

  float m_fFlexGrow = 0.0f;
  float m_fFlexShrink = 1.0f;
  ezRazorStyleValue m_FlexBasis = ezRazorStyleValue::MakeAuto();

  ezRazorStyleValue m_Width = ezRazorStyleValue::MakeAuto();
  ezRazorStyleValue m_Height = ezRazorStyleValue::MakeAuto();
  ezRazorStyleValue m_MinWidth = ezRazorStyleValue::MakeUndefined();
  ezRazorStyleValue m_MinHeight = ezRazorStyleValue::MakeUndefined();
  ezRazorStyleValue m_MaxWidth = ezRazorStyleValue::MakeUndefined();
  ezRazorStyleValue m_MaxHeight = ezRazorStyleValue::MakeUndefined();

  ezRazorStyleValue m_Margin[4];   ///< top, right, bottom, left
  ezRazorStyleValue m_Padding[4];  ///< top, right, bottom, left
  ezRazorStyleValue m_BorderWidth[4]; ///< top, right, bottom, left

  ezRazorStyleValue m_Gap = ezRazorStyleValue::MakePixels(0);

  // --- Absolute positioning offsets ---
  ezRazorStyleValue m_Top = ezRazorStyleValue::MakeUndefined();
  ezRazorStyleValue m_Right = ezRazorStyleValue::MakeUndefined();
  ezRazorStyleValue m_Bottom = ezRazorStyleValue::MakeUndefined();
  ezRazorStyleValue m_Left = ezRazorStyleValue::MakeUndefined();

  // --- Visual (Razor-owned, not sent to Yoga) ---
  ezColor m_BackgroundColor = ezColor::MakeZero();
  ezColor m_TextColor = ezColor::White;
  ezColor m_BorderColor = ezColor::MakeZero();

  float m_fBorderRadius[4] = {0, 0, 0, 0}; ///< top-left, top-right, bottom-right, bottom-left in pixels
  float m_fOpacity = 1.0f;
  float m_fFontSize = 16.0f;

  ezInt32 m_iZIndex = 0;
};
