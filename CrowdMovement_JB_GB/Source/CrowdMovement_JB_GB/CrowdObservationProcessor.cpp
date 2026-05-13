// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdObservationProcessor.h"
#include "CrowdFragments.h"
#include "CrowdTags.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"

UCrowdObservationProcessor::UCrowdObservationProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
	ExecutionOrder.ExecuteAfter.Add(TEXT("CrowdWalkingProcessor"));
	bAutoRegisterWithProcessingPhases = true;
}

void UCrowdObservationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    EntityQuery.Initialize(EntityManager);

	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdPOIFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdPhaseFragments>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdSocialGroupFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddTagRequirement<FTagPassiveObserving>(EMassFragmentPresence::All);

	EntityQuery.RegisterWithProcessor(*this);
}

void UCrowdObservationProcessor::Execute(FMassEntityManager & Manager, FMassExecutionContext & Contexte)
{
	EntityQuery.ForEachEntityChunk(Contexte,
		[](FMassExecutionContext& Ctx)
		{
			const float DeltaTime = Ctx.GetDeltaTimeSeconds();
			const int32 NumEntities = Ctx.GetNumEntities();
			
			const TArrayView<FCrowdPOIFragment>         POIs          = Ctx.GetMutableFragmentView<FCrowdPOIFragment>();
			const TArrayView<FCrowdPhaseFragments>      Phases        = Ctx.GetMutableFragmentView<FCrowdPhaseFragments>();
			const TArrayView<FCrowdSocialGroupFragment> Socials       = Ctx.GetMutableFragmentView<FCrowdSocialGroupFragment>();
			
			
			for (int32 i = 0; i < NumEntities; ++i)
			{
				FCrowdPOIFragment&         POI    = POIs[i];
				FCrowdPhaseFragments&      Phase  = Phases[i];
				FCrowdSocialGroupFragment& Social = Socials[i];
 
				POI.ObservationTimer += DeltaTime;
				Phase.PhaseTimer     += DeltaTime;
			

				if (POI.ObservationTimer >= POI.ObservationDuration)
				{
			
					POI.bIsObserving        = false;
					POI.ObservationTimer    = 0.f;
					
					POI.TargetPOILocation   = FVector::ZeroVector;
					POI.TargetPOIActor      = nullptr;
 
					
					if (Social.bIsLeader)
					{
						Social.SharedDestination = FVector::ZeroVector;
					}
 
					Phase.PreviousPhase = Phase.CurrentPhase;
					Phase.CurrentPhase  = ECrowdPhase::Passive_Walk;
					Phase.PhaseTimer    = 0.f;
 
					Ctx.Defer().RemoveTag<FTagPassiveObserving>(Ctx.GetEntity(i));
					Ctx.Defer().AddTag<FTagPassiveWalking>(Ctx.GetEntity(i));
				}
            }
		});
}
