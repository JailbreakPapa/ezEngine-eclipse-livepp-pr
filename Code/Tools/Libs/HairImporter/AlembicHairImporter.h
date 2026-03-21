#pragma once

#include <HairImporter/HairImporter.h>

namespace ezHairImporter
{
  /// Alembic (.abc) backend for hair strand import.
  ///
  /// Reads ICurves objects from an Alembic archive using the Ogawa backend.
  /// Handles coordinate system conversion from Blender (Z-up, right-handed) to
  /// ezEngine (Y-up, left-handed).
  ///
  /// Requires the Alembic third-party library. When BUILDSYSTEM_ENABLE_ALEMBIC_SUPPORT
  /// is not defined, Import() returns EZ_FAILURE.
  EZ_HAIRIMPORTER_DLL ezResult ImportAlembic(const ImportOptions& options, ImportResult& out_result,
    ezLogInterface* pLogInterface = nullptr, ezProgress* pProgress = nullptr);

} // namespace ezHairImporter
