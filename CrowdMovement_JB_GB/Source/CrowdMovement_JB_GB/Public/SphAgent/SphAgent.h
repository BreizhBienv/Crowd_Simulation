// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SphAgent.generated.h"

USTRUCT()
struct FSphAgentNeighbourData
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	class ASphAgent* neighbour;
};

UCLASS()
class CROWDMOVEMENT_JB_GB_API ASphAgent : public ACharacter
{
	GENERATED_BODY()

	/// The maximum distance other particles are considered
	UPROPERTY(EditAnywhere, Category="SPH")
	float h = 2;
	
	UPROPERTY(EditAnywhere, Category="SPH")
	float mass = 1;
	
	/// Density the particle is aiming for
	UPROPERTY(EditAnywhere, Category="SPH")
	float comfortDensity = 0;
	
	/// Pressure constant
	UPROPERTY(EditAnywhere, meta=(ClampMin=0, UIMin=0), Category="SPH")
	float k = 0;
	
	/// How much the agent is bothered by an obstacle
	UPROPERTY(EditAnywhere, Category="SPH")
	float obstacleAreaCoef = 1.f;
	
	UPROPERTY(EditAnywhere, Category="SPH")
	float muViscosity = 5.f;
	
	UPROPERTY(VisibleAnywhere, Category="SPH")
	float pressure = 0;
	
	UPROPERTY(VisibleAnywhere, Category="SPH")
	float density = 0;
	
	UPROPERTY(VisibleAnywhere, Category="SPH")
	TArray<FSphAgentNeighbourData> neighbours;
	
	UPROPERTY(VisibleAnywhere, Category="SPH")
	TArray<struct FObstacleSegment> wallSegments;
	
	UPROPERTY(VisibleAnywhere, Category="SPH")
	FVector acceleration = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, Category="SPH")
	bool debug = false;
	
public:
	// Sets default values for this character's properties
	ASphAgent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	float InfluenceDistance() const	{ return h;	}
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetNeighboursData(const TArray<FSphAgentNeighbourData>& data);
	
	void ComputeRequiredComponents(const TArray<struct FObstacleSegment>& obstacles);
	void ComputeSphForces(float DeltaTime);

protected:
	
	float ComputeDensityFromNeighbours() const;
	FVector2D ComputePressureFromNeighbours(const FSphAgentNeighbourData& neighbour) const;
	FVector2D ComputePressureFromObstacles() const;
	FVector2D ComputeViscosityFromNeighbours(const FSphAgentNeighbourData& neighbour) const;
	void ComputeAcceleration(const FVector2D& fPressure, const FVector2D& fViscosity);
	
};