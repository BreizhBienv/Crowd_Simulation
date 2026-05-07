#pragma once

#include "CoreMinimal.h"

struct FObstacleSegment;

class CROWDMOVEMENT_JB_GB_API SphHelper
{
public:
	
	/// Convert FVector in cm to FVector2D in meter
	static FVector2D ConvertVectorDimension(const FVector& vector);

	static float ComputeDensityFromNeighbour(const FVector2D& ri, const FVector2D& rj, float h, float mj);
	static float ComputeDensityFromObstacles(const FVector2D& ri, float h, float comfortDensity, float obstacleAreaCoef, 
		const TArray<FObstacleSegment>& obstacleSegments);
	static float ComputeDensityFromObstacle(const FVector2D& ri, float h, float comfortDensity, float obstacleAreaCoef, 
		const FObstacleSegment& segment);
	
	static float ComputePressure(float density, float comfortDensity, float k);
	
	static FVector2D ComputePressureFromObstacle(const FVector2D& ri, float h, float pressure, float obstacleAreaCoef, 
		const FObstacleSegment& obstacle);
	static FVector2D ComputePressureFromObstacles(const FVector2D& ri, float h, float pressure, float obstacleAreaCoef,
		const TArray<FObstacleSegment>& obstacleSegments);
	static FVector2D ComputePressureFromNeighbour(const FVector2D& ri, const float pi, float h, const FVector2D& rj, 
		const float pj, const float densityj, float mj);
	static FVector2D ComputeViscosityFromNeighbour(const FVector2D& ri, const FVector& vi, float h, const FVector2D& rj,
	                                               const FVector& vj, float densityj, float mj);
	
	static FVector ComputeAcceleration(const FVector2D& fPressure, const FVector2D& fViscosity, const float density);
	
	/// Used for the local density, takes r and h in m and not cm
	/// @param r direction vector with its distance between the agent and it's neighbour
	/// @param h distance of influence of the agent
	/// @return 
	static float Poly6Kernel(const FVector2D& r, float h);

	/// Used as gradient for the pressure force, takes r and h in m and not cm
	/// @param r direction vector with its distance between the agent and it's neighbour
	/// @param h distance of influence of the agent
	/// @return 
	static FVector2D SpikyKernel(const FVector2D& r, float h);

	/// Used as gradient for the viscosity, takes r and h in m and not cm
	/// @param r direction vector with its distance between the agent and it's neighbour
	/// @param h distance of influence of the agent
	/// @return 
	static float MullerKernel(const FVector2D& r, float h);
};
