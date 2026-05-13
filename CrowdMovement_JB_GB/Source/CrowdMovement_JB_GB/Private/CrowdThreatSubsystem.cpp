// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdThreatSubsystem.h"

void UCrowdThreatSubsystem::RegisterThreat(AActor* Actor, float Radius, float Intensity, bool bGlobal)
{
	if (!Actor) return;
 
	FActiveThreatData Data;
	Data.Location  = Actor->GetActorLocation();
	Data.RadiusSq  = FMath::Square(Radius);
	Data.Intensity = Intensity;
	Data.bGlobal   = bGlobal;
 
	const int32 Idx = ActiveThreats.Add(Data);
	ActorToIndex.Add(Actor, Idx);
}

void UCrowdThreatSubsystem::UnregisterThreat(AActor* Actor)
{if (!Actor) return;
 
	int32* Idx = ActorToIndex.Find(Actor);
	if (!Idx) return;
 
	ActiveThreats.RemoveAt(*Idx);
	ActorToIndex.Remove(Actor);
	
	for (auto& Pair : ActorToIndex)
	{
		if (Pair.Value > *Idx) --Pair.Value;
	}

}

void UCrowdThreatSubsystem::UpdateThreat(AActor* Actor, bool bActive, FVector NewLocation)
{
	if (!Actor) return;
 
	if (bActive)
	{
		int32* Idx = ActorToIndex.Find(Actor);
		if (Idx && ActiveThreats.IsValidIndex(*Idx))
		{
			ActiveThreats[*Idx].Location = NewLocation;
		}
	}
	else
	{
		UnregisterThreat(Actor);
	}
}
