// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdPOIActor.generated.h"

UENUM(BlueprintType)
enum class EPOIType : uint8
{
	Cockpit			UMETA(DisplayName = "Cockpit"),
	EngineeringBay	UMETA(DisplayName = "Salle des machines"),
	MedBay			UMETA(DisplayName = "Infirmerie"),
	PatrolPoint		UMETA(DisplayName = "Point de patrouille"),
	
	Canteen			UMETA(DisplayName = "Cantine"),
	Dormitory		UMETA(DisplayName = "Dortoir"),
	Lounge			UMETA(DisplayName = "Salle commune"),
	
	EscapePod		UMETA(DisplayName = "Pod d evacuation"),
};
 

UCLASS()
class CROWDMOVEMENT_JB_GB_API ACrowdPOIActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ACrowdPOIActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#if WITH_EDITOR
	// Mise à jour de la sphère de debug dans l'éditeur quand on change les valeurs
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

public:	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "POI")
	EPOIType POIType = EPOIType::Lounge;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "POI", meta = (ClampMin = "100", ClampMax = "2000"))
	float AttractionRadius = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "POI", meta = (ClampMin = "1", ClampMax = "30"))
	float ObeservationTime = 3.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "POI",meta = (ClampMin = "1", ClampMax = "60"))
	int32 MaxOccupant = -1;
	

private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USphereComponent> AttractionSphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UBillboardComponent> Billboard;


};
