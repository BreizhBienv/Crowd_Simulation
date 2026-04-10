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

	/* 
	 * The maximum distance other agents influence this agent 
	 */
	UPROPERTY(EditAnywhere)
	float influenceDistance = 100;
	
	UPROPERTY(EditAnywhere)
	float mass = 1;
	
	UPROPERTY(EditAnywhere)
	float comfortDensity = 0;
	
	UPROPERTY(EditAnywhere, meta=(ClampMin=0, UIMin=0))
	float pressureRepulsionConstant = 0;
	
	UPROPERTY(EditAnywhere)
	float muViscosity = 5.f;
	
	UPROPERTY(VisibleAnywhere)
	float densityPressure = 0;
	
	UPROPERTY(VisibleAnywhere)
	float sphDensity = 0;
	
	UPROPERTY(VisibleAnywhere)
	TArray<FSphAgentNeighbourData> neighbours;
	
	UPROPERTY(VisibleAnywhere)
	TArray<struct FWallSegment> wallSegments;
	
	UPROPERTY(VisibleAnywhere)
	FVector acceleration = FVector::ZeroVector;
	
public:
	// Sets default values for this character's properties
	ASphAgent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	float InfluenceDistance() const	{ return influenceDistance;	}
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetNeighboursData(const TArray<FSphAgentNeighbourData>& data);
	
	void ComputeSphForces(const TArray<struct FWallSegment>& obstacles);
	void ComputeRequiredComponents(const TArray<struct FWallSegment>& obstacles);

protected:
	
	static FVector2D GetActorLocation2D(const AActor* actor);
	float ComputeDensityFromNeighbours() const;
	float ComputeDensityFromObstacles(const TArray<FWallSegment>& obstacles, TArray<FWallSegment>& registeredObstacles) const;
	void ComputePressure();
	FVector2D ComputePressureFromNeighbours(const FSphAgentNeighbourData& neighbour) const;
	FVector2D ComputePressureFromObstacles() const;
	FVector2D ComputeViscosityFromNeighbours(const FSphAgentNeighbourData& neighbour) const;
	void ComputeAcceleration(const FVector2D& fPressure, const FVector2D& fViscosity);
	void ComputeObstacleRepulsion();
	void ComputeDesiredVelocity();

	/// Used for the local density
	/// @param r direction vector with its distance between the agent and it's neighbour
	/// @param h distance of influence of the agent
	/// @return 
	static float Poly6Kernel(const FVector2D& r, float h);

	/// Used as gradient for the pressure force
	/// @param r direction vector with its distance between the agent and it's neighbour
	/// @param h distance of influence of the agent
	/// @return 
	static FVector2D SpikyKernel(const FVector2D& r, float h);

	/// Used as gradient for the viscosity
	/// @param r direction vector with its distance between the agent and it's neighbour
	/// @param h distance of influence of the agent
	/// @return 
	static float MullerKernel(const FVector2D& r, float h);
	
};