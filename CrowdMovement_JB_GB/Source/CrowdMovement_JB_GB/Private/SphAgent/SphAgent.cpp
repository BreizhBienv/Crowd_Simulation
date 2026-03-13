// Fill out your copyright notice in the Description page of Project Settings.


#include "SphAgent/SphAgent.h"

// Sets default values
ASphAgent::ASphAgent()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASphAgent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASphAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASphAgent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

