// Fill out your copyright notice in the Description page of Project Settings.

#include "SphAgent/SphAgent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "SphAgent/SphWallLibrary.h"

#define CONVERT_M_TO_CM 100.f

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

void ASphAgent::ComputeRequiredComponents(const TArray<FWallSegment>& obstacles)
{
	const float densityFromNeighbours = ComputeDensityFromNeighbours();
	float densityFromObstacles = 0;
	
	wallSegments.Empty();
	for (const FWallSegment& segment : obstacles)
	{
		FWallSegment newData;
		if (ComputeSegmentData(segment, newData))
		{
			wallSegments.Add(newData);
			densityFromObstacles += ComputeDensityFromSegment(newData);
		}
	}
	
	density = densityFromNeighbours + densityFromObstacles;
	ComputePressure();
}

void ASphAgent::ComputeSphForces(float DeltaTime)
{
	const FVector2D fPressureFromObstacles = ComputePressureFromObstacles();
	FVector2D fPressureFromNeighbours = FVector2D::ZeroVector;
	FVector2D fViscosity = FVector2D::ZeroVector;
	for (FSphAgentNeighbourData data : neighbours)
	{
		fPressureFromNeighbours += ComputePressureFromNeighbours(data);
		fViscosity += ComputeViscosityFromNeighbours(data);
	}
	
	ComputeAcceleration(-fPressureFromNeighbours + fPressureFromObstacles, muViscosity * fViscosity, DeltaTime);
}

FVector2D ASphAgent::GetActorLocation2D(const AActor* actor)
{
	return FVector2D(actor->GetActorLocation().X / CONVERT_M_TO_CM, actor->GetActorLocation().Y / CONVERT_M_TO_CM);
}

float ASphAgent::ComputeDensityFromNeighbours() const
{
	float result = 0;
	
	const FVector2D location = GetActorLocation2D(this);
	for (FSphAgentNeighbourData data : neighbours)
	{
		FVector2D dirVector = location - GetActorLocation2D(data.neighbour);
		result += data.neighbour->mass * Poly6Kernel(dirVector, influenceDistance);
	}
	
	return result;
}

bool ASphAgent::ComputeSegmentData(const FWallSegment& segment, FWallSegment& outData) const
{	
	const FVector2D location = GetActorLocation2D(this);
	
	FVector2D closestPoint = FMath::ClosestPointOnSegment2D(location, segment.point0, segment.point1);
	FVector2D AgentToWall = closestPoint - location;
	const float distToAgent = AgentToWall.Size();
	if (distToAgent > influenceDistance)
		return false;
	
	AgentToWall.Normalize();
	const FVector2D densityCenterPoint = location + (AgentToWall * (distToAgent + (influenceDistance - distToAgent) / 2.f));
	const float area = SphWallLibrary::SegmentCircleArea(segment.point0 - location, segment.point1 - location, influenceDistance, GetWorld());
	
	// DrawDebugCircle(GetWorld(), GetActorLocation(), influenceDistance * CONVERT_M_TO_CM, 16, FColor::Blue, false, -1, 0, 0, FVector::ForwardVector, FVector::RightVector);
	// DrawDebugSphere(GetWorld(), FVector(densityCenterPoint.X * CONVERT_M_TO_CM, densityCenterPoint.Y * CONVERT_M_TO_CM, 0.f), 50.f, 16, FColor::Blue);
	// DrawDebugSphere(GetWorld(), FVector(segment.point0.X * CONVERT_M_TO_CM, segment.point0.Y * CONVERT_M_TO_CM, 0.f), 5.f, 16, FColor::Red);
	// DrawDebugSphere(GetWorld(), FVector(segment.point1.X * CONVERT_M_TO_CM, segment.point1.Y * CONVERT_M_TO_CM, 0.f), 5.f, 16, FColor::Red);
	
	FWallSegment registeredWallSegment;
	
	registeredWallSegment.point0 = segment.point0;
	registeredWallSegment.point1 = segment.point1;
	registeredWallSegment.Center = densityCenterPoint;
	registeredWallSegment.Area = area;
	
	outData = registeredWallSegment;
	return true;
}

float ASphAgent::ComputeDensityFromSegment(const FWallSegment& segment) const
{
	const FVector2D location = GetActorLocation2D(this);
	const FVector2D dirVector = location - segment.Center;
	return comfortDensity * segment.Area * obstacleAreaCoef * Poly6Kernel(dirVector, influenceDistance);
}

void ASphAgent::ComputePressure()
{
	const float substraction = FMath::Max(0, density - comfortDensity);
	pressure = k * substraction;
}

FVector2D ASphAgent::ComputePressureFromNeighbours(const FSphAgentNeighbourData& neighbour) const
{
	const FVector2D dirVector = GetActorLocation2D(this) - GetActorLocation2D(neighbour.neighbour);
	const float factor = neighbour.neighbour->density ? (pressure + neighbour.neighbour->pressure) / (2 * neighbour.neighbour->density) : 0;
	return neighbour.neighbour->mass * factor * SpikyKernel(dirVector, influenceDistance);
}

FVector2D ASphAgent::ComputePressureFromObstacles() const
{
	FVector2D result = FVector2D::ZeroVector;

	const FVector2D location = GetActorLocation2D(this);
	for (const FWallSegment& wall : wallSegments)
	{
		const FVector2D dirVector = location - wall.Center;
		result += -pressure * wall.Area * obstacleAreaCoef * SpikyKernel(dirVector, influenceDistance);
	}
	
	return result;
}

FVector2D ASphAgent::ComputeViscosityFromNeighbours(const FSphAgentNeighbourData& neighbour) const
{
	const FVector vel = GetVelocity() / CONVERT_M_TO_CM;
	const FVector otherVel = neighbour.neighbour->GetVelocity() / CONVERT_M_TO_CM;
	const FVector2D dirVector = GetActorLocation2D(this) - GetActorLocation2D(neighbour.neighbour);
	
	const FVector factor = neighbour.neighbour->density ? (otherVel - vel) / neighbour.neighbour->density : FVector::ZeroVector;
	const FVector2D vel2D(factor.X, factor.Y);
	
	return neighbour.neighbour->mass * vel2D * MullerKernel(dirVector, influenceDistance);
}

void ASphAgent::ComputeAcceleration(const FVector2D& fPressure, const FVector2D& fViscosity, float DeltaTime)
{
	const FVector fPressure3D(fPressure.X, fPressure.Y, 0);
	const FVector fViscosity3D(fViscosity.X, fViscosity.Y, 0);
	acceleration = density > 0 ? ((fPressure3D + fViscosity3D) / density) * CONVERT_M_TO_CM : FVector::ZeroVector;
	
	float AccelerationMagnitude = acceleration.Size();
	FVector AccelerationDirection = acceleration.GetSafeNormal();
	GetCharacterMovement()->MaxAcceleration = AccelerationMagnitude;
	AddMovementInput(AccelerationDirection, 1.0f);
	
	if (debug && GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, (acceleration * DeltaTime).ToString());	
	
	const FVector normalizedAcceleration = acceleration.GetSafeNormal();
	const FVector normalizedVelocity = GetCharacterMovement()->Velocity.GetSafeNormal();
	
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + normalizedAcceleration * 100.f, FColor::Yellow);
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + normalizedVelocity * 100.f, FColor::Magenta);
}

float ASphAgent::Poly6Kernel(const FVector2D& r, float h)
{
	return r.Length() < h ? 
		4.f / (UE_PI * FMath::Pow(h, 8)) * FMath::Pow(h * h - r.SquaredLength(), 3) : 
		0;
}

FVector2D ASphAgent::SpikyKernel(const FVector2D& r, float h)
{
	return r.Length() < h ? 
		-(30.f / (UE_PI * FMath::Pow(h, 5))) * (r / r.Length()) * ((h - r.Length()) * (h - r.Length())) : 
		FVector2D::ZeroVector;
}

float ASphAgent::MullerKernel(const FVector2D& r, float h)
{
	return r.Length() < h ? 
		360.f / (29.f * UE_PI * FMath::Pow(h, 5)) * (h - r.Length()) : 
		0;
}