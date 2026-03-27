// Fill out your copyright notice in the Description page of Project Settings.


#include "SphAgent/SphAgentManager.h"

#include "NavigationSystem.h"
#include "Detour/DetourNavMesh.h"
#include "Kismet/GameplayStatics.h"
#include "NavMesh/RecastNavMesh.h"
#include "SphAgent/SphAgent.h"

// Sets default values
ASphAgentManager::ASphAgentManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASphAgentManager::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASphAgent::StaticClass(), actors);
	Agents = reinterpret_cast<TArray<ASphAgent*>&>(actors);
	
	RegisterAgentsInHashGrid();
	
	ExtractWallSegments(GetNavMesh(GetWorld()), WallSegments);
	DebugObstacles();
}

// Called every frame
void ASphAgentManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DebugHashGrid();
}

ARecastNavMesh* ASphAgentManager::GetNavMesh(const UWorld* World) const
{
	if (!World)
		return nullptr;
	
	UNavigationSystemV1* navSys = 
		FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	
	if (!navSys)
		return nullptr;
	
	return Cast<ARecastNavMesh>(navSys->GetDefaultNavDataInstance());
}

void ASphAgentManager::ExtractWallSegments(ARecastNavMesh* NavMesh, TArray<FWallSegment>& OutSegments)
{
	if (!NavMesh) 
		return;
	
	const dtNavMesh* DetourMesh = NavMesh->GetRecastMesh();
	if (!DetourMesh) 
		return;
	
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
	
				FWallSegment Seg;
				Seg.Center = (V0 + V1) * 0.5f;
				Seg.Length = (V1 - V0).Length();
				Seg.point0 = V0;
				Seg.point1 = V1;
	
				OutSegments.Add(Seg);
			}
		}
	}
}

void ASphAgentManager::RegisterAgentsInHashGrid()
{
	HashGrid.Empty();
	
	for (ASphAgent* agent : Agents)
	{
		FVector location = agent->GetActorLocation();
		FIntVector2 gridCell = FIntVector2(
			FMath::FloorToInt(location.X / CellSize.X),
			FMath::FloorToInt(location.Y / CellSize.Y)
			);
		uint32 hashedIndex = GetTypeHash(gridCell);
		
		FHashGridData* data = HashGrid.Find(hashedIndex);
		if (data == nullptr)
		{
			FHashGridData newData;
			newData.Agents.Add(agent);
			newData.cellIndex = gridCell;
			HashGrid.Add(hashedIndex, newData);
		}
		else
			data->Agents.Add(agent);
	}
}

void ASphAgentManager::UpdateAgents()
{
	FindAgentsNeighbours();
	ComputeRequiredComponents();
	ComputeForces();
}

void ASphAgentManager::FindAgentsNeighbours()
{
	for (ASphAgent* agent : Agents)
	{
		TArray<FSphAgentNeighbourData> neighbours;
		
		FVector location = agent->GetActorLocation();
		FIntVector2 gridCell = FIntVector2(
			FMath::FloorToInt(location.X / CellSize.X),
			FMath::FloorToInt(location.Y / CellSize.Y)
			);
		
		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				FIntVector2 neighbourCell = FIntVector2(
					gridCell.X + i,
					gridCell.Y + j
					);
				uint32 neighbourHashedIndex = GetTypeHash(neighbourCell);

				if (FHashGridData* data = HashGrid.Find(neighbourHashedIndex))
				{
					for (ASphAgent* neighbourAgent : data->Agents)
					{
						if (agent == neighbourAgent)
							continue;

						neighbours.Add({neighbourAgent});
					}
				}
							
			}
		}
		
		agent->SetNeighboursData(neighbours);
	}
}

void ASphAgentManager::ComputeRequiredComponents()
{
	for (ASphAgent* agent : Agents)
		agent->ComputeRequiredComponents();
}

void ASphAgentManager::ComputeForces()
{
	for (ASphAgent* agent : Agents)
		agent->ComputeSphForces();
}

FVector ASphAgentManager::FromCellToWorldCoord(FIntVector2 CellId) const
{
	return (FVector(CellId.X + 0.5f, CellId.Y + 0.5f, 0)) * FVector(CellSize.X, CellSize.Y, 0);
}

void ASphAgentManager::DebugHashGrid()
{
	for (auto element : HashGrid)
	{
		FVector cellLocation = FromCellToWorldCoord(element.Value.cellIndex);
		FVector cellExtent = FVector(CellSize.X, CellSize.Y, 0) / 2.f - FVector(0.5f, 0.5f, 0);
		
		DrawDebugBox(GetWorld(), cellLocation, cellExtent, FColor::Green);
	}
}

void ASphAgentManager::DebugObstacles()
{
	for (const FWallSegment& segment : WallSegments)
	{
		DrawDebugLine(
			GetWorld(),
			FVector(segment.point0, 20.f),
			FVector(segment.point1, 20.f),
			FColor::Red,true,-1,0,2.f);
	}
}
