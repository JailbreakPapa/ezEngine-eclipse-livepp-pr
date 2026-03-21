#pragma once

#include <HairImporter/HairImporterDLL.h>

#include <Foundation/Types/Status.h>
#include <Foundation/Types/UniquePtr.h>
#include <RendererCore/Meshes/HairStrandResource.h>

class ezLogInterface;
class ezProgress;

namespace ezHairImporter
{
  struct EZ_HAIRIMPORTER_DLL ImportOptions
  {
    ezString m_sSourceFile;

    /// Global multiplier applied to all strand widths.
    float m_fGlobalWidthScale = 1.0f;

    /// Width used when the source file has no per-point width data (meters).
    float m_fDefaultWidth = 0.0005f;

    /// Tip width as a fraction of root width, for generating a taper when widths are uniform.
    float m_fTipWidthFraction = 0.1f;

    /// Maximum number of strands to import per group. 0 = unlimited.
    ezUInt32 m_uiMaxStrandsPerGroup = 0;

    /// Whether to auto-generate root UVs from strand root positions when the source has none.
    bool m_bGenerateUVs = true;
  };

  struct EZ_HAIRIMPORTER_DLL ImportResult
  {
    ezHairStrandResourceDescriptor m_Descriptor;
    ezDynamicArray<ezString> m_GroupNames;
  };

  /// Imports hair strand data from a source file (Alembic .abc).
  ///
  /// Returns EZ_SUCCESS if import succeeded, in which case result is populated.
  /// On failure, an error message is written to the log interface.
  EZ_HAIRIMPORTER_DLL ezResult Import(const ImportOptions& options, ImportResult& out_result,
    ezLogInterface* pLogInterface = nullptr, ezProgress* pProgress = nullptr);

} // namespace ezHairImporter
