// Fill out your copyright notice in the Description page of Project Settings.

#include "SphAgent/SphAgent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "SphAgent/SphHelper.h"
#include "SphAgent/SphObstacleHelper.h"
#include "SphAgent/SphObstacleSubSystem.h"

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

void ASphAgent::ComputeRequiredComponents(const TArray<FObstacleSegment>& obstacles)
{
	const float densityFromNeighbours = ComputeDensityFromNeighbours();
	float densityFromObstacles = 0;
	
	const FVector2D ri = SphHelper::ConvertVectorDimension(GetActorLocation());
	
	wallSegments.Empty();
	for (const FObstacleSegment& segment : obstacles)
	{
		FObstacleSegment newData;
		if (SphObstacleHelper::ComputeSegmentData(ri, h, segment, newData))
		{
			wallSegments.Add(newData);
			densityFromObstacles += SphHelper::ComputeDensityFromObstacle(
				ri, 
				h, 
				comfortDensity, 
				obstacleAreaCoef, 
				newData);
		}
	}
	
	density = densityFromNeighbours + densityFromObstacles;
	pressure = SphHelper::ComputePressure(density, comfortDensity, k);
}

void ASphAgent::ComputeSphForces(float DeltaTime)
{
	const FVector2D ri = SphHelper::ConvertVectorDimension(GetActorLocation());
	const FVector vi = GetVelocity();
	
	const FVector2D fPressureFromObstacles = ComputePressureFromObstacles();
	FVector2D fPressureFromNeighbours = FVector2D::ZeroVector;
	FVector2D fViscosity = FVector2D::ZeroVector;
	for (FSphAgentNeighbourData data : neighbours)
	{
		const FVector2D rj = SphHelper::ConvertVectorDimension(data.neighbour->GetActorLocation());
		const FVector vj = data.neighbour->GetVelocity();
		
		//fPressureFromNeighbours += ComputePressureFromNeighbours(data);
		fPressureFromNeighbours += SphHelper::ComputePressureFromNeighbour(
			ri, pressure, h, 
			rj, data.neighbour->pressure, data.neighbour->density, data.neighbour->mass);
		//fViscosity += ComputeViscosityFromNeighbours(data);
		fViscosity += SphHelper::ComputeViscosityFromNeighbour(
			ri, vi, h, 
			rj, vj, data.neighbour->density, data.neighbour->mass);
	}
	
	ComputeAcceleration(-fPressureFromNeighbours + fPressureFromObstacles, muViscosity * fViscosity);
}

float ASphAgent::ComputeDensityFromNeighbours() const
{
	float result = 0;
	
	const FVector2D ri = SphHelper::ConvertVectorDimension(GetActorLocation());
	for (FSphAgentNeighbourData data : neighbours)
	{
		const FVector2D rj = SphHelper::ConvertVectorDimension(data.neighbour->GetActorLocation());
		result += SphHelper::ComputeDensityFromNeighbour(ri, rj, h, data.neighbour->mass);
	}
	
	return result;
}

FVector2D ASphAgent::ComputePressureFromNeighbours(const FSphAgentNeighbourData& neighbour) const
{
	const FVector2D dirVector = 
		SphHelper::ConvertVectorDimension(GetActorLocation()) - 
		SphHelper::ConvertVectorDimension(neighbour.neighbour->GetActorLocation());
	const float factor = neighbour.neighbour->density ? (pressure + neighbour.neighbour->pressure) / (2 * neighbour.neighbour->density) : 0;
	return neighbour.neighbour->mass * factor * SphHelper::SpikyKernel(dirVector, h);
}

FVector2D ASphAgent::ComputePressureFromObstacles() const
{
	FVector2D result = FVector2D::ZeroVector;

	const FVector2D ri = SphHelper::ConvertVectorDimension(GetActorLocation());
	for (const FObstacleSegment& wall : wallSegments)
		result += SphHelper::ComputePressureFromObstacle(ri, h, pressure, obstacleAreaCoef, wall);
	
	return result;
}

FVector2D ASphAgent::ComputeViscosityFromNeighbours(const FSphAgentNeighbourData& neighbour) const
{
	const FVector vel = GetVelocity() / CONVERT_M_TO_CM;
	const FVector otherVel = neighbour.neighbour->GetVelocity() / CONVERT_M_TO_CM;
	const FVector2D dirVector = 
		SphHelper::ConvertVectorDimension(GetActorLocation()) - 
		SphHelper::ConvertVectorDimension(neighbour.neighbour->GetActorLocation());
	
	const FVector factor = neighbour.neighbour->density ? (otherVel - vel) / neighbour.neighbour->density : FVector::ZeroVector;
	const FVector2D vel2D(factor.X, factor.Y);
	
	return neighbour.neighbour->mass * vel2D * SphHelper::MullerKernel(dirVector, h);
}

void ASphAgent::ComputeAcceleration(const FVector2D& fPressure, const FVector2D& fViscosity)
{
	acceleration = SphHelper::ComputeAcceleration(fPressure, fViscosity, density);
	
	float AccelerationMagnitude = acceleration.Size();
	FVector AccelerationDirection = acceleration.GetSafeNormal();
	GetCharacterMovement()->MaxAcceleration = AccelerationMagnitude;
	AddMovementInput(AccelerationDirection, 1.0f);
	
	if (debug && GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, acceleration.ToString());	
	
	const FVector normalizedAcceleration = acceleration.GetSafeNormal();
	const FVector normalizedVelocity = GetCharacterMovement()->Velocity.GetSafeNormal();
	
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + normalizedAcceleration * 100.f, FColor::Yellow);
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + normalizedVelocity * 100.f, FColor::Magenta);
}