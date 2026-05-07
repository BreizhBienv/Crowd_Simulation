// Fill out your copyright notice in the Description page of Project Settings.


#include "SphAgent/SphObstacleSubSystem.h"

#include "NavigationSystem.h"
#include "Detour/DetourNavMesh.h"
#include "NavMesh/RecastNavMesh.h"

#define CONVERT_M_TO_CM 100.f

void USphObstacleSubSystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	if (const UNavigationSystemV1* navSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(&InWorld))
	{
		ExtractWallSegments(Cast<ARecastNavMesh>(navSys->GetDefaultNavDataInstance()));
		DebugObstacles();
	}
}

ARecastNavMesh* USphObstacleSubSystem::GetNavMesh(const UWorld* World)
{
	if (!World)
		return nullptr;
	
	const UNavigationSystemV1* navSys = 
		FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	
	if (!navSys)
		return nullptr;
	
	return Cast<ARecastNavMesh>(navSys->GetDefaultNavDataInstance());
}

void USphObstacleSubSystem::ExtractWallSegments(const ARecastNavMesh* NavMesh)
{
	if (!NavMesh) 
		return;
	
	const dtNavMesh* DetourMesh = NavMesh->GetRecastMesh();
	if (!DetourMesh) 
		return;
	
	ObstaclesSegments.Empty();
	
	for (int tileIndex = 0; tileIndex < DetourMesh->getMaxTiles(); ++tileIndex)
	{
		const dtMeshTile* Tile = DetourMesh->getTile(tileIndex);
		if (!Tile || !Tile->header)
			continue;
	
		for (int polyIndex = 0; polyIndex < Tile->header->polyCount; ++polyIndex)
		{
			const dtPoly* Poly = &Tile->polys[polyIndex];
	
			// Only walkable polygons
			if (Poly->getType() != DT_POLYTYPE_GROUND)
				continue;
	
			for (int i = 0; i < Poly->vertCount; ++i)
			{
				// Check if edge is boundary (no neighbour)
				if (Poly->neis[i] != 0)
					continue;
	
				int v0 = Poly->verts[i];
				int v1 = Poly->verts[(i + 1) % Poly->vertCount];
	
				const double* p0 = &Tile->verts[v0 * 3];
				const double* p1 = &Tile->verts[v1 * 3];
				
				FVector WorldV0 = FVector(-p0[0], -p0[2], p0[1]);
				FVector WorldV1 = FVector(-p1[0], -p1[2], p1[1]);
	
				// Convert to 2D (XZ → XY plane)
				FVector2D V0(WorldV0.X, WorldV0.Y);
				FVector2D V1(WorldV1.X, WorldV1.Y);
	
				FObstacleSegment Seg;
				Seg.point0 = V0 / CONVERT_M_TO_CM;
				Seg.point1 = V1 / CONVERT_M_TO_CM;
	
				ObstaclesSegments.Add(Seg);
			}
		}
	}
}

void USphObstacleSubSystem::DebugObstacles()
{
	for (const FObstacleSegment& segment : ObstaclesSegments)
	{
		DrawDebugLine(
			GetWorld(),
			FVector(segment.point0 * CONVERT_M_TO_CM, 20.f),
			FVector(segment.point1 * CONVERT_M_TO_CM, 20.f),
			FColor::Red,true,-1,0,2.f);
	}
}

void USphObstacleSubSystem::OnMapLoaded(UWorld* LoadedWorld)
{
	ExtractWallSegments(GetNavMesh(LoadedWorld));
	DebugObstacles();
}
