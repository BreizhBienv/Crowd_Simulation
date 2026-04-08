// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdDebugProcessor.h"

#include "CrowdFragments.h"
#include "CrowdTags.h"
#include "MassCommonFragments.h"
#include "MassNavigationFragments.h"
#include "MassExecutionContext.h"
#include "MassEntityTypes.h"   
#include "DrawDebugHelpers.h"

// to activate debug mode tape  
// mass.crowd.debug 1  activer
// mass.crowd.debug 0  désactiver


static TAutoConsoleVariable<int32> CVarCrowdDebug(
	TEXT("mass.crowd.debug"),
	0,
	TEXT("Active le debug visuel des agents de foule.\n")
	TEXT("0 = désactivé, 1 = activé"),
	ECVF_Cheat
);

UCrowdDebugProcessor::UCrowdDebugProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Behavior);

	bAutoRegisterWithProcessingPhases = true;
#if !UE_BUILD_SHIPPING
	
#else
	bAutoRegisterWithProcessingPhases = false;
#endif
}

void UCrowdDebugProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>&EntityManager)
{
#if !UE_BUILD_SHIPPING
	EntityQuery.Initialize(EntityManager);

	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdPOIFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdPhaseFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadOnly);

	EntityQuery.RegisterWithProcessor(*this);
#endif
}

void UCrowdDebugProcessor::Execute(FMassEntityManager & Manager, FMassExecutionContext & Context)
{
#if !UE_BUILD_SHIPPING
	if (CVarCrowdDebug.GetValueOnGameThread() == 0)
	{
		return;
	}

	UWorld* World = Manager.GetWorld();
	if (!World)
	{
		return;
	}
	EntityQuery.ForEachEntityChunk(Context,
		[](FMassExecutionContext& Ctx)
		{
			UWorld* World = Ctx.GetWorld();
			const int32 NumEntities = Ctx.GetNumEntities();
			const TConstArrayView<FTransformFragment>    Transforms = Ctx.GetFragmentView<FTransformFragment>();
			const TConstArrayView<FCrowdPOIFragment>     POIs = Ctx.GetFragmentView<FCrowdPOIFragment>();
			const TConstArrayView<FCrowdPhaseFragments>  Phases = Ctx.GetFragmentView<FCrowdPhaseFragments>();
			const TConstArrayView<FCrowdFragments>       Personalities = Ctx.GetFragmentView<FCrowdFragments>();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FVector AgentLocation = Transforms[i].GetTransform().GetLocation()
					+ FVector(0.f, 0.f, 90.f); 

				const FCrowdPOIFragment& POI = POIs[i];
				const FCrowdPhaseFragments& Phase = Phases[i];

				FColor LineColor = FColor::White;
				FString PhaseName = TEXT("?");

				switch (Phase.CurrentPhase)
				{
				case ECrowdPhase::Passive_Walk:
					LineColor = FColor::Green;
					PhaseName = TEXT("Walking");
					break;
				case ECrowdPhase::Passive_Observation:
					LineColor = FColor::Cyan;
					PhaseName = TEXT("Observing");
					break;
				case ECrowdPhase::Passive_Discution:
					LineColor = FColor::Yellow;
					PhaseName = TEXT("Talking");
					break;
				case ECrowdPhase::Active_Fleeing:
					LineColor = FColor::Orange;
					PhaseName = TEXT("Fleeing");
					break;
				case ECrowdPhase::Active_Fighting:
					LineColor = FColor::Red;
					PhaseName = TEXT("Fighting");
					break;
				}

				if (!POI.TargetPOILocation.IsNearlyZero())
				{
					DrawDebugLine(
						World,
						AgentLocation,
						POI.TargetPOILocation + FVector(0.f, 0.f, 10.f),
						LineColor,
						false, 
						-1.f,  
						0,
						2.f   
					);
					DrawDebugSphere(
						World,
						POI.TargetPOILocation + FVector(0.f, 0.f, 10.f),
						20.f,
						6,
						LineColor,
						false,
						-1.f
					);
				}
				const FString DebugText = FString::Printf(
					TEXT("%s\n%.1fs"),
					*PhaseName,
					Phase.PhaseTimer
				);

				DrawDebugString(
					World,
					AgentLocation + FVector(0.f, 0.f, 30.f),
					DebugText,
					nullptr,
					LineColor,
					0.f,   
					true   
				);
			}
		});
#endif
}
