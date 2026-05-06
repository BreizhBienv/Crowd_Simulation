// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdPOISubsystem.h"

void UCrowdPOISubsystem::RegisterPOI(AActor* Actor, FVector Location, float AttractionRadius, float ObservationDuration)
{
	if (!Actor)return;

	for (const FCrowdPOIData& POI : RegisteredPOIs)
	{
		if (POI.SourceActor == Actor) return;
	}

	FCrowdPOIData newPOI;

	newPOI.SourceActor = Actor;
	newPOI.Location = Location;
	newPOI.AttirenceRaduis = AttractionRadius;
	newPOI.ObservationDuration = ObservationDuration;

	RegisteredPOIs.Add(newPOI);
}

void UCrowdPOISubsystem::deletePOI(AActor * toRemove)
{
	if (!toRemove) return;

	RegisteredPOIs.RemoveAll([toRemove](const FCrowdPOIData& POI)
	{
		return POI.SourceActor == toRemove;
	});
}

void UCrowdPOISubsystem::GetNearPOI(const FVector & location, float raduis, TArray<FCrowdPOIData>& OutPOI) const
{
	OutPOI.Reset();

	for (FCrowdPOIData POI : RegisteredPOIs)
	{
		const float dist = FVector::Dist2D(location, POI.Location);
		if (dist <= raduis && dist <= POI.AttirenceRaduis)
			OutPOI.Add(POI);
	}
}

bool UCrowdPOISubsystem::GetRandomNearPOI(const FVector & location, float raduis, FCrowdPOIData & OutPOI) const
{
	TArray<FCrowdPOIData>outPOIs;
	GetNearPOI(location, raduis, outPOIs);

	if (outPOIs.IsEmpty()) return false;

	const int32 RandomIndex = FMath::RandRange(0, outPOIs.Num() - 1 );

	OutPOI = outPOIs[RandomIndex];
	return true;
}
