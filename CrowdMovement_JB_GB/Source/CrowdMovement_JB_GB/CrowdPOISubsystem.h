// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdFragments.h"
#include "CrowdPOIActor.h"
#include "CrowdPOISubsystem.generated.h"

USTRUCT(BlueprintType)
struct FCrowdPOIData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	float AttirenceRaduis = 500.f;
	
	UPROPERTY()
	EPOIType Type = EPOIType::Lounge;
	
	UPROPERTY()
	float ObservationDuration = 5.f;
	
	UPROPERTY()
	int32 MaxOccupant = -1;
	
	UPROPERTY()
	int32 CurrOccupant = 0;

	UPROPERTY()
	TWeakObjectPtr<AActor> SourceActor = nullptr;
	
	UPROPERTY()
	int32 ReservedOccupant = 0;
	
	bool HasRoom() const
	{
		return MaxOccupant < 0 || (CurrOccupant + ReservedOccupant) < MaxOccupant;
	}

};

UCLASS()
class CROWDMOVEMENT_JB_GB_API UCrowdPOISubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterPOI(AActor* Actor, FVector Location, EPOIType type, float AttractionRadius, float ObservationDuration,  int32 MaxOccupant);

	void DeletePOI(AActor* toRemove);

	
	// Generic POI Get
	void GetNearPOI(const FVector& location, float raduis  ,TArray<FCrowdPOIData>& OutPOI)const;

	bool GetRandomNearPOI(const FVector& location, float raduis, FCrowdPOIData& OutPOI)const;
	
	// Specifique POI Get
	
	void GetEscapePodsOrderedByDistance(const FVector& Location, TArray<FCrowdPOIData>& OutPods) const;

	bool GetRandomPOIForRole(const FVector& Location, float Radius, ECrewRole Role, FCrowdPOIData& OutPOI) const;
 
	bool GetRandomSocialPOI(const FVector& Location, float Radius, FCrowdPOIData& OutPOI) const;
	
	bool GetNearestEscapePoint(const FVector& Location, FCrowdPOIData& OutPOI) const;
	
	bool HasRoomForOccupant(AActor* POIActor) const;
	
	void IncrementOccupants(AActor* POIActor);
	void DecrementOccupants(AActor* POIActor);
	
	bool TryReserveOccupant(AActor* POIActor);
	void ReleaseReservation(AActor* POIActor);
	void ConfirmReservationAsOccupant(AActor* POIActor);
	
private:
	TMap<EPOIType, TArray<FCrowdPOIData>> POIsByType;
	
	TArray<FCrowdPOIData> RegisteredPOIs;
	
	static TArray<EPOIType> GetPOITypesForRole(ECrewRole Role);
	
	static TArray<EPOIType> GetSocialPOITypes();
	
	bool PickRandom(const TArray<FCrowdPOIData>& Candidates, FCrowdPOIData& OutPOI) const;
	void CollectCandidates(const TArray<EPOIType>& Types, const FVector& Location,
						   float Radius, TArray<FCrowdPOIData>& OutCandidates) const;
};
