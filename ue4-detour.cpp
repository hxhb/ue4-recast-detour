#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "Detour/DetourNavMesh.h"
#include "Detour/DetourNavMeshQuery.h"
#include "UE4RecastHelper.h"
#include "ObstaclesTools/BuildObstacles.hpp"

void printUsage();
int DoCheckPosision(int argc, char** argv);


int main(int argc,char** argv)
{
	// return DoCheckPosision(argc, argv);


	const char* filepath = "D:\\dungeon.obj";
	FObstaclesTool ObstaclesTool;
	ObstaclesTool.BuildObstacles(filepath);
}


void printUsage()
{
	printf("Usage:\n");
	printf("\tue4-detour.exe dtNavMesh.bin Loc.X Loc.Y,loc.Z Extren.X Extern.Y Extren.Z\n");
	printf("PS:{Extern.X Extern.Y Extern.Z} can be ignored,default is {10.f 10.f 10.f}\n");
	printf("For Example:\n");
	printf("\tue4-detour.exe dtNavMesh.bin -770.003 -593.709 130.267 10.0 10.0 10.0");
}

int DoCheckPosision(int argc, char** argv)
{
	if (argc < 5 || !(argc == 5 || argc == 8))
	{
		printUsage();
		return -1;
	}
	UE4RecastHelper::FVector3 InPoint((float)std::atof(argv[2]), (float)std::atof(argv[3]), (float)std::atof(argv[4]));
	UE4RecastHelper::FVector3 InExtern;

	if (argc == 5)
	{
		InExtern = UE4RecastHelper::FVector3((float)10.0f, (float)10.f, (float)10.f);

	}
	if (argc == 8)
	{
		InExtern = UE4RecastHelper::FVector3((float)std::atof(argv[5]), (float)std::atof(argv[6]), (float)std::atof(argv[7]));
	}
	// printf("InPoint: X=%f\tY=%f\tZ=%f\t\n", Point.X,Point.Y,Point.Z);

	dtNavMesh* NavMeshData = UE4RecastHelper::DeSerializedtNavMesh(argv[1]);
	if (NavMeshData)
	{

		printf("Deserialize %s as dtNavMesh successfuly!\n", argv[1]);
		bool isvalidPoint = UE4RecastHelper::dtIsValidNavigationPoint(NavMeshData, InPoint, InExtern);

		printf("InPoint: X=%f\tY=%f\tZ=%f\n", InPoint.X, InPoint.Y, InPoint.Z);
		printf("Extern: X=%f\tY=%f\tZ=%f\n", InExtern.X, InExtern.Y, InExtern.Z);
		printf("The Location is %s navigation position.\n", isvalidPoint ? "valid" : "invalid");

	}
	else {
		printf("Deserialize %s as dtNavMesh faild!\n", argv[1]);
	}


	if (!!NavMeshData)
		dtFreeNavMesh(NavMeshData);
	return 0;
}