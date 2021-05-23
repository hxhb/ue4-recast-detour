#include <iostream>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include "Detour/DetourNavMesh.h"
#include "Detour/DetourNavMeshQuery.h"
#include "UE4RecastHelper.h"

void printUsage()
{
	printf("Usage:\n");
	printf("\tue4-findpath.exe dtNavMesh.bin Loc1.X Loc1.Y,loc1.Z Loc2.X Loc2.Y,loc2.Z\n");
	printf("\tue4-detour.exe dtNavMesh.bin 340.0 -130.709 0.0 -470.0 480.0 0.0");
}

int main(int argc, char** argv)
{
	if (argc < 5 || !(argc == 5 || argc == 8))
	{
		printUsage();
		return -1;
	}
	UE4RecastHelper::FVector3 InLoc1((float)std::atof(argv[2]), (float)std::atof(argv[3]), (float)std::atof(argv[4]));
	UE4RecastHelper::FVector3 InLoc2((float)std::atof(argv[5]), (float)std::atof(argv[6]), (float)std::atof(argv[7]));

	printf("InLoc1: X=%f\tY=%f\tZ=%f\t\n", InLoc1.X,InLoc1.Y,InLoc1.Z);
	printf("InLoc2: X=%f\tY=%f\tZ=%f\t\n", InLoc2.X,InLoc2.Y,InLoc2.Z);

	std::string binpath = argv[1];//"E:\\UnrealProjects\\Blank426\\NavMesh\\NewMap-NavData-2021.05.23-12.20.02.bin";

	dtNavMesh* NavMeshData = UE4RecastHelper::DeSerializedtNavMesh(binpath.c_str());
	if (NavMeshData)
	{

		printf("Deserialize %s as dtNavMesh successfuly!\n", binpath.c_str());
		std::vector<UE4RecastHelper::FVector3> Paths;
		UE4RecastHelper::findStraightPath(NavMeshData, NULL, InLoc1, InLoc2, Paths);

		for (const auto& Point : Paths)
		{
			printf("Point: X=%f\tY=%f\tZ=%f\n", Point.X, Point.Y, Point.Z);
		}

	}
	else {
		printf("Deserialize %s as dtNavMesh faild!\n", argv[1]);
	}


	if (!!NavMeshData)
		dtFreeNavMesh(NavMeshData);
	return 0;
}