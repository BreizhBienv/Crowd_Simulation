#include "CrowdThreatActor.h"
#include "Components/SphereComponent.h"
#include "CrowdThreatSubsystem.h"
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

void ACrowdThreatActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bIsActive)
	{
		if (UCrowdThreatSubsystem* Sub = GetWorld()->GetSubsystem<UCrowdThreatSubsystem>())
			Sub->UnregisterThreat(this);
	}
	Super::EndPlay(EndPlayReason);
}
 
void ACrowdThreatActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (ThreatSphere)
		ThreatSphere->SetSphereRadius(ThreatRadius);
}
 
void ACrowdThreatActor::ActivateThreat()
{
	if (bIsActive) return;
	bIsActive = true;
 
	if (UCrowdThreatSubsystem* Sub = GetWorld()->GetSubsystem<UCrowdThreatSubsystem>())
		Sub->RegisterThreat(this, ThreatRadius, ThreatIntensity, bGlobalThreat);
}
 
void ACrowdThreatActor::DeactivateThreat()
{
	if (!bIsActive) return;
	bIsActive = false;
 
	if (UCrowdThreatSubsystem* Sub = GetWorld()->GetSubsystem<UCrowdThreatSubsystem>())
		Sub->UnregisterThreat(this);
}
 
void ACrowdThreatActor::ToggleThreat()
{
	bIsActive ? DeactivateThreat() : ActivateThreat();
}



