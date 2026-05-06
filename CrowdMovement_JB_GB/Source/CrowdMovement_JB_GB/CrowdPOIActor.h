// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdPOIActor.generated.h"

UCLASS()
class CROWDMOVEMENT_JB_GB_API ACrowdPOIActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACrowdPOIActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#if WITH_EDITOR
	// Mise à jour de la sphère de debug dans l'éditeur quand on change les valeurs
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

public:	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "POI", meta = (ClampMin = "100", ClampMax = "2000"))
	float AttractionRadius = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "POI", meta = (ClampMin = "1", ClampMax = "30"))
	float ObeservationTime = 3.f;

private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USphereComponent> AttractionSphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UBillboardComponent> Billboard;


};
