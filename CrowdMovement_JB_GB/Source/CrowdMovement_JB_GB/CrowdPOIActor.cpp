// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdPOIActor.h"
#include "CrowdPOISubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/BillboardComponent.h"

// Sets default values
ACrowdPOIActor::ACrowdPOIActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AttractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttractionSphere"));
	AttractionSphere->SetSphereRadius(AttractionRadius);
	AttractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttractionSphere->SetVisibility(true);
	AttractionSphere->bHiddenInGame = true; // invisible en jeu, visible dans l'éditeur
	RootComponent = AttractionSphere;

	// Billboard pour repérer facilement le POI dans la scène
	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ACrowdPOIActor::BeginPlay()
{
	Super::BeginPlay();

	if (UCrowdPOISubsystem* Subsystem = GetWorld()->GetSubsystem<UCrowdPOISubsystem>())
	{
		Subsystem->RegisterPOI(this, GetActorLocation(), AttractionRadius, ObeservationTime);
	}
	
}

void ACrowdPOIActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UCrowdPOISubsystem* Subsystem = GetWorld()->GetSubsystem<UCrowdPOISubsystem>())
	{
		Subsystem->deletePOI(this);
	}

	Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
void ACrowdPOIActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (AttractionSphere)
	{
		AttractionSphere->SetSphereRadius(AttractionRadius);
	}
}
#endif


