// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SphWallLibrary.generated.h"

class ARecastNavMesh;

USTRUCT()
struct FWallSegment
{
	GENERATED_BODY()
	
	FVector2D Center;
	float Area;
	FVector2D point0;
	FVector2D point1;
};

class CROWDMOVEMENT_JB_GB_API SphWallLibrary
{
public:
	static ARecastNavMesh* GetNavMesh(const UWorld* World);
	static void ExtractWallSegments(ARecastNavMesh* NavMesh, TArray<FWallSegment>& OutSegments);
	static float SegmentCircleArea(const FVector2D& A, const FVector2D& B, float R, UWorld* World);
	static int IntersectSegmentCircle(const FVector2D& A, const FVector2D& B, float R, float& t0, float& t1);
	static float TriangleArea(const FVector2D& A, const FVector2D& B);
	static float SectorArea(const FVector2D& A, const FVector2D& B, float R);
	static float AngleRad(const FVector2D& A, const FVector2D& B);
};
