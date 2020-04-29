
#pragma once
#include "InputGeom.h"
#include "MeshLoaderObj.h"
#include "Recast/Recast.h"

class TileMeshExample
{
public:
	TileMeshExample()
	{
		m_geom = new InputGeom;
	}
	~TileMeshExample()
	{
		delete m_geom;
	}

	bool Init(const char* ObjPath);

	class InputGeom* m_geom;
	class dtNavMesh* m_navMesh;
	class dtNavMeshQuery* m_navQuery;



	bool m_keepInterResults;
	float m_totalBuildTimeMs;

	unsigned char* m_triareas;
	rcHeightfield* m_solid;
	rcCompactHeightfield* m_chf;
	rcContourSet* m_cset;
	rcPolyMesh* m_pmesh;
	rcConfig m_cfg;
	rcPolyMeshDetail* m_dmesh;

};

bool TileMeshExample::Init(const char* ObjPath)
{
	this->m_geom->loadMesh(NULL, ObjPath);

	BuildSettings TileMeshBuildSettings;
	TileMeshBuildSettings.cellSize = 19.f;
	TileMeshBuildSettings.cellHeight = 10.f;
	TileMeshBuildSettings.agentHeight = 144.f;
	TileMeshBuildSettings.agentRadius = 35.f;
	TileMeshBuildSettings.agentMaxClimb = 35;
	TileMeshBuildSettings.agentMaxSlope = 44.f;
	TileMeshBuildSettings.regionMinSize = 0;
	TileMeshBuildSettings.regionMergeSize = 21;
	TileMeshBuildSettings.edgeMaxLen = 63;
	TileMeshBuildSettings.edgeMaxError = 1.0;
	TileMeshBuildSettings.vertsPerPoly = 6;
	TileMeshBuildSettings.detailSampleDist = 600.f;
	TileMeshBuildSettings.detailSampleMaxError = 1.f;
	// TileMeshBuildSettings.partitionType = m_partitionType;

	this->m_geom->m_hasBuildSettings = true;
	this->m_geom->m_buildSettings = TileMeshBuildSettings;

	return false;
}
