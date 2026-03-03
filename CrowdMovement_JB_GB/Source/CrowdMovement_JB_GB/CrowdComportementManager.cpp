// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdComportementManager.h"

// Sets default values
ACrowdComportementManager::ACrowdComportementManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACrowdComportementManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACrowdComportementManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

