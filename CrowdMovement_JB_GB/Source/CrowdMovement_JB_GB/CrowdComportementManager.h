// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdComportementManager.generated.h"

enum class ECrowdPhase : uint8
{
	Passive,
	Active
};

enum class EReactionType : uint8
{
	None,
	Flee,
	Fight,
	Freeze
};

UCLASS()
class CROWDMOVEMENT_JB_GB_API ACrowdComportementManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACrowdComportementManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
