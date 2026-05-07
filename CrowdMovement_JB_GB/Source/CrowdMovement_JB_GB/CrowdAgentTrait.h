// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "CrowdFragments.h"
#include "CrowdAgentTrait.generated.h"

/**
 declare tous les fragment des entiter et ramdomise 
 les valeurs de personalité
 **/
UCLASS(meta = (DisplayName = "Crowd Agent Trait"))
class CROWDMOVEMENT_JB_GB_API UCrowdAgentTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	// ship function 
	
	UPROPERTY(EditAnywhere, Category = "Crowd|Role")
	ECrewRole DefaultRole = ECrewRole::Crew;
 
	UPROPERTY(EditAnywhere, Category = "Crowd|Role")
	bool bRandomizeRole = false;
	
	// personality 
	UPROPERTY(EditAnywhere, Category = "Crowd|Personality")
	EPersonalityType DefaultPersonality = EPersonalityType::Calm;

	UPROPERTY(EditAnywhere, Category = "Crowd|Personality")
	bool bRandomizePersonality = true;

	//random personality param 

	UPROPERTY(EditAnywhere, Category = "Crowd|Personality", 
		meta = (EditCondition = "bRandomizePersonality", ClampMin = "0", ClampMax = "1"))
	FVector2D courageRange = FVector2D(0.1f, 0.9f);

	UPROPERTY(EditAnywhere, Category = "Crowd|Personality",
		meta = (EditCondition = "bRandomizePersonality", ClampMin = "0", ClampMax = "1"))
	FVector2D curiosityRange = FVector2D(0.1f, 0.9f);

	UPROPERTY(EditAnywhere, Category = "Crowd|Personality",
		meta = (EditCondition = "bRandomizePersonality", ClampMin = "0", ClampMax = "1"))
	FVector2D socialAffinityRange = FVector2D(0.2f, 0.8f);

	UPROPERTY(EditAnywhere, Category = "Crowd|Personality",
		meta = (EditCondition = "bRandomizePersonality", ClampMin = "0", ClampMax = "1"))
	FVector2D speedRange = FVector2D(120.f, 200.f);

	UPROPERTY(EditAnywhere, Category = "Crowd|Settings")
	FSharedCrowdSettings SharedSettings;
	
	UPROPERTY(EditAnywhere, Category = "Crowd|SphSettings")
	FSharedSphCrowdSettings SphCrowdSettings;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext,
		const UWorld& World) const override;
};
