// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdThreatActor.generated.h"

UCLASS()
class CROWDMOVEMENT_JB_GB_API ACrowdThreatActor : public AActor
{
	GENERATED_BODY()

public:
	ACrowdThreatActor();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threat", meta = (ClampMin = "100"))
	float ThreatRadius = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threat", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float ThreatIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threat")
	bool bGlobalThreat = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Threat")
	bool bIsActive = false;
	
	UFUNCTION(BlueprintCallable, Category = "Threat")
	void ActivateThreat();

	UFUNCTION(BlueprintCallable, Category = "Threat")
	void DeactivateThreat();

	UFUNCTION(BlueprintCallable, Category = "Threat")
	void ToggleThreat();
	
	FORCEINLINE bool    IsActive()    const { return bIsActive; }
	FORCEINLINE float   GetRadius()   const { return ThreatRadius; }
	FORCEINLINE float   GetIntensity()const { return ThreatIntensity; }
	FORCEINLINE bool    IsGlobal()    const { return bGlobalThreat; }
	FORCEINLINE FVector GetLocation() const { return GetActorLocation(); }

protected:
	virtual void BeginPlay() override;
	void EndPlay(EEndPlayReason::Type EndPlayReason);
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USphereComponent> ThreatSphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UBillboardComponent> Billboard;
};

	

