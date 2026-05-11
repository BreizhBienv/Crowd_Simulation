// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdTalkingProcessor.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"       
#include "MassEntityManager.h"
#include "CrowdFragments.h"
#include "CrowdTags.h"
#include "MassCommonFragments.h"          
#include "MassMovementFragments.h"      


UCrowdTalkingProcessor::UCrowdTalkingProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
	ExecutionOrder.ExecuteAfter.Add(TEXT("CrowdWalkingProcessor"));
	bAutoRegisterWithProcessingPhases = true;
}

void UCrowdTalkingProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);

	
	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdPhaseFragments>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdSocialGroupFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdPOIFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
 
	EntityQuery.AddConstSharedRequirement<FSharedCrowdSettings>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FTagPassiveDiscution>(EMassFragmentPresence::All);
 
	EntityQuery.RegisterWithProcessor(*this);
}

void UCrowdTalkingProcessor::Execute(FMassEntityManager & Manager, FMassExecutionContext & Context)
{
	EntityQuery.ForEachEntityChunk(Context,
		[](FMassExecutionContext& Ctx) 
		{
			
			const float DeltaTime   = Ctx.GetDeltaTimeSeconds();
			const int32 NumEntities = Ctx.GetNumEntities();
 
			const TConstArrayView<FCrowdFragments>      Personalities = Ctx.GetFragmentView<FCrowdFragments>();
			const TArrayView<FCrowdPhaseFragments>      Phases        = Ctx.GetMutableFragmentView<FCrowdPhaseFragments>();
			const TArrayView<FCrowdSocialGroupFragment> Socials       = Ctx.GetMutableFragmentView<FCrowdSocialGroupFragment>();
			const TArrayView<FCrowdPOIFragment>         POIs          = Ctx.GetMutableFragmentView<FCrowdPOIFragment>();
			const TArrayView<FMassMoveTargetFragment>   MoveTargets   = Ctx.GetMutableFragmentView<FMassMoveTargetFragment>();

			for (int i = 0; i < NumEntities; ++i)
			{
				FCrowdPhaseFragments&       Phase       = Phases[i];
				FCrowdSocialGroupFragment&  Social      = Socials[i];
				FCrowdPOIFragment&          POI         = POIs[i];
				FMassMoveTargetFragment&    MoveTarget  = MoveTargets[i];
 
				Phase.PhaseTimer   += DeltaTime;
				Social.TalkTimer   += DeltaTime;
				
				if (Social.TalkTimer >= Social.TalkDuration)
				{
					Social.TalkTimer    = 0.f;
					Social.TalkDuration = 0.f;
 
					// Réinitialiser la destination POI pour forcer un nouveau choix (try to fix)
					POI.TargetPOILocation = FVector::ZeroVector;
					POI.TargetPOIActor    = nullptr;
 
					
					if (Social.bIsLeader)
					{
						Social.SharedDestination = FVector::ZeroVector;
					}
 
					Phase.PreviousPhase = Phase.CurrentPhase;
					Phase.CurrentPhase  = ECrowdPhase::Passive_Walk;
					Phase.PhaseTimer    = 0.f;
					
					MoveTarget.IntentAtGoal = EMassMovementAction::Move;
 
					Ctx.Defer().RemoveTag<FTagPassiveDiscution>(Ctx.GetEntity(i));
					Ctx.Defer().AddTag<FTagPassiveWalking>(Ctx.GetEntity(i));
				}
			}
		});
}
