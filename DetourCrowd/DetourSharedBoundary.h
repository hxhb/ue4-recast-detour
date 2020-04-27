// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

// #include "CoreMinimal.h"
#include "Detour/DetourNavMeshQuery.h"
#include <vector>
#include <set>

template<typename T>
using TArray = std::vector<T>;

template<typename T>
using TSet = std::set<T>;

struct dtSharedBoundaryEdge
{
	float v0[3];
	float v1[3];
	dtPolyRef p0;
	dtPolyRef p1;
};

struct dtSharedBoundaryData
{
	float Center[3];
	float Radius;
	float AccessTime;
	dtQueryFilter* Filter;
	/*uint8*/unsigned char SingleAreaId;
	
	TArray<dtSharedBoundaryEdge> Edges;
	TSet<dtPolyRef> Polys;

	dtSharedBoundaryData() : Filter(nullptr) {}
};

class dtSharedBoundary
{
public:
	TSparseArray<dtSharedBoundaryData> Data;
	dtQueryFilter SingleAreaFilter;
	float CurrentTime;
	float NextClearTime;

	void Initialize();
	void Tick(float DeltaTime);

	/*int32*/int FindData(float* Center, float Radius, dtPolyRef ReqPoly, dtQueryFilter* NavFilter) const;
	/*int32*/int FindData(float* Center, float Radius, dtPolyRef ReqPoly, /*uint8*/unsigned char SingleAreaId) const;

	/*int32*/int CacheData(float* Center, float Radius, dtPolyRef CenterPoly, dtNavMeshQuery* NavQuery, dtQueryFilter* NavFilter);
	/*int32*/int CacheData(float* Center, float Radius, dtPolyRef CenterPoly, dtNavMeshQuery* NavQuery, /*uint8*/unsigned char SingleAreaId);

	void FindEdges(dtSharedBoundaryData& Data, dtPolyRef CenterPoly, dtNavMeshQuery* NavQuery, dtQueryFilter* NavFilter);
	bool HasSample(/*int32*/int Idx) const;

private:

	bool IsValid(/*int32*/int Idx, dtNavMeshQuery* NavQuery, dtQueryFilter* NavFilter) const;
};
