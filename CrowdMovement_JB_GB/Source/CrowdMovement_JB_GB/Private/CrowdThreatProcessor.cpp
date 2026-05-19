// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdThreatProcessor.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "CrowdMovement_JB_GB/CrowdFragments.h"
#include "CrowdMovement_JB_GB/CrowdTags.h"
#include "CrowdThreatSubsystem.h"
#include "MassNavigationFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityManager.h"

UCrowdThreatProcessor::UCrowdThreatProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
	ExecutionOrder.ExecuteAfter.Add(TEXT("CrowdInitProcessor"));
	ExecutionOrder.ExecuteBefore.Add(TEXT("CrowdWalkingProcessor"));
	
	bAutoRegisterWithProcessingPhases = true;
}

void UCrowdThreatProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
	
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdPhaseFragments>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdSocialGroupFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadOnly);
 
	EntityQuery.AddConstSharedRequirement<FSharedCrowdSettings>(EMassFragmentPresence::All);
	
	EntityQuery.AddTagRequirement<FTagActiveFleeing>(EMassFragmentPresence::None);
	
	EntityQuery.RegisterWithProcessor(*this);
	
}

void UCrowdThreatProcessor::Execute(FMassEntityManager& Manager, FMassExecutionContext& Context)
{
	UWorld* World = Manager.GetWorld();
	if (!World) return;
	
	UCrowdThreatSubsystem* ThreatSub = World->GetSubsystem<UCrowdThreatSubsystem>();
	if (!ThreatSub || !ThreatSub->HasActiveThreats()) return;
	
	const TArray<FActiveThreatData>& ActiveThreats = ThreatSub->GetActiveThreats();
	
	
	EntityQuery.ForEachEntityChunk(Context,
		[&ActiveThreats](FMassExecutionContext& Ctx)
		{
			const float DeltaTime   = Ctx.GetDeltaTimeSeconds();
			const int32 NumEntities = Ctx.GetNumEntities();
 
			const TConstArrayView<FTransformFragment>   Transforms    = Ctx.GetFragmentView<FTransformFragment>();
			const TConstArrayView<FCrowdFragments>      Personalities = Ctx.GetFragmentView<FCrowdFragments>();
			const TArrayView<FCrowdPhaseFragments>      Phases        = Ctx.GetMutableFragmentView<FCrowdPhaseFragments>();
			const TArrayView<FCrowdSocialGroupFragment> Socials       = Ctx.GetMutableFragmentView<FCrowdSocialGroupFragment>();
			const TArrayView<FMassMoveTargetFragment>   MoveTargets   = Ctx.GetMutableFragmentView<FMassMoveTargetFragment>();
			const FSharedCrowdSettings& Settings                      = Ctx.GetConstSharedFragment<FSharedCrowdSettings>();
 
			for (int32 i = 0; i < NumEntities; ++i)
			{
				FCrowdPhaseFragments&      Phase       = Phases[i];
				FCrowdSocialGroupFragment& Social      = Socials[i];
				FMassMoveTargetFragment&   MoveTarget  = MoveTargets[i];
				const FCrowdFragments&     Personality = Personalities[i];
				const FVector AgentLocation = Transforms[i].GetTransform().GetLocation();
 
				float ThreatDelta = 0.f;
				for (const FActiveThreatData& Threat : ActiveThreats)
				{
					const bool bInRange = Threat.bGlobal ||
						FVector::DistSquared2D(AgentLocation, Threat.Location) <= Threat.RadiusSq;
 
					if (!bInRange) continue;
 
					const float PersonalityMod =
						(Personality.Personality == EPersonalityType::Nervous) ? 1.5f :
						(Personality.Personality == EPersonalityType::Leader)  ? 0.7f : 1.0f;
 
					const float CourageMod = FMath::Lerp(1.4f, 0.6f, Personality.CourageScore);
 
					ThreatDelta += Settings.ThreatAccumulationRate
						* Threat.Intensity * PersonalityMod * CourageMod * DeltaTime;
				}
 
				if (ThreatDelta <= 0.f) continue;
 
				Phase.ThreatAwareness += ThreatDelta;
 
				if (Phase.ThreatAwareness >= Settings.FleeingThreshold)
				{
					Phase.ThreatAwareness = Settings.FleeingThreshold;
					Phase.PreviousPhase   = Phase.CurrentPhase;
					Phase.CurrentPhase    = ECrowdPhase::Active_Fleeing;
					Phase.PhaseTimer      = 0.f;
 
					Social.GroupID           = -1;
					Social.bIsLeader         = false;
					Social.GroupLeader       = FMassEntityHandle();
					Social.SharedDestination = FVector::ZeroVector;
 
					MoveTarget.DesiredSpeed = FMassInt16Real(0.f);
					MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
 
					Ctx.Defer().RemoveTag<FTagPassiveWalking>(Ctx.GetEntity(i));
					Ctx.Defer().RemoveTag<FTagPassiveObserving>(Ctx.GetEntity(i));
					Ctx.Defer().RemoveTag<FTagPassiveDiscution>(Ctx.GetEntity(i));
					Ctx.Defer().AddTag<FTagActiveFleeing>(Ctx.GetEntity(i));
				}
			}
		});
}
