// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdPOISubsystem.h"

void UCrowdPOISubsystem::RegisterPOI(AActor* Actor, FVector Location,EPOIType type ,float AttractionRadius, float ObservationDuration, int32 MaxOccupant)
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
	newPOI.MaxOccupant = MaxOccupant;
	newPOI.Type = type;
	newPOI.CurrOccupant = 0;

	RegisteredPOIs.Add(newPOI);
}

void UCrowdPOISubsystem::DeletePOI(AActor * toRemove)
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

	for (const FCrowdPOIData& POI : RegisteredPOIs)
	{
		const float Dist = FVector::Dist2D(location, POI.Location);
		if (Dist <= raduis && Dist <= POI.AttirenceRaduis)
		{
			if (POI.MaxOccupant < 0 || POI.CurrOccupant < POI.MaxOccupant)
			{
				OutPOI.Add(POI);
			}
		}
	}
}

bool UCrowdPOISubsystem::GetRandomNearPOI(const FVector & location, float raduis, FCrowdPOIData & OutPOI) const
{
	TArray<FCrowdPOIData> Candidates;
	GetNearPOI(location, raduis, Candidates);
	if (Candidates.IsEmpty()) return false;
 
	OutPOI = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
	return true;
}

bool UCrowdPOISubsystem::GetRandomPOIForRole(const FVector& Location, float Radius, ECrewRole Role,
	FCrowdPOIData& OutPOI) const
{
	TArray<FCrowdPOIData>outPOIs;
	TArray<EPOIType> CompatibleType = GetPOITypesForRole(Role);
	
	for (const FCrowdPOIData& POI : RegisteredPOIs)
	{
		if (!CompatibleType.Contains(POI.Type))continue;
		
		const float Dist = FVector::Dist2D(Location, POI.Location);
		if (Dist > Radius) continue;
 
		if (POI.MaxOccupant >= 0 && POI.CurrOccupant >= POI.MaxOccupant) continue;
 
		outPOIs.Add(POI);
		
	}
	if (outPOIs.IsEmpty())
	{
		for (const FCrowdPOIData& POI : RegisteredPOIs)
		{
			if (!CompatibleType.Contains(POI.Type)) continue;
			if (POI.MaxOccupant >= 0 && POI.CurrOccupant >= POI.MaxOccupant) continue;
			outPOIs.Add(POI);
		}
	}
 
	if (outPOIs.IsEmpty()) return false;
 
	OutPOI = outPOIs[FMath::RandRange(0, outPOIs.Num() - 1)];
	return true;
}

bool UCrowdPOISubsystem::GetRandomSocialPOI(const FVector& Location, float Radius, FCrowdPOIData& OutPOI) const
{
	const TArray<EPOIType> SocialTypes = GetSocialPOITypes();
 
	TArray<FCrowdPOIData> Candidates;
	for (const FCrowdPOIData& POI : RegisteredPOIs)
	{
		if (!SocialTypes.Contains(POI.Type)) continue;
 
		const float Dist = FVector::Dist2D(Location, POI.Location);
		if (Dist > Radius) continue;
 
		if (POI.MaxOccupant >= 0 && POI.CurrOccupant >= POI.MaxOccupant) continue;
 
		Candidates.Add(POI);
	}
 
	if (Candidates.IsEmpty())
	{
		for (const FCrowdPOIData& POI : RegisteredPOIs)
		{
			if (!SocialTypes.Contains(POI.Type)) continue;
			Candidates.Add(POI);
		}
	}
 
	if (Candidates.IsEmpty()) return false;
 
	OutPOI = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
	return true;
}

bool UCrowdPOISubsystem::GetNearestEscapePoint(const FVector& Location, FCrowdPOIData& OutPOI) const
{
	float   BestDist = FLT_MAX;
	bool    bFound   = false;
 
	for (const FCrowdPOIData& POI : RegisteredPOIs)
	{
		if (POI.Type != EPOIType::EscapePod) continue;
 
		const float Dist = FVector::Dist(Location, POI.Location);
		if (Dist < BestDist)
		{
			BestDist = Dist;
			OutPOI   = POI;
			bFound   = true;
		}
	}
	return bFound;
}

void UCrowdPOISubsystem::IncrementOccupants(AActor* POIActor)
{
	for (FCrowdPOIData& POI : RegisteredPOIs)
	{
		if (POI.SourceActor == POIActor)
		{
			POI.CurrOccupant++;
			return;
		}
	}
}

void UCrowdPOISubsystem::DecrementOccupants(AActor* POIActor)
{
	for (FCrowdPOIData& POI : RegisteredPOIs)
	{
		if (POI.SourceActor == POIActor)
		{
			POI.CurrOccupant = FMath::Max(0, POI.CurrOccupant - 1);
			return;
		}
	}
}

TArray<EPOIType> UCrowdPOISubsystem::GetPOITypesForRole(ECrewRole Role)
{
	switch (Role)
	{
	case ECrewRole::Pilot:
		return { EPOIType::Cockpit };
 
	case ECrewRole::Engineer:
		return { EPOIType::EngineeringBay };
 
	case ECrewRole::Medic:
		return { EPOIType::MedBay };
 
	case ECrewRole::Guard:
		return { EPOIType::PatrolPoint };
 
	case ECrewRole::Crew:
	default:
		return { EPOIType::Canteen, EPOIType::Lounge, EPOIType::Dormitory };
	}
}

TArray<EPOIType> UCrowdPOISubsystem::GetSocialPOITypes()
{
	return { EPOIType::Canteen, EPOIType::Lounge, EPOIType::Dormitory };
}
