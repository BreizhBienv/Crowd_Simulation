// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdInitObserver.h"
#include "CrowdFragments.h"
#include "crowdTags.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"

UCrowdInitObserver::UCrowdInitObserver()
{
	ObservedType = FCrowdPhaseFragments::StaticStruct();
	Operation = EMassObservedOperation::Add;

	bAutoRegisterWithProcessingPhases = true;
}

void UCrowdInitObserver::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);

	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdPOIFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdBoidsFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.RegisterWithProcessor(*this);
}

void UCrowdInitObserver::Execute(FMassEntityManager & Manager, FMassExecutionContext & Contexte)
{
	EntityQuery.ForEachEntityChunk(Contexte,
		[](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();

			const TConstArrayView<FCrowdFragments> Personalities = Context.GetFragmentView<FCrowdFragments>();

			const TArrayView<FCrowdPOIFragment> CrowdPOIFragmentsList = Context.GetMutableFragmentView<FCrowdPOIFragment>();

			const TArrayView<FCrowdBoidsFragment> CrowdBoidsFragmentsList = Context.GetMutableFragmentView<FCrowdBoidsFragment>();

			for (int32 i = 0; i < NumEntities; i++)
			{
				const FCrowdFragments& Personality = Personalities[i];
				FCrowdPOIFragment& POI = CrowdPOIFragmentsList[i];
				FCrowdBoidsFragment& Boids = CrowdBoidsFragmentsList[i];

				POI.ObservationDuration = FMath::Lerp(3.f, 7.f, Personality.CuriosityScore);

				if(Personalities[i].Personality == EPersonalityType::Leader)
				{
					Boids.CompatibilityRadius = 500.f;
				}
				else 
				{
					Boids.CompatibilityRadius = FMath::Lerp(150.f, 400.f, Personality.SocialAffinityScore);
				}
				Boids.SocialUrge = 0.f;
				Boids.GroupID = -1;
				Boids.bInGroup = false;
			}
		});
}
