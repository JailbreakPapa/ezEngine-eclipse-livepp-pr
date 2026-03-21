#include <HairImporter/HairImporterPCH.h>

#include <HairImporter/AlembicHairImporter.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Math/BoundingBoxSphere.h>

#ifdef BUILDSYSTEM_ENABLE_ALEMBIC_SUPPORT

#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreOgawa/All.h>

namespace Abc = Alembic::Abc;
namespace AbcGeom = Alembic::AbcGeom;

namespace ezHairImporter
{
  // Blender exports Z-up right-handed; ezEngine is Y-up left-handed.
  static ezVec3 ConvertCoordinate(float x, float y, float z)
  {
    return ezVec3(x, z, -y);
  }

  static void TraverseCurves(const Abc::IObject& obj, const ImportOptions& options,
    ImportResult& out_result, ezLogInterface* pLogInterface)
  {
    for (size_t i = 0; i < obj.getNumChildren(); ++i)
    {
      Abc::IObject child = obj.getChild(i);

      if (AbcGeom::ICurves::matches(child.getHeader()))
      {
        AbcGeom::ICurves curvesObj(child, Abc::kWrapExisting);
        AbcGeom::ICurvesSchema& schema = curvesObj.getSchema();
        AbcGeom::ICurvesSchema::Sample sample;
        schema.get(sample);

        const Abc::P3fArraySamplePtr positions = sample.getPositions();
        const Abc::Int32ArraySamplePtr numVertices = sample.getCurvesNumVertices();

        if (!positions || !numVertices || numVertices->size() == 0)
          continue;

        ezUInt32 numStrands = static_cast<ezUInt32>(numVertices->size());
        if (options.m_uiMaxStrandsPerGroup > 0 && numStrands > options.m_uiMaxStrandsPerGroup)
          numStrands = options.m_uiMaxStrandsPerGroup;

        // Find the max point count per strand (all strands in a group get padded to the same count)
        ezUInt32 maxPointsPerStrand = 0;
        for (ezUInt32 s = 0; s < numStrands; ++s)
        {
          ezUInt32 pc = static_cast<ezUInt32>((*numVertices)[s]);
          maxPointsPerStrand = ezMath::Max(maxPointsPerStrand, pc);
        }

        if (maxPointsPerStrand < 2)
          continue;

        auto& group = out_result.m_Descriptor.m_StrandGroups.ExpandAndGetRef();
        group.m_uiNumStrands = numStrands;
        group.m_uiPointsPerStrand = maxPointsPerStrand;
        group.m_Points.SetCountUninitialized(numStrands * maxPointsPerStrand);
        group.m_Widths.SetCountUninitialized(numStrands * maxPointsPerStrand);
        group.m_RootUVs.SetCountUninitialized(numStrands);
        group.m_Randoms.SetCountUninitialized(numStrands);

        // Try to read widths
        AbcGeom::IFloatGeomParam widthParam = schema.getWidthsParam();
        bool hasWidths = widthParam.valid();
        Abc::FloatArraySamplePtr widthSamples;
        if (hasWidths)
        {
          AbcGeom::IFloatGeomParam::Sample widthSample;
          widthParam.getExpanded(widthSample);
          widthSamples = widthSample.getVals();
        }

        ezBoundingBox bbox = ezBoundingBox::MakeInvalid();
        ezUInt32 srcPosIdx = 0;

        for (ezUInt32 s = 0; s < numStrands; ++s)
        {
          ezUInt32 pointCount = static_cast<ezUInt32>((*numVertices)[s]);
          ezUInt32 dstBase = s * maxPointsPerStrand;

          for (ezUInt32 p = 0; p < maxPointsPerStrand; ++p)
          {
            if (p < pointCount && srcPosIdx < positions->size())
            {
              const auto& pos = (*positions)[srcPosIdx];
              group.m_Points[dstBase + p] = ConvertCoordinate(pos.x, pos.y, pos.z);

              float width = options.m_fDefaultWidth;
              if (hasWidths && srcPosIdx < widthSamples->size())
                width = (*widthSamples)[srcPosIdx];

              // Apply taper: linearly interpolate width toward tip
              float t = static_cast<float>(p) / static_cast<float>(maxPointsPerStrand - 1);
              float taper = ezMath::Lerp(1.0f, options.m_fTipWidthFraction, t);
              group.m_Widths[dstBase + p] = width * options.m_fGlobalWidthScale * taper;

              bbox.ExpandToInclude(group.m_Points[dstBase + p]);
              srcPosIdx++;
            }
            else
            {
              // Pad shorter strands by repeating last point
              group.m_Points[dstBase + p] = group.m_Points[dstBase + pointCount - 1];
              group.m_Widths[dstBase + p] = 0.0f; // zero width for padding
            }
          }

          // Root UV: generate from root position projected onto XZ plane
          if (options.m_bGenerateUVs)
          {
            ezVec3 root = group.m_Points[dstBase];
            group.m_RootUVs[s] = ezVec2(root.x * 0.5f + 0.5f, root.z * 0.5f + 0.5f);
          }
          else
          {
            group.m_RootUVs[s] = ezVec2(0.0f, 0.0f);
          }

          // Per-strand random: deterministic hash from strand index
          ezUInt32 hash = s;
          hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
          hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
          hash = (hash >> 16) ^ hash;
          group.m_Randoms[s] = static_cast<float>(hash & 0xFFFFFF) / static_cast<float>(0xFFFFFF);
        }

        // Skip remaining positions if we capped strand count
        if (options.m_uiMaxStrandsPerGroup > 0 && numStrands < static_cast<ezUInt32>(numVertices->size()))
        {
          for (ezUInt32 s = numStrands; s < static_cast<ezUInt32>(numVertices->size()); ++s)
            srcPosIdx += static_cast<ezUInt32>((*numVertices)[s]);
        }

        out_result.m_GroupNames.PushBack(child.getName().c_str());

        ezLog::Info(pLogInterface, "Imported hair group '{}': {} strands, {} points/strand",
          child.getName().c_str(), numStrands, maxPointsPerStrand);
      }

      // Recurse into children
      TraverseCurves(child, options, out_result, pLogInterface);
    }
  }

  ezResult ImportAlembic(const ImportOptions& options, ImportResult& out_result,
    ezLogInterface* pLogInterface, ezProgress* pProgress)
  {
    try
    {
      Abc::IArchive archive(Alembic::AbcCoreOgawa::ReadArchive(), options.m_sSourceFile.GetData());

      if (!archive.valid())
      {
        ezLog::Error(pLogInterface, "Failed to open Alembic archive: '{}'", options.m_sSourceFile);
        return EZ_FAILURE;
      }

      Abc::IObject root = archive.getTop();
      TraverseCurves(root, options, out_result, pLogInterface);

      if (out_result.m_Descriptor.m_StrandGroups.IsEmpty())
      {
        ezLog::Error(pLogInterface, "No hair curves found in Alembic archive: '{}'", options.m_sSourceFile);
        return EZ_FAILURE;
      }

      // Compute combined bounds
      ezBoundingBox combinedBox = ezBoundingBox::MakeInvalid();
      for (const auto& group : out_result.m_Descriptor.m_StrandGroups)
      {
        for (const auto& pt : group.m_Points)
          combinedBox.ExpandToInclude(pt);
      }
      out_result.m_Descriptor.m_Bounds = ezBoundingBoxSphere::MakeFromBox(combinedBox);

      ezLog::Success(pLogInterface, "Imported {} hair strand group(s) from '{}'",
        out_result.m_Descriptor.m_StrandGroups.GetCount(), options.m_sSourceFile);

      return EZ_SUCCESS;
    }
    catch (const std::exception& e)
    {
      ezLog::Error(pLogInterface, "Alembic import error: {}", e.what());
      return EZ_FAILURE;
    }
  }

} // namespace ezHairImporter

#else // !BUILDSYSTEM_ENABLE_ALEMBIC_SUPPORT

namespace ezHairImporter
{
  ezResult ImportAlembic(const ImportOptions& options, ImportResult& out_result,
    ezLogInterface* pLogInterface, ezProgress* pProgress)
  {
    ezLog::Error(pLogInterface,
      "Alembic support is not enabled. Enable EZ_3RDPARTY_ALEMBIC_SUPPORT in the CMake configuration to import .abc files.");
    return EZ_FAILURE;
  }

} // namespace ezHairImporter

#endif
