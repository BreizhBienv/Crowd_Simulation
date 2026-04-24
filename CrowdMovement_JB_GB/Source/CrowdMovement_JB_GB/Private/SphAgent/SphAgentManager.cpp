// Fill out your copyright notice in the Description page of Project Settings.


#include "SphAgent/SphAgentManager.h"

#include "Kismet/GameplayStatics.h"
#include "SphAgent/SphAgent.h"
#include "SphAgent/SphWallLibrary.h"

#define CONVERT_M_TO_CM 100.f

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
	
	
	SphWallLibrary::ExtractWallSegments(SphWallLibrary::GetNavMesh(GetWorld()), WallSegments);
	DebugObstacles();
}

// Called every frame
void ASphAgentManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	RegisterAgentsInHashGrid();
	UpdateAgents(DeltaTime);
	
	DebugHashGrid();
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

void ASphAgentManager::UpdateAgents(float DeltaTime)
{
	FindAgentsNeighbours();
	ComputeRequiredComponents();
	ComputeForces(DeltaTime);
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
		agent->ComputeRequiredComponents(WallSegments);
}

void ASphAgentManager::ComputeForces(float DeltaTime)
{
	for (ASphAgent* agent : Agents)
		agent->ComputeSphForces(DeltaTime);
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
			FVector(segment.point0 * CONVERT_M_TO_CM, 20.f),
			FVector(segment.point1 * CONVERT_M_TO_CM, 20.f),
			FColor::Red,true,-1,0,2.f);
	}
}
