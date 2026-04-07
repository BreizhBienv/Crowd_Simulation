// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdWalkingProcessor.h"

#include "CrowdFragments.h"
#include "CrowdTags.h"

#include "MassExecutionContext.h"
#include "MassActorSubsystem.h"          
#include "MassCommonFragments.h"          
#include "MassMovementFragments.h"      


UCrowdWalkingProcessor::UCrowdWalkingProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
	bAutoRegisterWithProcessingPhases = true;
}

void UCrowdWalkingProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdPhaseFragments>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	//EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdPOIFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddConstSharedRequirement<FSharedCrowdSettings>(EMassFragmentPresence::All);

	EntityQuery.AddTagRequirement<FTagPassiveWalking>(EMassFragmentPresence::All);

	EntityQuery.RegisterWithProcessor(*this);
}

void UCrowdWalkingProcessor::Execute(FMassEntityManager& Manager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Manager, Context,
		[](FMassExecutionContext& Ctx)
		{
			const float DeltaTime = Ctx.GetDeltaTimeSeconds();
			const int32 NumEntities = Ctx.GetNumEntities();

			const TConstArrayView<FCrowdFragments>      Personalities = Ctx.GetFragmentView<FCrowdFragments>();
			const TConstArrayView<FTransformFragment>   Transforms = Ctx.GetFragmentView<FTransformFragment>();
			const TConstArrayView<FAgentRadiusFragment> Radii = Ctx.GetFragmentView<FAgentRadiusFragment>();
			const TArrayView<FCrowdPhaseFragments>      Phases = Ctx.GetMutableFragmentView<FCrowdPhaseFragments>();
			//const TArrayView<FMassMoveTargetFragment>   MoveTargets = Ctx.GetMutableFragmentView<FMassMoveTargetFragment>();
			const TArrayView<FCrowdPOIFragment>         POIs = Ctx.GetMutableFragmentView<FCrowdPOIFragment>();

			const FSharedCrowdSettings& Settings = Ctx.GetConstSharedFragment<FSharedCrowdSettings>();

			constexpr float WanderRadius = 800.f;

			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FCrowdFragments& Personality = Personalities[i];
				const FTransformFragment& Transform = Transforms[i];
				const FAgentRadiusFragment& Radius = Radii[i];
				FCrowdPhaseFragments& Phase = Phases[i];
				//FMassMoveTargetFragment& MoveTarget = MoveTargets[i];
				FCrowdPOIFragment& POI = POIs[i];

				const FVector CurrentLocation = Transform.GetTransform().GetLocation();

				if (POI.TargetPOILocation.IsNearlyZero())
				{
					const float Angle = FMath::FRandRange(0.f, 2.f * PI);
					const float Dist = FMath::FRandRange(WanderRadius * 0.3f, WanderRadius);

					POI.TargetPOILocation = CurrentLocation + FVector(
						FMath::Cos(Angle) * Dist,
						FMath::Sin(Angle) * Dist,
						0.f
					);
				}

				float SpeedMultiplier = 1.0f;
				switch (Personality.Personality)
				{
				case EPersonalityType::Calm:    SpeedMultiplier = 0.85f; break;
				case EPersonalityType::Social:  SpeedMultiplier = 1.0f;  break;
				case EPersonalityType::Nervous: SpeedMultiplier = 1.2f;  break;
				case EPersonalityType::Leader:  SpeedMultiplier = 1.15f; break;
				}

				const float DesiredSpeed = Personality.SpeedBase * SpeedMultiplier;

				//MoveTarget.Center = POI.TargetPOILocation;
				//MoveTarget.Forward = (POI.TargetPOILocation - CurrentLocation).GetSafeNormal2D();
				//MoveTarget.DesiredSpeed = FMassInt16Real(DesiredSpeed);
				//MoveTarget.IntentAtGoal = EMassMovementAction::Move;

				Phase.PhaseTimer += DeltaTime;

				const float ArrivalThreshold = Radius.Radius * 2.f + 50.f;
				const float DistToTarget = FVector::Dist2D(CurrentLocation, POI.TargetPOILocation);

				if (DistToTarget < ArrivalThreshold)
				{
	
					POI.bIsObserving = true;
					POI.ObservationTimer = 0.f;


					// Arrêter le mouvement
					//MoveTarget.Center = CurrentLocation;
					//MoveTarget.DesiredSpeed = FMassInt16Real(0.f);
					//MoveTarget.IntentAtGoal = EMassMovementAction::Stand;

					// Transition de phase
					Phase.PreviousPhase = Phase.CurrentPhase;
					Phase.CurrentPhase = ECrowdPhase::Passive_Observation;
					Phase.PhaseTimer = 0.f;

					Ctx.Defer().RemoveTag<FTagPassiveWalking>(Ctx.GetEntity(i));
					Ctx.Defer().AddTag<FTagPassiveObserving>(Ctx.GetEntity(i));
				}
			}
		});
}