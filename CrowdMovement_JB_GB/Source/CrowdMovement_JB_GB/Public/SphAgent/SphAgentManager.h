// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SphAgentManager.generated.h"

class ARecastNavMesh;
class ASphAgent;

USTRUCT(Blueprintable)
struct FHashGridData
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	FIntVector2 cellIndex;
	
	UPROPERTY(VisibleAnywhere)
	TArray<ASphAgent*> Agents;
};

USTRUCT()
struct FWallSegment
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
    FVector2D Center;   // r_k^p
	
	UPROPERTY(VisibleAnywhere)
    float Length;       // a(P_k)
	
	UPROPERTY(VisibleAnywhere)
	FVector2D point0;
	
	UPROPERTY(VisibleAnywhere)
	FVector2D point1;
};

UCLASS()
class CROWDMOVEMENT_JB_GB_API ASphAgentManager : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	TArray<ASphAgent*> Agents;
	
	UPROPERTY(VisibleAnywhere)
	TArray<FWallSegment> WallSegments;
	
	UPROPERTY(VisibleAnywhere, Category="Hash Grid")
	TMap<uint32, FHashGridData> HashGrid;
	
	UPROPERTY(EditAnywhere, Category="Hash Grid")
	FIntVector2 CellSize = FIntVector2(500, 500);
	
public:
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bDebugHashGrid = true;
	
	// Sets default values for this actor's properties
	ASphAgentManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	ARecastNavMesh* GetNavMesh(const UWorld* World) const;
	void ExtractWallSegments(ARecastNavMesh* NavMesh, TArray<FWallSegment>& OutSegments);
	
	
	void RegisterAgentsInHashGrid();
	void UpdateAgents();
	void FindAgentsNeighbours();
	void ComputeRequiredComponents();
	void ComputeForces();
	FVector FromCellToWorldCoord(FIntVector2 CellId) const;
	
	void DebugHashGrid();
	void DebugObstacles();
};
