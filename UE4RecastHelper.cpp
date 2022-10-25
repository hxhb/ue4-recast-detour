// Copyright 2019 Lipeng Zha, Inc. All Rights Reserved.

#include "UE4RecastHelper.h"
#include "Detour/DetourStatus.h"
#include "Detour/DetourNavMeshQuery.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>

#pragma warning(disable:4996)

static const long RCN_NAVMESH_VERSION = 1;
static const int INVALID_NAVMESH_POLYREF = 0;
static const int MAX_POLYS = 256;
static const int NAV_ERROR_NEARESTPOLY = -2;


bool UE4RecastHelper::dtIsValidNavigationPoint(dtNavMesh* InNavMeshData, const UE4RecastHelper::FVector3& InPoint, const UE4RecastHelper::FVector3& InExtent)
{
	bool bSuccess = false;

	using namespace UE4RecastHelper;

	if (!InNavMeshData) return bSuccess;

	FVector3 RcPoint = UE4RecastHelper::Unreal2RecastPoint(InPoint);
	const FVector3 ModifiedExtent = InExtent;
	FVector3 RcExtent = UE4RecastHelper::Unreal2RecastPoint(ModifiedExtent).GetAbs();
	FVector3 ClosestPoint;


	dtNavMeshQuery NavQuery;
#ifdef USE_DETOUR_BUILT_INTO_UE4
	dtQuerySpecialLinkFilter LinkFilter;
	NavQuery.init(InNavMeshData, 0, &LinkFilter);
	// UE_LOG(LogTemp, Warning, TEXT("CALL NavQuery.init(InNavMeshData, 0, &LinkFilter);"));
#else
	NavQuery.init(InNavMeshData, 0);
#endif

	dtPolyRef PolyRef;
	dtQueryFilter QueryFilter;

#ifdef USE_DETOUR_BUILT_INTO_UE4
	NavQuery.findNearestPoly2D(&RcPoint.X, &RcExtent.X, &QueryFilter, &PolyRef, (float*)(&ClosestPoint));
	// UE_LOG(LogTemp, Warning, TEXT("CALL findNearestPoly2D"));
#else
	NavQuery.findNearestPoly(&RcPoint.X, &RcExtent.X, &QueryFilter, &PolyRef, (float*)(&ClosestPoint));
#endif

	// UE_LOG(LogTemp, Log, TEXT("dtIsValidNavigationPoint PolyRef is %ud."), PolyRef);
	if (PolyRef > 0)
	{
		const FVector3& UnrealClosestPoint = UE4RecastHelper::Recast2UnrealPoint(ClosestPoint);
		const FVector3 ClosestPointDelta = UnrealClosestPoint - InPoint;
		if (-ModifiedExtent.X <= ClosestPointDelta.X && ClosestPointDelta.X <= ModifiedExtent.X
			&& -ModifiedExtent.Y <= ClosestPointDelta.Y && ClosestPointDelta.Y <= ModifiedExtent.Y
			&& -ModifiedExtent.Z <= ClosestPointDelta.Z && ClosestPointDelta.Z <= ModifiedExtent.Z)
		{
			bSuccess = true;
		}
	}

	return bSuccess;
}

static const int MAX_PATHQUEUE_NODES = 4096;
static const int MAX_COMMON_NODES = 512;

int UE4RecastHelper::findStraightPath(dtNavMesh* InNavMeshData, dtNavMeshQuery* InNavmeshQuery, const FVector3& start, const FVector3& end, std::vector<FVector3>& paths)
{
	bool bSuccess = false;

	using namespace UE4RecastHelper;

	if (!InNavMeshData) return bSuccess;

	FVector3 RcStart = UE4RecastHelper::Unreal2RecastPoint(start);
	FVector3 RcEnd = UE4RecastHelper::Unreal2RecastPoint(end);
	FVector3 RcExtent{ 10.f,10.f,10.f };

	dtNavMeshQuery NavQuery;
#ifdef USE_DETOUR_BUILT_INTO_UE4
	dtQuerySpecialLinkFilter LinkFilter;
	NavQuery.init(InNavMeshData, 0, &LinkFilter);
	// UE_LOG(LogTemp, Warning, TEXT("CALL NavQuery.init(InNavMeshData, 0, &LinkFilter);"));
#else
	NavQuery.init(InNavMeshData, 1024);
#endif


	FVector3 StartClosestPoint;
	FVector3 EndClosestPoint;

	dtPolyRef StartPolyRef;
	dtPolyRef EndPolyRef;
	dtQueryFilter QueryFilter;

#ifdef USE_DETOUR_BUILT_INTO_UE4
	dtStatus StartStatus = NavQuery.findNearestPoly2D(&RcStart.X, &RcExtent.X, &QueryFilter, &StartPolyRef, (float*)(&StartClosestPoint));
	// UE_LOG(LogTemp, Warning, TEXT("CALL findNearestPoly2D"));
#else
	dtStatus StartStatus = NavQuery.findNearestPoly(&RcStart.X, &RcExtent.X, &QueryFilter, &StartPolyRef, &StartClosestPoint.X);
#endif

#ifdef USE_DETOUR_BUILT_INTO_UE4
	dtStatus EndStatus = NavQuery.findNearestPoly2D(&RcEnd.X, &RcExtent.X, &QueryFilter, &EndPolyRef, (float*)(&EndClosestPoint));
	// UE_LOG(LogTemp, Warning, TEXT("CALL findNearestPoly2D"));
#else
	dtStatus EndStatus = NavQuery.findNearestPoly(&RcEnd.X, &RcExtent.X, &QueryFilter, &EndPolyRef, &EndClosestPoint.X);
#endif

	if (!dtStatusSucceed(StartStatus) || !dtStatusSucceed(EndStatus))
	{
		//UE_LOG(LogTemp, Log, TEXT("find Start or End nearest poly faild."))
		return false;
	}

#ifdef USE_DETOUR_BUILT_INTO_UE4
	UE_LOG(LogTemp, Log, TEXT("FindDetourPath StartPolyRef is %u."), StartPolyRef);
	UE_LOG(LogTemp, Log, TEXT("FindDetourPath EndPolyRef is %u."), EndPolyRef);

	UE_LOG(LogTemp, Log, TEXT("FindDetourPath StartClosestPoint is %s."), *FVector(StartClosestPoint.X,StartClosestPoint.Y,StartClosestPoint.Z).ToString());
	UE_LOG(LogTemp, Log, TEXT("FindDetourPath EndClosestPoint is %s."), *FVector(EndClosestPoint.X, EndClosestPoint.Y, EndClosestPoint.Z).ToString());
#endif

	dtQueryResult Result;
	float totalcost[1024 * 3] = {0.f};

	dtStatus FindPathStatus = NavQuery.findPath(StartPolyRef, EndPolyRef, &StartClosestPoint.X, &EndClosestPoint.X,FLT_MAX, &QueryFilter, Result, totalcost);
#ifdef USE_DETOUR_BUILT_INTO_UE4
	UE_LOG(LogTemp, Log, TEXT("FindDetourPath FindPath return status is %u."), FindPathStatus);

	UE_LOG(LogTemp, Log, TEXT("FindDetourPath dtQueryResult size is %u."), Result.size());
#endif
	if (dtStatusSucceed(FindPathStatus))
	{
		std::vector<dtPolyRef> local_paths;

		for (int index = 0; index < Result.size(); ++index)
		{
			// UE_LOG(LogTemp, Log, TEXT("Find Path index is %d ref is %u."), index, result.getRef(index));
			local_paths.push_back(Result.getRef(index));
			float currentpos[3]{ 0.f };
			Result.getPos(index, currentpos);
			// UE_LOG(LogTemp, Log, TEXT("Find Path index is %d pos is %s."), index, *UFlibExportNavData::Recast2UnrealPoint(FVector(currentpos[0], currentpos[1], currentpos[2])).ToString());
			// OutPaths.Add(UFlibExportNavData::Recast2UnrealPoint(FVector(currentpos[0], currentpos[1], currentpos[2])));
		}
		dtQueryResult findStraightPathResult;
		NavQuery.findStraightPath(&StartClosestPoint.X, &EndClosestPoint.X, local_paths.data(), static_cast<int>(local_paths.size()), findStraightPathResult);

		for (int index = 0; index < findStraightPathResult.size(); ++index)
		{
			float currentpos[3]{ 0.f };
			findStraightPathResult.getPos(index, currentpos);
			// UE_LOG(LogTemp, Log, TEXT("findStraightPath index is %d ref is %u."), index, findStraightPathResult.getRef(index));
			// UE_LOG(LogTemp, Log, TEXT("findStraightPath index is %d pos is %s."), index, *UFlibExportNavData::Recast2UnrealPoint(FVector(currentpos[0], currentpos[1], currentpos[2])).ToString());
			paths.push_back(UE4RecastHelper::Recast2UnrealPoint(FVector3(currentpos[0], currentpos[1], currentpos[2])));
		}
	}

//	InNavmeshQuery->findStraightPath(&StartNearestPt.X, &EndNearestPt.X, );
	return 0;
}

bool UE4RecastHelper::GetRandomPointInRadius(dtNavMeshQuery* InNavmeshQuery, dtQueryFilter* InQueryFilter, const FVector3& InOrigin, const FVector3& InRedius, FVector3& OutPoint)
{
	bool bStatus = false;
	dtNavMeshQuery* NavQuery = InNavmeshQuery;
	if (!NavQuery)
	{
		return false;
	}
	dtPolyRef OriginPolyRef;
	FVector3 ClosestPoint;
	FVector3 RcPoint = UE4RecastHelper::Unreal2RecastPoint(InOrigin);

#ifdef USE_DETOUR_BUILT_INTO_UE4
	InNavmeshQuery->findNearestPoly2D(&RcPoint.X, &InRedius.X, InQueryFilter, &OriginPolyRef, (float*)(&ClosestPoint));
	// UE_LOG(LogTemp, Warning, TEXT("CALL findNearestPoly2D"));
#else
	NavQuery->findNearestPoly(&RcPoint.X, &InRedius.X, InQueryFilter, &OriginPolyRef, (float*)(&ClosestPoint));
#endif

	dtPolyRef ResultPoly;
	FVector3 ResultPoint;
	auto NormalRand = []()->float
	{
		return std::rand() / (float)RAND_MAX;
	};

	dtStatus Status = NavQuery->findRandomPointAroundCircle(OriginPolyRef, &RcPoint.X, InRedius.X, InQueryFilter, NormalRand, &ResultPoly, &ResultPoint.X);

	if (dtStatusSucceed(Status))
	{
		OutPoint = UE4RecastHelper::Recast2UnrealPoint(ResultPoint);
		bStatus = true;
	}
	return bStatus;
}

void UE4RecastHelper::SerializedtNavMesh(const char* path, const dtNavMesh* mesh)
{
	using namespace UE4RecastHelper;
	if (!mesh) return;

	std::FILE* fp = std::fopen(path, "wb");
	if (!fp)
		return;

	// Store header.
	NavMeshSetHeader header;
	header.magic = NAVMESHSET_MAGIC;
	header.version = NAVMESHSET_VERSION;
	header.numTiles = 0;
	// auto dtNavMesh_getTile = GET_PRIVATE_MEMBER_FUNCTION(dtNavMesh, getTile);

	for (int i = 0; i < mesh->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = mesh->getTile(i);
		// const dtMeshTile* tile = CALL_MEMBER_FUNCTION(mesh, dtNavMesh_getTile, i);
		if (!tile || !tile->header || !tile->dataSize) continue;
		header.numTiles++;
	}
	std::memcpy(&header.params, mesh->getParams(), sizeof(dtNavMeshParams));
	std::fwrite(&header, sizeof(NavMeshSetHeader), 1, fp);

	// Store tiles.
	for (int i = 0; i < mesh->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = mesh->getTile(i);
		// const dtMeshTile* tile = CALL_MEMBER_FUNCTION(mesh, dtNavMesh_getTile, i);
		if (!tile || !tile->header || !tile->dataSize) continue;

		NavMeshTileHeader tileHeader;
		tileHeader.tileRef = mesh->getTileRef(tile);
		tileHeader.dataSize = tile->dataSize;
		std::fwrite(&tileHeader, sizeof(tileHeader), 1, fp);

		std::fwrite(tile->data, tile->dataSize, 1, fp);
	}

	std::fclose(fp);
}

dtNavMesh* UE4RecastHelper::DeSerializedtNavMesh(const char* path)
{

	std::FILE* fp = std::fopen(path, "rb");
	if (!fp) return 0;

	using namespace UE4RecastHelper;
	// Read header.
	NavMeshSetHeader header;
	size_t sizenum = sizeof(NavMeshSetHeader);
	size_t readLen = std::fread(&header, sizenum, 1, fp);
	if (readLen != 1)
	{
		std::fclose(fp);
		return 0;
	}
	if (header.magic != NAVMESHSET_MAGIC)
	{
		std::fclose(fp);
		return 0;
	}
	if (header.version != NAVMESHSET_VERSION)
	{
		std::fclose(fp);
		return 0;
	}

	dtNavMesh* mesh = dtAllocNavMesh();
	if (!mesh)
	{
		std::fclose(fp);
		return 0;
	}
	dtStatus status = mesh->init(&header.params);
	if (dtStatusFailed(status))
	{
		std::fclose(fp);
		return 0;
	}

	// Read tiles.
	for (int i = 0; i < header.numTiles; ++i)
	{
		NavMeshTileHeader tileHeader;
		readLen = std::fread(&tileHeader, sizeof(tileHeader), 1, fp);
		if (readLen != 1)
		{
			std::fclose(fp);
			return 0;
		}

		if (!tileHeader.tileRef || !tileHeader.dataSize)
			break;

		unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
		if (!data) break;
		std::memset(data, 0, tileHeader.dataSize);
		readLen = fread(data, tileHeader.dataSize, 1, fp);
		if (readLen != 1)
		{
			dtFree(data);
			fclose(fp);
			return 0;
		}

		mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
	}

	std::fclose(fp);

	return mesh;
}




namespace UE4RecastHelper
{
	FVector3 Recast2UnrealPoint(const FVector3& Vector)
	{
		return FVector3(-Vector.X, -Vector.Z, Vector.Y);
	}

	FVector3 Unreal2RecastPoint(const FVector3& Vector)
	{
		return FVector3(-Vector.X, Vector.Z, -Vector.Y);
	}
};