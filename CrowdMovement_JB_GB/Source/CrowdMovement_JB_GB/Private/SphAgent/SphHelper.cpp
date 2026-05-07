
#include "SphAgent/SphHelper.h"

#include "SphAgent/SphObstacleHelper.h"
#include "SphAgent/SphObstacleSubSystem.h"

#define CONVERT_M_TO_CM 100.f

FVector2D SphHelper::ConvertVectorDimension(const FVector& vector)
{
	return FVector2D(vector.X / CONVERT_M_TO_CM, vector.Y / CONVERT_M_TO_CM);
}

float SphHelper::ComputeDensityFromNeighbour(const FVector2D& ri, const FVector2D& rj, const float h, const float mj)
{
	return mj * Poly6Kernel(ri - rj, h);
}

float SphHelper::ComputeDensityFromObstacles(const FVector2D& ri, const float h, const float comfortDensity,
	float obstacleAreaCoef, const TArray<FObstacleSegment>& obstacleSegments)
{
	float result = 0.f;
	for (const FObstacleSegment& segment : obstacleSegments)
	{
		FObstacleSegment newData;
		if (SphObstacleHelper::ComputeSegmentData(ri, h, segment, newData))
			result += ComputeDensityFromObstacle(ri, h, comfortDensity, obstacleAreaCoef, newData);
	}
	
	return result;
}

float SphHelper::ComputeDensityFromObstacle(const FVector2D& ri, const float h, const float comfortDensity, 
	const float obstacleAreaCoef, const FObstacleSegment& segment)
{
	return comfortDensity * segment.Area * obstacleAreaCoef * Poly6Kernel(ri - segment.Center, h);
}

float SphHelper::ComputePressure(const float density, const float comfortDensity, const float k)
{
	const float substraction = FMath::Max(0, density - comfortDensity);
	return k * substraction;
}

FVector2D SphHelper::ComputePressureFromObstacle(const FVector2D& ri, const float h, const float pressure, 
	const float obstacleAreaCoef, const FObstacleSegment& obstacle)
{
	return -pressure * obstacle.Area * obstacleAreaCoef * SpikyKernel(ri - obstacle.Center, h);
}

FVector2D SphHelper::ComputePressureFromObstacles(const FVector2D& ri, const float h, const float pressure, 
	const float obstacleAreaCoef, const TArray<FObstacleSegment>& obstacleSegments)
{
	FVector2D result = FVector2D::ZeroVector;
	for (const FObstacleSegment& segment : obstacleSegments)
	{
		FObstacleSegment newData;
		if (SphObstacleHelper::ComputeSegmentData(ri, h, segment, newData))
			result += ComputePressureFromObstacle(ri, h, pressure, obstacleAreaCoef, newData);
	}
	
	return result;
}

FVector2D SphHelper::ComputePressureFromNeighbour(const FVector2D& ri, const float pi, float h, const FVector2D& rj,
	const float pj, const float densityj, const float mj)
{
	const float factor = densityj ? (pi + pj) / (2 * densityj) : 0;
	return mj * factor * SpikyKernel(ri - rj, h);
}

FVector2D SphHelper::ComputeViscosityFromNeighbour(const FVector2D& ri, const FVector& vi, const float h, const FVector2D& rj,
                                                   const FVector& vj, const float densityj, const float mj)
{
	const FVector vel = vi / CONVERT_M_TO_CM;
	const FVector otherVel = vj / CONVERT_M_TO_CM;
	
	const FVector factor = densityj ? (otherVel - vel) / densityj : FVector::ZeroVector;
	const FVector2D vel2D(factor.X, factor.Y);
	
	return mj * vel2D * MullerKernel(ri - rj, h);
}

FVector SphHelper::ComputeAcceleration(const FVector2D& fPressure, const FVector2D& fViscosity, const float density)
{
	const FVector fPressure3D(fPressure.X, fPressure.Y, 0);
	const FVector fViscosity3D(fViscosity.X, fViscosity.Y, 0);
	return density > 0 ? ((fPressure3D + fViscosity3D) / density) * CONVERT_M_TO_CM : FVector::ZeroVector;
}

float SphHelper::Poly6Kernel(const FVector2D& r, const float h)
{
	return r.Length() < h ? 
		4.f / (UE_PI * FMath::Pow(h, 8)) * FMath::Pow(h * h - r.SquaredLength(), 3) : 
		0;
}

FVector2D SphHelper::SpikyKernel(const FVector2D& r, const float h)
{
	return r.Length() < h ? 
		-(30.f / (UE_PI * FMath::Pow(h, 5))) * (r / r.Length()) * ((h - r.Length()) * (h - r.Length())) : 
		FVector2D::ZeroVector;
}

float SphHelper::MullerKernel(const FVector2D& r, const float h)
{
	return r.Length() < h ? 
		360.f / (29.f * UE_PI * FMath::Pow(h, 5)) * (h - r.Length()) : 
		0;
}