#include <HairImporter/HairImporterPCH.h>

#include <HairImporter/HairImporter.h>
#include <HairImporter/AlembicHairImporter.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>

namespace ezHairImporter
{
  ezResult Import(const ImportOptions& options, ImportResult& out_result,
    ezLogInterface* pLogInterface, ezProgress* pProgress)
  {
    if (options.m_sSourceFile.IsEmpty())
    {
      ezLog::Error(pLogInterface, "No source file specified for hair import.");
      return EZ_FAILURE;
    }

    if (!ezOSFile::ExistsFile(options.m_sSourceFile))
    {
      ezLog::Error(pLogInterface, "Hair source file does not exist: '{}'", options.m_sSourceFile);
      return EZ_FAILURE;
    }

    ezStringBuilder ext = ezPathUtils::GetFileExtension(options.m_sSourceFile);
    ext.ToLower();

    if (ext == "abc")
    {
      return ImportAlembic(options, out_result, pLogInterface, pProgress);
    }

    ezLog::Error(pLogInterface, "Unsupported hair file format: '.{}'", ext);
    return EZ_FAILURE;
  }

} // namespace ezHairImporter
