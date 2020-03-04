#include "UE4RecastHelper.h"
#include <cstring>
#include <cstdio>

#pragma warning (disable:4996)

// #include "HACK_PRIVATE_MEMBER_UTILS.hpp"

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


bool UE4RecastHelper::FindDetourPathByNavMesh(dtNavMesh* InNavMesh, const FVector3& InStart, const FVector3& InEnd, std::vector<FVector3>& OutPaths)
{
	dtNavMeshQuery NavQuery;
	dtQueryFilter QueryFilter;

	NavQuery.init(InNavMesh, 1024);

	FVector3 RcStart = UE4RecastHelper::Unreal2RecastPoint(InStart);
	FVector3 RcEnd = UE4RecastHelper::Unreal2RecastPoint(InEnd);
	float Extern[3]{ 10.f,10.f,10.f };

	float StartPoint[3]{ RcStart.X,RcStart.Y,RcStart.Z };
	dtPolyRef StartPolyRef;
	float StartNarestPt[3]{ 0.f };
	dtStatus StartStatus = NavQuery.findNearestPoly(StartPoint, Extern, &QueryFilter, &StartPolyRef, StartNarestPt);

	float EndPoint[3]{ RcEnd.X,RcEnd.Y,RcEnd.Z };
	dtPolyRef EndPolyRef;
	float EndNarestPt[3]{ 0.f };
	dtStatus EndStatus = NavQuery.findNearestPoly(EndPoint, Extern, &QueryFilter, &EndPolyRef, EndNarestPt);

	// UE_LOG(LogTemp, Log, TEXT("Start Point FindNearestPoly status is %u,PolyRef is %u."), StartStatus, StartPolyRef);
	// UE_LOG(LogTemp, Log, TEXT("End Point FindNearestPoly status is %u.,PolyRef is %u."), EndStatus, EndPolyRef);
	// UE_LOG(LogTemp, Log, TEXT("Start Point FindNearestPoly narestpt is %s."), *FVector3(StartPoint[0], StartPoint[1], StartPoint[2]).ToString());
	// UE_LOG(LogTemp, Log, TEXT("End Point FindNearestPoly narestpt is %s."), *FVector3(EndPoint[0], EndPoint[1], EndPoint[2]).ToString());

	dtQueryResult result;
	float totalcost[1024 * 3];
	dtStatus FindPathStatus = NavQuery.findPath(StartPolyRef, EndPolyRef, StartNarestPt, EndNarestPt, &QueryFilter, result, totalcost);

	// UE_LOG(LogTemp, Log, TEXT("findPath status is %u.,result size is %u."), FindPathStatus, result.size());
	std::vector<dtPolyRef> path;

	for (int index = 0; index < result.size(); ++index)
	{
		// UE_LOG(LogTemp, Log, TEXT("Find Path index is %d ref is %u."), index, result.getRef(index));
		path.push_back(result.getRef(index));
		float currentpos[3]{ 0.f };
		result.getPos(index, currentpos);
		// UE_LOG(LogTemp, Log, TEXT("Find Path index is %d pos is %s."), index, *UE4RecastHelper::Recast2UnrealPoint(FVector3(currentpos[0], currentpos[1], currentpos[2])).ToString());
		// OutPaths.Add(UFlibExportNavData::Recast2UnrealPoint(FVector3(currentpos[0], currentpos[1], currentpos[2])));
	}
	dtQueryResult findStraightPathResult;
	NavQuery.findStraightPath(StartNarestPt, EndNarestPt, path.data(), path.size(), findStraightPathResult);


	// UE_LOG(LogTemp, Log, TEXT("findStraightPath size is %u."), findStraightPathResult.size());
	for (int index = 0; index < findStraightPathResult.size(); ++index)
	{
		float currentpos[3]{ 0.f };
		findStraightPathResult.getPos(index, currentpos);
		// UE_LOG(LogTemp, Log, TEXT("findStraightPath index is %d ref is %u."), index, findStraightPathResult.getRef(index));
		// UE_LOG(LogTemp, Log, TEXT("findStraightPath index is %d pos is %s."), index, *UE4RecastHelper::Recast2UnrealPoint(FVector3(currentpos[0], currentpos[1], currentpos[2])).ToString());
		OutPaths.push_back(UE4RecastHelper::Recast2UnrealPoint(FVector3(currentpos[0], currentpos[1], currentpos[2])));
	}
	return true;
}

// DECL_HACK_PRIVATE_NOCONST_FUNCTION(dtNavMesh, getTile, dtMeshTile*, int);

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

UE4RecastHelper::FVector3 UE4RecastHelper::Recast2UnrealPoint(const FVector3& Vector)
{
	return FVector3(-Vector.X, -Vector.Z, Vector.Y);
}

UE4RecastHelper::FVector3 UE4RecastHelper::Unreal2RecastPoint(const FVector3& Vector)
{
	return FVector3(-Vector.X, Vector.Z, -Vector.Y);
}
