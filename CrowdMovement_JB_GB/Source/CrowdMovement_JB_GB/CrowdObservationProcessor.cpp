// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdObservationProcessor.h"
#include "CrowdFragments.h"
#include "crowdTags.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "MassCommonFragments.h"

UCrowdObservationProcessor::UCrowdObservationProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
	bAutoRegisterWithProcessingPhases = true;
}

void UCrowdObservationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdPOIFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdBoidsFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddTagRequirement<FTagPassiveObserving>(EMassFragmentPresence::All);

	EntityQuery.RegisterWithProcessor(*this);
}

void UCrowdObservationProcessor::Execute(FMassEntityManager & Manager, FMassExecutionContext & Contexte)
{
	EntityQuery.ForEachEntityChunk(Contexte,
		[](FMassExecutionContext& Context)
		{
			const float DeltaTime = Context.GetDeltaTimeSeconds();
			const int32 NumEntities = Context.GetNumEntities();

			const TArrayView<FCrowdPOIFragment> POIS = Context.GetMutableFragmentView<FCrowdPOIFragment>();

			const TArrayView<FCrowdPhaseFragments> Phases = Context.GetMutableFragmentView<FCrowdPhaseFragments>();

            for (int32 i = 0; i < NumEntities; ++i)
            {
                FCrowdPOIFragment& POI = POIS[i];
                FCrowdPhaseFragments& Phase = Phases[i];

                POI.ObservationTimer += DeltaTime;
                Phase.PhaseTimer += DeltaTime;

                if (POI.ObservationTimer >= POI.ObservationDuration)
                {
                    POI.bIsObserving = false;
                    POI.ObservationTimer = 0.f;
                    POI.ObservationDuration = 0.f;
                    POI.TargetPOILocation = FVector::ZeroVector;
                    POI.TargetPOIActor = nullptr;

                    Phase.PreviousPhase = Phase.CurrentPhase;
                    Phase.CurrentPhase = ECrowdPhase::Passive_Walk;
                    Phase.PhaseTimer = 0.f;

                    Context.Defer().RemoveTag<FTagPassiveObserving>(Context.GetEntity(i));
                    Context.Defer().AddTag<FTagPassiveWalking>(Context.GetEntity(i));
                }
            }


		});
}
