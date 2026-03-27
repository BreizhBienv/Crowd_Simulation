// Fill out your copyright notice in the Description page of Project Settings.


#include "SphAgent/SphAgent.h"

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

void ASphAgent::ComputeSphForces()
{	
	FVector fPressure = FVector::ZeroVector;
	FVector fViscosity = FVector::ZeroVector;
	for (FSphAgentNeighbourData data : neighbours)
	{
		fPressure += ComputePressureFromNeighbour(data);
		fViscosity += ComputeViscosityFromNeighbour(data);
	}
	
	ComputeAcceleration(-fPressure, muViscosity * fViscosity);
}

void ASphAgent::ComputeRequiredComponents()
{
	ComputeDensity();
	ComputePressure();
}

void ASphAgent::ComputeDensity()
{
	localDensity = 0;

	FVector location = GetActorLocation();
	for (FSphAgentNeighbourData data : neighbours)
	{
		FVector dirVector = location - data.neighbour->GetActorLocation();
		localDensity += data.neighbour->mass * Poly6Kernel(dirVector, influenceDistance);
	}
}

void ASphAgent::ComputePressure()
{
	float substraction = FMath::Max(0, localDensity - comfortDensity);
	densityPressure = pressureRepulsionConstant * (substraction * substraction);
}

FVector ASphAgent::ComputePressureFromNeighbour(const FSphAgentNeighbourData& neighbour) const
{	
	FVector dirVector = GetActorLocation() - neighbour.neighbour->GetActorLocation();
	float factor = (densityPressure + neighbour.neighbour->densityPressure) / (2 * neighbour.neighbour->localDensity);
	return neighbour.neighbour->mass * factor * SpikyKernel(dirVector, influenceDistance);
}

FVector ASphAgent::ComputeViscosityFromNeighbour(const FSphAgentNeighbourData& neighbour) const
{
	FVector computeForce;
	
	FVector location = GetActorLocation();
	for (FSphAgentNeighbourData data : neighbours)
	{
		FVector dirVector = location - data.neighbour->GetActorLocation();
		FVector factor = (data.neighbour->GetVelocity() - GetVelocity()) / data.neighbour->localDensity;
		computeForce += data.neighbour->mass * factor * MullerKernel(dirVector, influenceDistance);
	}
	
	return muViscosity * computeForce;
}

void ASphAgent::ComputeAcceleration(const FVector& fPressure, const FVector& fViscosity)
{
	acceleration += (fPressure + fViscosity) / localDensity;
}

float ASphAgent::Poly6Kernel(const FVector& r, float h)
{
	return FMath::Abs(r.Length()) < h ? 
		4.f / (UE_PI * FMath::Pow(h, 8)) * FMath::Pow(h * h - r.Length() * r.Length(), 3) : 
		0;
}

FVector ASphAgent::SpikyKernel(const FVector& r, float h)
{
	return FMath::Abs(r.Length()) < h ? 
		-(30.f / UE_PI * FMath::Pow(h, 5)) * (r / r.Length()) * ((h - r.Length()) * (h - r.Length())) : 
		FVector::ZeroVector;
}

float ASphAgent::MullerKernel(const FVector& r, float h)
{
	return r.Length() < h ? 
		360.f / (29.f * UE_PI * FMath::Pow(h, 5)) * (h - r.Length()) : 
		0;
} 

