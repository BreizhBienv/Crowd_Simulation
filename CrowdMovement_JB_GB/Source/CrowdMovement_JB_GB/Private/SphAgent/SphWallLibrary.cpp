// Fill out your copyright notice in the Description page of Project Settings.


#include "SphAgent/SphWallLibrary.h"

#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "Detour/DetourNavMesh.h"

void SphWallLibrary::ExtractWallSegments(ARecastNavMesh* NavMesh, TArray<FWallSegment>& OutSegments)
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
				Seg.point0 = V0;
				Seg.point1 = V1;
	
				OutSegments.Add(Seg);
			}
		}
	}
}

ARecastNavMesh* SphWallLibrary::GetNavMesh(const UWorld* World)
{
	if (!World)
		return nullptr;
	
	const UNavigationSystemV1* navSys = 
		FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	
	if (!navSys)
		return nullptr;
	
	return Cast<ARecastNavMesh>(navSys->GetDefaultNavDataInstance());
}

/// @param A Should be in circle local space, meaning in the local_A = world_A - circle_center
/// @param B In circle local space too
/// @param R circle radius
/// @return The area between the segment and the edge of the circle
float SphWallLibrary::SegmentCircleArea(const FVector2D& A, const FVector2D& B, float R)
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
	d.Normalize();
	
	//Segment points projected on circle
	const FVector2D I0 = A + d * t0;
	const FVector2D I1 = A + d * t1;

	return SectorArea(I0, I1, R) - TriangleArea(I0, I1);
}

int SphWallLibrary::IntersectSegmentCircle(const FVector2D& A, const FVector2D& B, float R, float& t0, float& t1)
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

float SphWallLibrary::TriangleArea(const FVector2D& A, const FVector2D& B)
{
	return 0.5f * FVector2D::CrossProduct(A, B);
}

float SphWallLibrary::SectorArea(const FVector2D& A, const FVector2D& B, float R)
{
	float theta = AngleRad(A, B);
	return 0.5f * (R * R) * theta;
}

float SphWallLibrary::AngleRad(const FVector2D& A, const FVector2D& B)
{
	return FMath::Atan2(FVector2D::CrossProduct(A, B), FVector2D::DotProduct(A, B));
}