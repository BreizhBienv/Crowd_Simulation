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
	if (!toRemove) 
		return;
	
	int32 RemovedIdx = RegisteredPOIs.IndexOfByPredicate(
		[toRemove](const FCrowdPOIData& P)
		{ return P.SourceActor == toRemove; });
	
	if (RemovedIdx == INDEX_NONE) 
		return;
	
	RegisteredPOIs.RemoveAt(RemovedIdx);
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
	return PickRandom(Candidates, OutPOI);
}

void UCrowdPOISubsystem::GetEscapePodsOrderedByDistance(const FVector& Location, TArray<FCrowdPOIData>& OutPods) const
{
	OutPods.Reset();

	for (const FCrowdPOIData& POI : RegisteredPOIs)
	{
		if (POI.Type == EPOIType::EscapePod)
			OutPods.Add(POI);
	}
	
	OutPods.Sort([&Location](const FCrowdPOIData& A, const FCrowdPOIData& B)
	{
		return FVector::Dist2D(Location, A.Location) < FVector::Dist2D(Location, B.Location);
	});
}

bool UCrowdPOISubsystem::GetRandomPOIForRole(const FVector& Location, float Radius, ECrewRole Role,
                                             FCrowdPOIData& OutPOI) const
{
	const TArray<EPOIType> ValidTypes = GetPOITypesForRole(Role);
	
	TArray<FCrowdPOIData> Candidates;
	CollectCandidates(ValidTypes, Location, Radius, Candidates);
	return PickRandom(Candidates, OutPOI);
}

bool UCrowdPOISubsystem::GetRandomSocialPOI(const FVector& Location, float Radius, FCrowdPOIData& OutPOI) const
{
	const TArray<EPOIType> SocialTypes = GetSocialPOITypes();
 
	TArray<FCrowdPOIData> Candidates;
	CollectCandidates(SocialTypes, Location, Radius, Candidates);
	return PickRandom(Candidates, OutPOI);
}

bool UCrowdPOISubsystem::GetNearestEscapePoint(const FVector& Location, FCrowdPOIData& OutPOI) const
{
	const TArray<FCrowdPOIData>* EscapeBucket = POIsByType.Find(EPOIType::EscapePod);
	if (!EscapeBucket || EscapeBucket->IsEmpty()) return false;
 
	float BestDist = FLT_MAX;
	bool  bFound   = false;
 
	for (const FCrowdPOIData& POI : *EscapeBucket)
	{
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

bool UCrowdPOISubsystem::HasRoomForOccupant(AActor* POIActor) const
{
	for (const FCrowdPOIData& POI : RegisteredPOIs)
	{
		if (POI.SourceActor == POIActor)
			return POI.MaxOccupant < 0 || POI.CurrOccupant < POI.MaxOccupant;
	}
	return true;
}

void UCrowdPOISubsystem::IncrementOccupants(AActor* POIActor)
{
	for (FCrowdPOIData& POI : RegisteredPOIs)
	{
		if (POI.SourceActor == POIActor) 
			{ POI.CurrOccupant++; return; }
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

bool UCrowdPOISubsystem::TryReserveOccupant(AActor* POIActor)
{
	for (FCrowdPOIData& POI : RegisteredPOIs)
	{
		if (POI.SourceActor != POIActor) continue;
		if (!POI.HasRoom()) return false;
		POI.ReservedOccupant++;
		return true;
	}
	return false;
}

void UCrowdPOISubsystem::ReleaseReservation(AActor* POIActor)
{
	for (FCrowdPOIData& POI : RegisteredPOIs)
	{
		if (POI.SourceActor == POIActor)
		{
			POI.ReservedOccupant = FMath::Max(0, POI.ReservedOccupant - 1);
			return;
		}
	}
}

void UCrowdPOISubsystem::ConfirmReservationAsOccupant(AActor* POIActor)
{
	for (FCrowdPOIData& POI : RegisteredPOIs)
	{
		if (POI.SourceActor == POIActor)
		{
			// La réservation se transforme en occupation réelle
			POI.ReservedOccupant = FMath::Max(0, POI.ReservedOccupant - 1);
			POI.CurrOccupant++;
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

bool UCrowdPOISubsystem::PickRandom(const TArray<FCrowdPOIData>& Candidates, FCrowdPOIData& OutPOI) const
{
	if (Candidates.IsEmpty()) 
		return false;
	
	OutPOI = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
	
	return true;
}

void UCrowdPOISubsystem::CollectCandidates(const TArray<EPOIType>& Types, const FVector& Location, float Radius,
	TArray<FCrowdPOIData>& OutCandidates) const
{
	for (const FCrowdPOIData& POI : RegisteredPOIs)
	{
		if (!Types.Contains(POI.Type)) continue;
		if (POI.MaxOccupant >= 0 && POI.CurrOccupant >= POI.MaxOccupant) continue;
		OutCandidates.Add(POI);
	}
}
