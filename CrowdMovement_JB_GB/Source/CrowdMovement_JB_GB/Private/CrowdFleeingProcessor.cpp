// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdFleeingProcessor.h"
#include "CrowdMovement_JB_GB/CrowdFragments.h"
#include "CrowdMovement_JB_GB/CrowdTags.h"
#include "CrowdMovement_JB_GB/CrowdPOISubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassNavigationFragments.h"
#include "MassEntityManager.h"

UCrowdFleeingProcessor::UCrowdFleeingProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
	ExecutionOrder.ExecuteAfter.Add(TEXT("CrowdThreatProcessor"));
 
	bAutoRegisterWithProcessingPhases = true;
}

void UCrowdFleeingProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
	
	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdPhaseFragments>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdPOIFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
 
	EntityQuery.AddConstSharedRequirement<FSharedCrowdSettings>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FTagActiveFleeing>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UCrowdFleeingProcessor::Execute(FMassEntityManager& Manager, FMassExecutionContext& Context)
{
	UCrowdPOISubsystem* POISubsystem = Manager.GetWorld()->GetSubsystem<UCrowdPOISubsystem>();
 
	EntityQuery.ForEachEntityChunk(Context,
		[POISubsystem](FMassExecutionContext& Ctx)
		{
			const float DeltaTime   = Ctx.GetDeltaTimeSeconds();
			const int32 NumEntities = Ctx.GetNumEntities();
 
			const TConstArrayView<FCrowdFragments>    Personalities = Ctx.GetFragmentView<FCrowdFragments>();
			const TConstArrayView<FTransformFragment> Transforms    = Ctx.GetFragmentView<FTransformFragment>();
			const TArrayView<FCrowdPhaseFragments>    Phases        = Ctx.GetMutableFragmentView<FCrowdPhaseFragments>();
			const TArrayView<FCrowdPOIFragment>       POIs          = Ctx.GetMutableFragmentView<FCrowdPOIFragment>();
			const TArrayView<FMassMoveTargetFragment> MoveTargets   = Ctx.GetMutableFragmentView<FMassMoveTargetFragment>();
			const FSharedCrowdSettings& Settings                    = Ctx.GetConstSharedFragment<FSharedCrowdSettings>();
 
			constexpr float ArrivalThreshold = 120.f;
			
			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FCrowdFragments&   Personality = Personalities[i];
				FCrowdPhaseFragments&    Phase       = Phases[i];
				FCrowdPOIFragment&       POI         = POIs[i];
				FMassMoveTargetFragment& MoveTarget  = MoveTargets[i];
 
				const FVector CurrentLocation = Transforms[i].GetTransform().GetLocation();
 
				Phase.PhaseTimer += DeltaTime;
				
				if (POI.TargetPOILocation.IsNearlyZero())
				{
					FCrowdPOIData EscapePod;
					if (POISubsystem && POISubsystem->GetNearestEscapePoint(CurrentLocation, EscapePod))
					{
						POI.TargetPOILocation = EscapePod.Location;
						POI.TargetPOIActor    = EscapePod.SourceActor.Get();
					}
					else
						continue;
				}
				
				const float FleeSpeedMultiplier = 1.5f + (1.f - Personality.CourageScore) * 0.8f;
				float DesiredSpeed = Personality.SpeedBase * FleeSpeedMultiplier;
				
				if (Personality.Personality == EPersonalityType::Leader)
					DesiredSpeed *= 0.85f;
				
				const FVector Direction = (POI.TargetPOILocation - CurrentLocation).GetSafeNormal2D();
 
				MoveTarget.Center       = POI.TargetPOILocation;
				MoveTarget.Forward      = Direction;
				MoveTarget.DesiredSpeed = FMassInt16Real(DesiredSpeed);
				MoveTarget.IntentAtGoal = EMassMovementAction::Move;
				
				const float DistToPod = FVector::Dist2D(CurrentLocation, POI.TargetPOILocation);
				if (DistToPod < ArrivalThreshold)
				{
					MoveTarget.Center       = CurrentLocation;
					MoveTarget.DesiredSpeed = FMassInt16Real(0.f);
					MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
 
					POI.TargetPOILocation = FVector::ZeroVector;
					POI.TargetPOIActor    = nullptr;
 
					Phase.PreviousPhase = Phase.CurrentPhase;
					Phase.CurrentPhase  = ECrowdPhase::Passive_Observation;
					Phase.PhaseTimer    = 0.f;
					Phase.ThreatAwareness = 0.f;
					
					Ctx.Defer().RemoveTag<FTagActiveFleeing>(Ctx.GetEntity(i));
					Ctx.Defer().AddTag<FTagPassiveObserving>(Ctx.GetEntity(i));
				}
			}
			
		});
}
