// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdTalkingProcessor.h"
#include "MassExecutionContext.h"
#include "MassActorSubsystem.h"       
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

	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdPOIFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdPhaseFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FSharedCrowdSettings>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FTagPassiveDiscution>(EMassFragmentPresence::All);
	EntityQuery.RegisterWithProcessor(*this);
}

void UCrowdTalkingProcessor::Execute(FMassEntityManager & Manager, FMassExecutionContext & Context)
{
	EntityQuery.ForEachEntityChunk(Context,
		[](FMassExecutionContext& Ctx) 
		{

		});
}
