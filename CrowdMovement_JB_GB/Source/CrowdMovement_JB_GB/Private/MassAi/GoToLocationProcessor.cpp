// Fill out your copyright notice in the Description page of Project Settings.


#include "MassAi/GoToLocationProcessor.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"
#include "CrowdMovement_JB_GB/CrowdFragments.h"
#include "CrowdMovement_JB_GB/CrowdTags.h"

UGoToLocationProcessor::UGoToLocationProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
	bAutoRegisterWithProcessingPhases = true;
}


void UGoToLocationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
 
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FSphForcesFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSharedRequirement<FSharedLocationFragment>(EMassFragmentAccess::ReadOnly);
	
	EntityQuery.AddTagRequirement<FTagSphDebug>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FTagPassiveWalking>(EMassFragmentPresence::All);
 
	EntityQuery.RegisterWithProcessor(*this);
}

void UGoToLocationProcessor::Execute(FMassEntityManager& Manager, FMassExecutionContext& Context)
{
	UWorld* World = Manager.GetWorld();
	EntityQuery.ForEachEntityChunk(Context,
		[World](FMassExecutionContext& Ctx)
		{
			const int32 NumEntities = Ctx.GetNumEntities();
 
			const TConstArrayView<FCrowdFragments>    Personalities = Ctx.GetFragmentView<FCrowdFragments>();
			const TConstArrayView<FTransformFragment> Transforms    = Ctx.GetFragmentView<FTransformFragment>();
			const TConstArrayView<FSphForcesFragment> SphForces    = Ctx.GetFragmentView<FSphForcesFragment>();
			const TArrayView<FMassMoveTargetFragment> MoveTargets   = Ctx.GetMutableFragmentView<FMassMoveTargetFragment>();
			
			const FSharedLocationFragment& sharedLocation = Ctx.GetSharedFragment<FSharedLocationFragment>();
 
			constexpr float ArrivalThreshold = 150.f;
 
			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FCrowdFragments&    Personality = Personalities[i];
				const FTransformFragment& Transform		= Transforms[i];
				FMassMoveTargetFragment&  MoveTarget	= MoveTargets[i];
				const FVector& targetLocation			= sharedLocation.location;
				
				float SpeedMultiplier = 1.0f;
				switch (Personality.Personality)
				{
					case EPersonalityType::Calm:    SpeedMultiplier = 0.85f; break;
					case EPersonalityType::Social:  SpeedMultiplier = 1.0f;  break;
					case EPersonalityType::Nervous: SpeedMultiplier = 1.2f;  break;
					case EPersonalityType::Leader:  SpeedMultiplier = 1.15f; break;
				}
 
				const float DesiredSpeed = Personality.SpeedBase * SpeedMultiplier;
				
				// --- Direction blending ---
				const FVector AgentLoc      = Transform.GetTransform().GetLocation();
				const FVector ToTarget      = (targetLocation - AgentLoc).GetSafeNormal2D();  // goal pull
				const FVector SphOffset     = SphForces[i].acceleration.GetSafeNormal2D();  // crowd push
				
				// Weight: goal direction dominates, SPH steers around obstacles/crowd
				// Tune SphWeight (0.3–0.8) to taste
				constexpr float SphWeight   = 0.8f;
				const FVector BlendedDir    = (ToTarget + SphOffset * SphWeight).GetSafeNormal2D();
				
				// Write a virtual target slightly ahead in the blended direction
				// instead of the raw target — this lets Mass navigation steer smoothly
				constexpr float LookAheadDist = 100.f;
				const FVector VirtualTarget = AgentLoc + BlendedDir * LookAheadDist;

				MoveTarget.CreateNewAction(EMassMovementAction::Move, *World);
				MoveTarget.DesiredSpeed.Set(DesiredSpeed);
				MoveTarget.IntentAtGoal   = EMassMovementAction::Stand;
				MoveTarget.SlackRadius    = 150.f;

				MoveTarget.Center         = VirtualTarget;         // blended, not raw target
				MoveTarget.Forward        = BlendedDir;
				MoveTarget.DistanceToGoal = (targetLocation - AgentLoc).Size2D(); // true remaining dist
				
				
				DrawDebugLine(World, AgentLoc, VirtualTarget, FColor::Orange, false, -1, 0, 5);
				
				// MoveTarget.CreateNewAction(EMassMovementAction::Move, *World);
				// MoveTarget.DesiredSpeed.Set(DesiredSpeed);
				// MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
				// MoveTarget.SlackRadius = 150.f;
				//
				// MoveTarget.Center = targetLocation;
				//
				// const FVector TargetVector = MoveTarget.Center - Transform.GetTransform().GetLocation();
				// MoveTarget.DistanceToGoal = TargetVector.Size2D();
				// MoveTarget.Forward = TargetVector.GetSafeNormal2D();
			}
		});
}