// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SphAgent.generated.h"

UCLASS()
class CROWDMOVEMENT_JB_GB_API ASphAgent : public ACharacter
{
	GENERATED_BODY()

	/* 
	 * The maximum distance other agents influence this agent 
	 */
	UPROPERTY(EditAnywhere)
	float influenceDistance;
	
public:
	// Sets default values for this character's properties
	ASphAgent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	float InfluenceDistance() const	{ return influenceDistance;	}
};

USTRUCT()
struct FSphAgentNeighboursData
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	ASphAgent* neighbour;
	
	UPROPERTY(VisibleAnywhere)
	float distance;
};