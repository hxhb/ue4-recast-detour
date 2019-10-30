#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "Detour/DetourNavMesh.h"
#include "Detour/DetourNavMeshQuery.h"
#include "UE4RecastHelper.h"

int main(int argc,char** argv)
{
	if (argc != 5)
	{
		printf("argc != 5");
		return -1;
	}
	UE4RecastHelper::FCustomVector Point((float)std::atof(argv[2]), (float)std::atof(argv[3]), (float)std::atof(argv[4]) );

	// printf("InPoint: X=%f\tY=%f\tZ=%f\t\n", Point.X,Point.Y,Point.Z);

	dtNavMesh* NvMeshData= UE4RecastHelper::DeSerializedtNavMesh(argv[1]);
	if (NvMeshData)
	{
		printf("Deserialize dtNavMesh is valid\n");
		bool isvalidPoint = UE4RecastHelper::dtIsValidNagivationPoint(NvMeshData, Point);
		if (isvalidPoint)
		{
			printf("InPoint: X=%f\tY=%f\tZ=%f\n", Point.X, Point.Y, Point.Z);
			printf("The Location is valid position.\n");
		}
		else {
			printf("InPoint: X=%f\tY=%f\tZ=%f\n", Point.X, Point.Y, Point.Z);
			printf("The Location is invalid position.\n");
		}
	}
	else {
		printf("Deserialize dtNavMesh is invalid");
	}
	return 0;
}