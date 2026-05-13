// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdThreatSubsystem.generated.h"

USTRUCT()
struct FActiveThreatData
{
	GENERATED_BODY()
 
	FVector Location  = FVector::ZeroVector;
	float   RadiusSq  = 0.f;
	float   Intensity = 1.f;
	bool    bGlobal   = false;
};

UCLASS()
class CROWDMOVEMENT_JB_GB_API UCrowdThreatSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	void RegisterThreat(AActor* Actor, float Radius, float Intensity, bool bGlobal);
	void UnregisterThreat(AActor* Actor);
	void UpdateThreat(AActor* Actor, bool bActive, FVector NewLocation);
	
	const TArray<FActiveThreatData>& GetActiveThreats() const { return ActiveThreats; }
	bool HasActiveThreats() const { return !ActiveThreats.IsEmpty(); }
 
private:
	TArray<FActiveThreatData> ActiveThreats;
	
	TMap<TWeakObjectPtr<AActor>, int32> ActorToIndex;
};
