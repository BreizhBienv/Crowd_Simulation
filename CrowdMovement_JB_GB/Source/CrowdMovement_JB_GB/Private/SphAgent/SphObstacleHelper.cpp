// Fill out your copyright notice in the Description page of Project Settings.


#include "SphAgent/SphObstacleHelper.h"

#include "SphAgent/SphObstacleSubSystem.h"

#define CONVERT_M_TO_CM 100.f

bool SphObstacleHelper::ComputeSegmentData(const FVector2D ri, const float h, const FObstacleSegment& segment, FObstacleSegment& outData)
{	
	FVector2D closestPoint = FMath::ClosestPointOnSegment2D(ri, segment.point0, segment.point1);
	FVector2D AgentToWall = closestPoint - ri;
	const float distToAgent = AgentToWall.Size();
	if (distToAgent > h)
		return false;
	
	AgentToWall.Normalize();
	const FVector2D densityCenterPoint = ri + (AgentToWall * (distToAgent + (h - distToAgent) / 2.f));
	const float area = SegmentCircleArea(segment.point0 - ri, segment.point1 - ri, h);
	
	FObstacleSegment registeredWallSegment;
	
	registeredWallSegment.point0 = segment.point0;
	registeredWallSegment.point1 = segment.point1;
	registeredWallSegment.Center = densityCenterPoint;
	registeredWallSegment.Area = area;
	
	outData = registeredWallSegment;
	return true;
}

/// @param A Should be in circle local space, meaning in the local_A = world_A - circle_center
/// @param B In circle local space too
/// @param R circle radius
/// @return The area between the segment and the edge of the circle
float SphObstacleHelper::SegmentCircleArea(const FVector2D& A, const FVector2D& B, float R)
{
	float t0, t1;
	const int hit = IntersectSegmentCircle(A, B, R, t0, t1);
	
	if (hit == 0)
		return 0;

	// Clamp to segment
	t0 = FMath::Clamp(t0, 0.0f, 1.0f);
	t1 = FMath::Clamp(t1, 0.0f, 1.0f);
	
	// Sort t
	if (t0 > t1)
		Swap(t0, t1);
	
	FVector2D d = B - A;
	//d.Normalize();
	
	//Segment points projected on circle
	const FVector2D I0 = A + d * t0;
	const FVector2D I1 = A + d * t1;
	
	float area = SectorArea(I0, I1, R) - TriangleArea(I0, I1);
	return area;
}

int SphObstacleHelper::IntersectSegmentCircle(const FVector2D& A, const FVector2D& B, float R, float& t0, float& t1)
{
	FVector2D d = B - A;

	float a = d.SizeSquared();
	float b = 2.f * FVector2D::DotProduct(A, d);
	float c = A.SizeSquared() - R * R;

	float disc = b * b - 4 * a * c;

	if (disc < 0)
		return 0; //No intersections
	
	if (disc == 0)
	{
		t0 = -b / (2 * a);
		t1 = t0;
		return 1; //Tangent, one intersection
	}
	
	float sqrtDisc = FMath::Sqrt(disc);
	t0 = (-b - sqrtDisc) / (2 * a);
	t1 = (-b + sqrtDisc) / (2 * a);
	return 2; //Two intersections
}

float SphObstacleHelper::TriangleArea(const FVector2D& A, const FVector2D& B)
{
	return 0.5f * FVector2D::CrossProduct(A, B);
}

float SphObstacleHelper::SectorArea(const FVector2D& A, const FVector2D& B, float R)
{
	float theta = FMath::Abs(AngleRad(A, B));	
	return 0.5f * (R * R) * theta;
}

float SphObstacleHelper::AngleRad(const FVector2D& A, const FVector2D& B)
{
	return FMath::Atan2(FVector2D::CrossProduct(A, B), FVector2D::DotProduct(A, B));
}