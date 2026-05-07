// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SphObstacleSubSystem.generated.h"


class ANavigationData;
class ARecastNavMesh;

USTRUCT()
struct FObstacleSegment
{
	GENERATED_BODY()
	
	FVector2D Center;
	float Area;
	FVector2D point0;
	FVector2D point1;
};

/**
 * 
 */
UCLASS()
class CROWDMOVEMENT_JB_GB_API USphObstacleSubSystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
	TArray<FObstacleSegment> ObstaclesSegments;
	
	ARecastNavMesh* GetNavMesh(const UWorld* World);
	void ExtractWallSegments(const ARecastNavMesh* NavMesh);
	void DebugObstacles();
	
	UFUNCTION()
	void OnMapLoaded(UWorld* LoadedWorld);

protected:	
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	
public:
	const TArray<FObstacleSegment>& GetObstacleSegments() const { return ObstaclesSegments; }
};
