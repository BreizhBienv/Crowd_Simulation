// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdSocialProcessor.h"
#include "CrowdMovement_JB_GB/CrowdFragments.h"
#include "CrowdMovement_JB_GB/CrowdTags.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassNavigationFragments.h"
#include "MassEntityManager.h"


UCrowdSocialProcessor::UCrowdSocialProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
	ExecutionOrder.ExecuteAfter.Add(TEXT("CrowdWalkingProcessor"));
	bAutoRegisterWithProcessingPhases = true;
}

void UCrowdSocialProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
	
	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdSocialGroupFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdPhaseFragments>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdPOIFragment>(EMassFragmentAccess::ReadWrite);
	
	EntityQuery.AddConstSharedRequirement<FSharedCrowdSettings>(EMassFragmentPresence::All);
	
	EntityQuery.AddTagRequirement<FTagPassiveWalking>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UCrowdSocialProcessor::Execute(FMassEntityManager& Manager, FMassExecutionContext& Context)
{
	TMap<int32, FVector> LeaderPositions;
	LeaderPositions.Reserve(256);
	
	EntityQuery.ForEachEntityChunk(Context,
	[&LeaderPositions](FMassExecutionContext& Ctx)
	{
		const int32 N = Ctx.GetNumEntities();
		const TConstArrayView<FTransformFragment>        Transforms = Ctx.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FCrowdSocialGroupFragment> Socials    = Ctx.GetFragmentView<FCrowdSocialGroupFragment>();
 
		for (int32 i = 0; i < N; ++i)
		{
			const FCrowdSocialGroupFragment& S = Socials[i];
			if (S.GroupID >= 0 && S.bIsLeader)
				LeaderPositions.Add(S.GroupID, Transforms[i].GetTransform().GetLocation());
		}
	});
	
	EntityQuery.ForEachEntityChunk(Context,
		[&LeaderPositions](FMassExecutionContext& Ctx)
		{
			const float DeltaTime   = Ctx.GetDeltaTimeSeconds();
			const int32 NumEntities = Ctx.GetNumEntities();
 
			const TConstArrayView<FCrowdFragments>      Personalities = Ctx.GetFragmentView<FCrowdFragments>();
			const TConstArrayView<FTransformFragment>   Transforms    = Ctx.GetFragmentView<FTransformFragment>();
			const TArrayView<FCrowdSocialGroupFragment> Socials       = Ctx.GetMutableFragmentView<FCrowdSocialGroupFragment>();
			const TArrayView<FCrowdPhaseFragments>      Phases        = Ctx.GetMutableFragmentView<FCrowdPhaseFragments>();
			const TArrayView<FMassMoveTargetFragment>   MoveTargets   = Ctx.GetMutableFragmentView<FMassMoveTargetFragment>();
			const TArrayView<FCrowdPOIFragment>         POIs          = Ctx.GetMutableFragmentView<FCrowdPOIFragment>();
			const FSharedCrowdSettings& Settings                      = Ctx.GetConstSharedFragment<FSharedCrowdSettings>();
			
			for (int32 i = 0; i < NumEntities; ++i)
			{
				FCrowdSocialGroupFragment& Social    = Socials[i];
				FCrowdPhaseFragments&      Phase     = Phases[i];
				FMassMoveTargetFragment&   MoveTarget = MoveTargets[i];
				
				if (Social.GroupID < 0) continue;
				
				const FVector CurrLocation = Transforms[i].GetTransform().GetLocation();
				
				if(!Social.bIsLeader)
				{
					const FVector* LeaderPos = LeaderPositions.Find(Social.GroupID);
					
					if (!LeaderPos) continue;
					
					const float DistToLeader = FVector::Dist2D(CurrLocation, *LeaderPos);
					
					if (DistToLeader > Settings.WalkTogetherRadius)
					{
						const float CurrSpeed = MoveTarget.DesiredSpeed.Get();
						MoveTarget.DesiredSpeed = FMassInt16Real(CurrSpeed * Settings.CatchUpSpeedMultiplier);
					}
					
					if (DistToLeader < Settings.TalkRadius)
					{
						Social.TalkDuration = FMath::FRandRange(Settings.TalkDurationMin, Settings.TalkDurationMax);
						Social.TalkTimer    = 0.f;
 
						MoveTarget.Center       = CurrLocation;
						MoveTarget.DesiredSpeed = FMassInt16Real(0.f);
						MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
 
						Phase.PreviousPhase = Phase.CurrentPhase;
						Phase.CurrentPhase  = ECrowdPhase::Passive_Discution;
						Phase.PhaseTimer    = 0.f;
 
						Ctx.Defer().RemoveTag<FTagPassiveWalking>(Ctx.GetEntity(i));
						Ctx.Defer().AddTag<FTagPassiveDiscution>(Ctx.GetEntity(i));
					}
				}
				else
				{
					for (int32 j = 0; j < NumEntities; ++j)
					{
						if (j == i) continue;
						const FCrowdSocialGroupFragment& Other = Socials[j];
						if (Other.GroupID != Social.GroupID || Other.bIsLeader) continue;
 
						const float Dist = FVector::Dist2D(
							CurrLocation,
							Transforms[j].GetTransform().GetLocation());
 
						if (Dist < Settings.TalkRadius)
						{
							Social.TalkDuration = FMath::FRandRange(Settings.TalkDurationMin, Settings.TalkDurationMax);
							Social.TalkTimer    = 0.f;
 
							MoveTarget.Center       = CurrLocation;
							MoveTarget.DesiredSpeed = FMassInt16Real(0.f);
							MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
 
							Phase.PreviousPhase = Phase.CurrentPhase;
							Phase.CurrentPhase  = ECrowdPhase::Passive_Discution;
							Phase.PhaseTimer    = 0.f;
 
							Ctx.Defer().RemoveTag<FTagPassiveWalking>(Ctx.GetEntity(i));
							Ctx.Defer().AddTag<FTagPassiveDiscution>(Ctx.GetEntity(i));
							break;
						}
					}
				}
			}
		});
}
