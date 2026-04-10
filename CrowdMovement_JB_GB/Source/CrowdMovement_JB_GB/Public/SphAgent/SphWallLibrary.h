// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class ARecastNavMesh;

USTRUCT()
struct FWallSegment
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	FVector2D Center;
	
	UPROPERTY(VisibleAnywhere)
	float Area;
	
	UPROPERTY(VisibleAnywhere)
	FVector2D point0;
	
	UPROPERTY(VisibleAnywhere)
	FVector2D point1;
};

class CROWDMOVEMENT_JB_GB_API SphWallLibrary
{
public:
	static ARecastNavMesh* GetNavMesh(const UWorld* World);
	static void ExtractWallSegments(ARecastNavMesh* NavMesh, TArray<FWallSegment>& OutSegments);
	static float SegmentCircleArea(const FVector2D& A, const FVector2D& B, float R);
	static int IntersectSegmentCircle(const FVector2D& A, const FVector2D& B, float R, float& t0, float& t1);
	static float TriangleArea(const FVector2D& A, const FVector2D& B);
	static float SectorArea(const FVector2D& A, const FVector2D& B, float R);
	static float AngleRad(const FVector2D& A, const FVector2D& B);
};
