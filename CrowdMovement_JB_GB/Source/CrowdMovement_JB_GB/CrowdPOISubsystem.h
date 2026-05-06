// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
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
	float ObservationDuration = 200.f;

	UPROPERTY()
	TWeakObjectPtr<AActor> SourceActor = nullptr;

};

UCLASS()
class CROWDMOVEMENT_JB_GB_API UCrowdPOISubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterPOI(AActor* Actor, FVector Location, float AttractionRadius, float ObservationDuration);

	void deletePOI(AActor* toRemove);

	void GetNearPOI(const FVector& location, float raduis  ,TArray<FCrowdPOIData>& OutPOI)const;

	bool GetRandomNearPOI(const FVector& location, float raduis, FCrowdPOIData& OutPOI)const;

private:
	TArray<FCrowdPOIData> RegisteredPOIs;
	
};
