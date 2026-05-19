#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassEntityHandle.h"
#include "SphAgent/SphObstacleSubSystem.h"
#include "CrowdFragments.generated.h"

UENUM(BlueprintType)
enum class EPersonalityType : uint8
{
	Calm	UMETA(DisplayName = "Calme"),
	Social	UMETA(DisplayName = "Sociale"),
	Nervous	UMETA(DisplayName = "Nerveux"),
	Leader	UMETA(DisplayName = "Leader"),
};

UENUM(BlueprintType)
enum class ECrowdPhase : uint8
{
	Passive_Walk,
	Passive_Observation,
	Passive_Discution,
	Active_Fleeing,
	Active_Fighting,
};

UENUM(BlueprintType)
enum class ECrewRole : uint8
{
	Pilot		UMETA(DisplayName = "Pilote"),
	Engineer	UMETA(DisplayName = "Ingenieur"),
	Medic		UMETA(DisplayName = "Medecin"),
	Guard		UMETA(DisplayName = "Garde"),
	Crew		UMETA(DisplayName = "Equipage"),
};

USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FCrowdAnimationFragment : public FMassFragment
{
	GENERATED_BODY()
	
	//blend Anim
	UPROPERTY()
    float CurrentSpeed = 0.f;
	
	UPROPERTY()
	ECrowdPhase AnimPhase = ECrowdPhase::Passive_Walk;
	
	UPROPERTY()
	ECrewRole AnimRole = ECrewRole::Crew;
	
	UPROPERTY()
	FName ActiveAnimState = NAME_None;
};


USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FCrowdFragments : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Personality")
	EPersonalityType Personality = EPersonalityType::Calm;

	UPROPERTY(EditAnywhere, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CourageScore = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CuriosityScore = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
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
	
	static constexpr int32 MaxNavPoints = 32;
	FVector NavPathPoints[MaxNavPoints] = {};
	
	UPROPERTY()
	int32 NavPathPointCount = 0;
	
	UPROPERTY()
	int32 CurrentNavPointIndex = 0;
	
	UPROPERTY()
	bool bHasNavPath = false;

	UPROPERTY()
	float ObservationTimer = 0.f;

	UPROPERTY()
	float ObservationDuration = 5.f;

	UPROPERTY()
	bool bIsObserving = false;
	
	UPROPERTY()
	bool bHasReservation = false;
};


USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FCrowdRoleFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Role")
	ECrewRole Role = ECrewRole::Crew;


	UPROPERTY()
	bool bOnDuty = true;


	UPROPERTY()
	float DutyTimer = 0.f;


	UPROPERTY()
	float DutyDuration = 120.f;
};


USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FCrowdSocialGroupFragment : public FMassFragment
{
	GENERATED_BODY()
	
	UPROPERTY()
	int32 GroupID = -1;


	FMassEntityHandle GroupLeader;

	UPROPERTY()
	bool bIsLeader = false;


	UPROPERTY()
	FVector SharedDestination = FVector::ZeroVector;


	UPROPERTY()
	float TalkTimer = 0.f;


	UPROPERTY()
	float TalkDuration = 0.f;
};



USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FSharedCrowdSettings : public FMassConstSharedFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Perception")
	float PerceptionRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Social")
	int32 MaxGroupSize = 4;

	UPROPERTY(EditAnywhere, Category = "Social", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GroupFormationChance = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Social", meta = (ClampMin = "100.0"))
	float WalkTogetherRadius = 350.f;

	UPROPERTY(EditAnywhere, Category = "Social", meta = (ClampMin = "50.0"))
	float TalkRadius = 120.f;

	UPROPERTY(EditAnywhere, Category = "Social", meta = (ClampMin = "1.0"))
	float TalkDurationMin = 5.f;

	UPROPERTY(EditAnywhere, Category = "Social", meta = (ClampMin = "1.0"))
	float TalkDurationMax = 20.f;

	UPROPERTY(EditAnywhere, Category = "Social", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float CatchUpSpeedMultiplier = 1.4f;
	
	UPROPERTY(EditAnywhere, Category = "Threat", meta = (ClampMin = "0.1"))
	float FleeingThreshold = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Threat", meta = (ClampMin = "0.1"))
	float ThreatAccumulationRate = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Threat", meta = (ClampMin = "100.0"))
	float ThreatDetectionRadius = 1500.f;
};

USTRUCT()
struct FSphSettingsFragment : public FMassFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="Crowd|Sph|Settings")
	bool randomizeSphValues = false;	
	

	UPROPERTY(EditAnywhere, Category="Crowd|Sph|Settings")
	FVector2D hRange = FVector2D(0.5f, 2.f);
	
	
	UPROPERTY(EditAnywhere, Category="Crowd|Sph|Settings")
	FVector2D massRange = FVector2D(50.f, 120.f);
	
	
	UPROPERTY(EditAnywhere, Category="Crowd|Sph|Settings")
	FVector2D comfortRange = FVector2D(3.f, 7.f);
	
	
	UPROPERTY(EditAnywhere, Category="Crowd|Sph|Settings")
	FVector2D coefRange = FVector2D(2.f, 4.f);
};

USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FSharedSphCrowdSettings : public FMassSharedFragment
{
	GENERATED_BODY()
	
	/// Pressure constant
	UPROPERTY(EditAnywhere, meta=(ClampMin=0, UIMin=0))
	float k = 5;
	
	UPROPERTY(EditAnywhere)
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

USTRUCT()
struct FSharedLocationFragment : public FMassSharedFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector location = FVector::ZeroVector;
};

namespace UE::Mass::ProcessorGroupNames
{
	const FName SphComponents = FName(TEXT("SphComponents"));
	const FName SphForces = FName(TEXT("SphForces"));
}
