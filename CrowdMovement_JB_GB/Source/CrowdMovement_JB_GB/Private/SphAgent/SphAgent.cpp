// Fill out your copyright notice in the Description page of Project Settings.

#include "SphAgent/SphAgent.h"
#include "SphAgent/SphWallLibrary.h"

// Sets default values
ASphAgent::ASphAgent()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASphAgent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASphAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASphAgent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ASphAgent::SetNeighboursData(const TArray<FSphAgentNeighbourData>& data)
{
	neighbours = data;
}

void ASphAgent::ComputeSphForces(const TArray<FWallSegment>& obstacles)
{	
	FVector2D fPressureFromObstacles = ComputePressureFromObstacles();
	FVector2D fPressureFromNeighbours = FVector2D::ZeroVector;
	FVector2D fViscosity = FVector2D::ZeroVector;
	for (FSphAgentNeighbourData data : neighbours)
	{
		fPressureFromNeighbours += ComputePressureFromNeighbours(data);
		fViscosity += ComputeViscosityFromNeighbours(data);
	}
	
	FVector2D fPressure = -fPressureFromObstacles + fPressureFromObstacles;
	
	ComputeAcceleration(-fPressure, muViscosity * fViscosity);
}

void ASphAgent::ComputeRequiredComponents(const TArray<FWallSegment>& obstacles)
{
	float densityFromNeighbours = ComputeDensityFromNeighbours();
	float densityFromObstacles = ComputeDensityFromObstacles(obstacles, wallSegments);
	sphDensity = densityFromNeighbours + densityFromObstacles;
	ComputePressure();
}

FVector2D ASphAgent::GetActorLocation2D(const AActor* actor)
{
	return FVector2D(actor->GetActorLocation().X, actor->GetActorLocation().Y);
}

float ASphAgent::ComputeDensityFromNeighbours() const
{
	float density = 0;

	FVector2D location = GetActorLocation2D(this);
	for (FSphAgentNeighbourData data : neighbours)
	{
		FVector2D dirVector = location - GetActorLocation2D(data.neighbour);
		density += data.neighbour->mass * Poly6Kernel(dirVector, influenceDistance);
	}
	
	return density;
}

float ASphAgent::ComputeDensityFromObstacles(const TArray<FWallSegment>& obstacles, TArray<FWallSegment>& registeredObstacles) const
{
	float density = 0.f;
	TArray<FWallSegment> outObstacles;
	
	FVector2D location(GetActorLocation().X, GetActorLocation().Y);
	for (const FWallSegment& wall : obstacles)
	{
		FVector2D closestPoint = FMath::ClosestPointOnSegment2D(location, wall.point0, wall.point1);
		FVector2D AgentToWall = closestPoint - location;
		float distToAgent = AgentToWall.Size();
		if (distToAgent > influenceDistance)
			continue;
		
		AgentToWall.Normalize();
		FVector2D densityCenterPoint = AgentToWall * (distToAgent + (influenceDistance - distToAgent) / 2.f);
		FVector2D dirVector = location - densityCenterPoint;
		float wallArea = SphWallLibrary::SegmentCircleArea(wall.point0 - location, wall.point1 - location, influenceDistance);
		density += comfortDensity * wallArea * Poly6Kernel(dirVector, influenceDistance);
		
		outObstacles.Add({densityCenterPoint, wallArea, wall.point0, wall.point1});
	}
	
	registeredObstacles = outObstacles;
	return density;
}

void ASphAgent::ComputePressure()
{
	float substraction = FMath::Max(0, sphDensity - comfortDensity);
	densityPressure = pressureRepulsionConstant * substraction;
}

FVector2D ASphAgent::ComputePressureFromNeighbours(const FSphAgentNeighbourData& neighbour) const
{	
	FVector2D dirVector = GetActorLocation2D(this) - GetActorLocation2D(neighbour.neighbour);
	float factor = (densityPressure + neighbour.neighbour->densityPressure) / (2 * neighbour.neighbour->sphDensity);
	return neighbour.neighbour->mass * factor * SpikyKernel(dirVector, influenceDistance);
}

FVector2D ASphAgent::ComputePressureFromObstacles() const
{
	FVector2D pressure = FVector2D::ZeroVector;
	
	FVector2D location(GetActorLocation().X, GetActorLocation().Y);
	for (const FWallSegment& wall : wallSegments)
	{
		FVector2D dirVector = location - wall.Center;
		pressure += -densityPressure * wall.Area * SpikyKernel(dirVector, influenceDistance);
	}
	
	return pressure;
}

FVector2D ASphAgent::ComputeViscosityFromNeighbours(const FSphAgentNeighbourData& neighbour) const
{
	FVector2D computeForce;
	
	FVector2D location = GetActorLocation2D(this);
	for (FSphAgentNeighbourData data : neighbours)
	{
		FVector2D dirVector = location - GetActorLocation2D(data.neighbour);
		FVector factor = (data.neighbour->GetVelocity() - GetVelocity()) / data.neighbour->sphDensity;
		FVector2D vel2D(factor.X, factor.Y);
		computeForce += data.neighbour->mass * vel2D * MullerKernel(dirVector, influenceDistance);
	}
	
	return muViscosity * computeForce;
}

void ASphAgent::ComputeAcceleration(const FVector2D& fPressure, const FVector2D& fViscosity)
{
	FVector fPressure3D(fPressure.X, fPressure.Y, 0);
	FVector fViscosity3D(fViscosity.X, fViscosity.Y, 0);
	acceleration += (fPressure3D + fViscosity3D) / sphDensity;
}

float ASphAgent::Poly6Kernel(const FVector2D& r, float h)
{
	return FMath::Abs(r.Length()) < h ? 
		4.f / (UE_PI * FMath::Pow(h, 8)) * FMath::Pow(h * h - r.Length() * r.Length(), 3) : 
		0;
}

FVector2D ASphAgent::SpikyKernel(const FVector2D& r, float h)
{
	return FMath::Abs(r.Length()) < h ? 
		-(30.f / UE_PI * FMath::Pow(h, 5)) * (r / r.Length()) * ((h - r.Length()) * (h - r.Length())) : 
		FVector2D::ZeroVector;
}

float ASphAgent::MullerKernel(const FVector2D& r, float h)
{
	return r.Length() < h ? 
		360.f / (29.f * UE_PI * FMath::Pow(h, 5)) * (h - r.Length()) : 
		0;
} 

