#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/String.h>

class ezHairStrandAssetProperties : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezHairStrandAssetProperties, ezReflectedClass);

public:
  ezHairStrandAssetProperties();

  /// Source .abc file containing hair curve data.
  ezString m_sSourceFile;

  /// Global multiplier applied to all strand widths.
  float m_fGlobalWidthScale = 1.0f;

  /// Width used when the source file has no per-point width data (meters).
  float m_fDefaultWidth = 0.0005f;

  /// Tip width as a fraction of root width.
  float m_fTipWidthFraction = 0.1f;

  /// Maximum number of strands to import per group. 0 = unlimited.
  ezUInt32 m_uiMaxStrandsPerGroup = 0;

  /// Whether to auto-generate root UVs from strand root positions.
  bool m_bGenerateUVs = true;
};
