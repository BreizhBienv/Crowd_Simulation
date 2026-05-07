// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FObstacleSegment;

class CROWDMOVEMENT_JB_GB_API SphObstacleHelper
{
public:
	static bool ComputeSegmentData(FVector2D ri, float h, const struct FObstacleSegment& segment, FObstacleSegment& outData);
	static float SegmentCircleArea(const FVector2D& A, const FVector2D& B, float R);
	static int IntersectSegmentCircle(const FVector2D& A, const FVector2D& B, float R, float& t0, float& t1);
	static float TriangleArea(const FVector2D& A, const FVector2D& B);
	static float SectorArea(const FVector2D& A, const FVector2D& B, float R);
	static float AngleRad(const FVector2D& A, const FVector2D& B);
};
