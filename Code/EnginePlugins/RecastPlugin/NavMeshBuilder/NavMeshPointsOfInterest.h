#pragma once

#include <Foundation/Time/Time.h>
#include <RecastPlugin/Components/PointOfInterestGraph.h>
#include <RecastPlugin/RecastPluginDLL.h>

struct rcPolyMesh;

struct EZ_RECASTPLUGIN_DLL ezNavMeshPointsOfInterest
{
  ezVec3 m_vFloorPosition;
  ezUInt32 m_uiVisibleMarker = 0;
};

class EZ_RECASTPLUGIN_DLL ezNavMeshPointOfInterestGraph
{
public:
  ezNavMeshPointOfInterestGraph();
  ~ezNavMeshPointOfInterestGraph();

  void ExtractInterestPointsFromMesh(const rcPolyMesh& mesh, bool bReinitialize = true /* bad interface design */);

  ezUInt32 GetCheckVisibilityTimeStamp() const { return m_uiCheckVisibilityTimeStamp; }
  void IncreaseCheckVisibiblityTimeStamp(ezTime now);

  ezPointOfInterestGraph<ezNavMeshPointsOfInterest>& GetGraph() { return m_NavMeshPointGraph; }
  const ezPointOfInterestGraph<ezNavMeshPointsOfInterest>& GetGraph() const { return m_NavMeshPointGraph; }

protected:
  ezTime m_LastTimeStampStep;
  ezUInt32 m_uiCheckVisibilityTimeStamp = 100;
  ezPointOfInterestGraph<ezNavMeshPointsOfInterest> m_NavMeshPointGraph;
};
