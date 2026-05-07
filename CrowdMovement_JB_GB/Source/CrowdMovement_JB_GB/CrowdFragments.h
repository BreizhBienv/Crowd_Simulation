#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h" 
#include "MassCommonFragments.h"
#include "MassEntityHandle.h"
#include "SphAgent/SphObstacleSubSystem.h"
#include "CrowdFragments.generated.h"

//**
// create all the parameters need to simulate a crowd comportement 
//**

UENUM(BlueprintType)
enum class EPersonalityType :uint8
{
	Calm	UMETA(DisplayName = "Calme"),
	Social	UMETA(DisplayName = "Sociale"),
	Nervous UMETA(DisplayName = "Nerveux"),
	Leader  UMETA(DisplayName = "Leader"),
};

UENUM(BlueprintType)
enum class ECrowdPhase :uint8
{
	Passive_Walk,
	Passive_Observation,
	Passive_Discution,
	Active_Fleeing,
	Active_Fighting,
};




USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FCrowdFragments : public FMassFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Personality")
	EPersonalityType Personality = EPersonalityType::Calm;

	UPROPERTY(EditAnywhere, Category = "Personality")
	float CourageScore = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Personality")
	float CuriosityScore = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Personality")
	float SocialAffinityScore = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Personality")
	float SpeedBase = 150.f;

};


USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FCrowdPhaseFragments : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	ECrowdPhase CurrentPhase = ECrowdPhase::Passive_Walk;

	UPROPERTY()
	ECrowdPhase PreviousPhase = ECrowdPhase::Passive_Walk;

	UPROPERTY()
	float PhaseTimer = 0.f;

	UPROPERTY()
	float ThreatAwareness = 0.f;

};

USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FCrowdPOIFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	FVector TargetPOILocation = FVector::ZeroVector;

	UPROPERTY()
	TWeakObjectPtr<AActor> TargetPOIActor = nullptr;

	UPROPERTY()
	float ObservationTimer = 0.f;

	UPROPERTY()
	float ObservationDuration = 5.f;

	UPROPERTY()
	bool bIsObserving = false;
};

USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FCrowdBoidsFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	int32 GroupID = -1;

	UPROPERTY()
	bool bInGroup = false;

	FMassEntityHandle GroupLeader;

	UPROPERTY()
	float CompatibilityRadius = 300.f;

	UPROPERTY()
	float SocialUrge = 0.f;


};

USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FSharedCrowdSettings : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float PerceptionRadius = 500.f;

	UPROPERTY(EditAnywhere)
	int32 MaxGroupSize = 6;

	UPROPERTY(EditAnywhere)
	float BoidCohesionStrength = 0.4f;

	UPROPERTY(EditAnywhere)
	float BoidSeparationStrength = 0.8f;

	UPROPERTY(EditAnywhere)
	float BoidAlignmentStrength = 0.3f;

	UPROPERTY(EditAnywhere)
	int32 PersonalityCompatibilityThreshold = 1;
};

USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FSharedSphCrowdSettings : public FMassSharedFragment
{
	GENERATED_BODY()
	
	/// Pressure constant
	UPROPERTY(EditAnywhere, meta=(ClampMin=0, UIMin=0), Category="SPH")
	float k = 5;
	
	UPROPERTY(EditAnywhere, Category="SPH")
	float muViscosity = 10.f;
	
	TArray<FObstacleSegment> ObstaclesSegments;
};

USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FSphFragment : public FMassFragment
{
	GENERATED_BODY()
	
	/// The maximum distance in meters other particles are considered
	UPROPERTY(EditAnywhere, Category="SPH")
	float h = 2;
	
	/// The mass of the agent in kg
	UPROPERTY(EditAnywhere, Category="SPH")
	float mass = 60;
	
	/// Density in Kg/m^3 the particle is aiming for
	UPROPERTY(EditAnywhere, Category="SPH")
	float comfortDensity = 5;
	
	/// How much the agent is bothered by an obstacle
	UPROPERTY(EditAnywhere, Category="SPH")
	float obstacleAreaCoef = 3.f;
};

USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FSphForcesFragment : public FMassFragment
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, Category="SPH")
	float pressure = 0;
	
	UPROPERTY(VisibleAnywhere, Category="SPH")
	float density = 0;
	
	UPROPERTY(VisibleAnywhere, Category="SPH")
	FVector acceleration = FVector::ZeroVector;
};

namespace UE::Mass::ProcessorGroupNames
{
	const FName SphComponents = FName(TEXT("SphComponents"));
	const FName SphForces = FName(TEXT("SphForces"));
}
