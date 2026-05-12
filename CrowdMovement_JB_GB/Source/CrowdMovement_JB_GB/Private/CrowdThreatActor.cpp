#include "CrowdThreatActor.h"
#include "Components/SphereComponent.h"
#include "Components/BillboardComponent.h"
 
ACrowdThreatActor::ACrowdThreatActor()
{
	PrimaryActorTick.bCanEverTick = false;
 
	ThreatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ThreatSphere"));
	ThreatSphere->SetSphereRadius(ThreatRadius);
	ThreatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ThreatSphere->bHiddenInGame = true;
	RootComponent = ThreatSphere;
 
	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetupAttachment(RootComponent);
}
 
void ACrowdThreatActor::BeginPlay()
{
	Super::BeginPlay();
}
 
void ACrowdThreatActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (ThreatSphere)
	{
		ThreatSphere->SetSphereRadius(ThreatRadius);
	}
}
 
void ACrowdThreatActor::ActivateThreat()
{
	bIsActive = true;
}
 
void ACrowdThreatActor::DeactivateThreat()
{
	bIsActive = false;
}
 
void ACrowdThreatActor::ToggleThreat()
{
	bIsActive = !bIsActive;
}



