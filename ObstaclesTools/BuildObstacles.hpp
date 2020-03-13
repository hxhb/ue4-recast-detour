#pragma once
#include "../Detour/DetourNavMesh.h"
#include "../Detour/DetourNavMeshQuery.h"

#include "MeshLoaderObj.h"
#include "ChunkyTriMesh.h"

class FMeshManager
{
public:
	FMeshManager() :m_mesh(0) {}
	bool LoadMesh(const char* filepath)
	{
		memset(&m_data, 0, sizeof(rcAdditionalData));

		m_mesh = new rcMeshLoaderObj;
		if (!m_mesh)
		{
			printf("ERROR:loadMesh: Out of memory 'm_mesh'.");
			return false;
		}
		if (!m_mesh->load(filepath, &m_data))
		{
			printf("ERROR:buildTiledNavigation: Could not load '%s'", filepath);
			return false;
		}

		if (!m_mesh->usedExtendedData())
		{
			rcCalcBounds(m_mesh->getVerts(), m_mesh->getVertCount(), m_meshBMin, m_meshBMax);
		}
		else
		{
			memcpy(m_meshBMin, m_data.bmin, sizeof(float) * 3);
			memcpy(m_meshBMax, m_data.bmax, sizeof(float) * 3);
		}
		m_chunkyMesh = new rcChunkyTriMesh;
		if (!m_chunkyMesh)
		{
			printf("ERROR:buildTiledNavigation: Out of memory 'm_chunkyMesh'.");
			return false;
		}
		if (!rcCreateChunkyTriMesh(m_mesh->getVerts(), m_mesh->getTris(), m_mesh->getTriCount(), 256, m_chunkyMesh))
		{
			printf("ERROR:buildTiledNavigation: Failed to build chunky mesh.");
			return false;
		}

		return true;
	}
	/// Method to return static mesh data.
	inline const rcMeshLoaderObj* getMesh() const { return m_mesh; }
	inline const float* getMeshBoundsMin() const { return m_meshBMin; }
	inline const float* getMeshBoundsMax() const { return m_meshBMax; }


private:
	rcChunkyTriMesh* m_chunkyMesh;
	rcMeshLoaderObj* m_mesh;
	rcAdditionalData m_data;
	float m_meshBMin[3], m_meshBMax[3];
};
struct FObstaclesTool
{
public:
	FObstaclesTool()
	{
		m_geom = new FMeshManager;
	}
	~FObstaclesTool()
	{
		delete m_geom;

	}
	bool BuildObstacles(const char* filepath);
protected:
	class FMeshManager* m_geom;
	class dtNavMesh* m_navMesh;
	class dtNavMeshQuery* m_navQuery;
	// class dtCrowd* m_crowd;

	unsigned char m_navMeshDrawFlags;

	float m_cellSize;
	float m_cellHeight;
	float m_agentHeight;
	float m_agentRadius;
	float m_agentMaxClimb;
	float m_agentMaxSlope;
	float m_regionMinSize;
	float m_regionMergeSize;
	float m_regionChunkSize;
	int m_regionPartitioning;
	float m_edgeMaxLen;
	float m_edgeMaxError;
	float m_vertsPerPoly;
	float m_detailSampleDist;
	float m_detailSampleMaxError;
};


bool FObstaclesTool::BuildObstacles(const char* filepath)
{
	if (!m_geom->LoadMesh(filepath))
	{
		printf("Load Obj:%s is Faild!", filepath);
	}
	// Init cache
	const float* bmin = m_geom->getMeshBoundsMin();
	const float* bmax = m_geom->getMeshBoundsMax();
	int gw = 0, gh = 0;
	rcCalcGridSize(bmin, bmax, m_cellSize, &gw, &gh);
	//const int ts = (int)m_tileSize;
	//const int tw = (gw + ts - 1) / ts;
	//const int th = (gh + ts - 1) / ts;


	return true;
}