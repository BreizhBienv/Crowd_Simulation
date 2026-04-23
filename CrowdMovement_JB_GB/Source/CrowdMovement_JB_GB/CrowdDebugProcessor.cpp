// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdDebugProcessor.h"

#include "CrowdFragments.h"
#include "CrowdTags.h"
#include "MassCommonFragments.h"
#include "MassNavigationFragments.h"
#include "MassExecutionContext.h"
#include "MassEntityTypes.h"   
#include "DrawDebugHelpers.h"
#include <String>

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

	ExecutionOrder.ExecuteAfter.Add(TEXT("CrowdWalkingProcessor"));
	ExecutionOrder.ExecuteAfter.Add(TEXT("CrowdObservationProcessor"));

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
    EntityQuery.AddRequirement<FCrowdBoidsFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.RegisterWithProcessor(*this);
#endif
}

void UCrowdDebugProcessor::Execute(FMassEntityManager & Manager, FMassExecutionContext & Context)
{
#if !UE_BUILD_SHIPPING
	if (CVarCrowdDebug.GetValueOnAnyThread() == 0)
	{
		return;
	}

	UWorld* World = Manager.GetWorld();
	if (!World)
	{
		return;
	}

	struct FDebugDrawData
	{
		FVector AgentLocation;
		FVector TargetLocation;
		FColor  Color;
		FString Label;
	};

	TArray<FDebugDrawData> DrawList;


    EntityQuery.ForEachEntityChunk(Context,
        [&DrawList](FMassExecutionContext& Ctx)
        {
            const int32 NumEntities = Ctx.GetNumEntities();
            const TConstArrayView<FTransformFragment>   Transforms = Ctx.GetFragmentView<FTransformFragment>();
            const TConstArrayView<FCrowdPOIFragment>    POIs = Ctx.GetFragmentView<FCrowdPOIFragment>();
			const TConstArrayView<FCrowdFragments>       Personalities = Ctx.GetFragmentView<FCrowdFragments>();
            const TConstArrayView<FCrowdPhaseFragments> Phases = Ctx.GetFragmentView<FCrowdPhaseFragments>();
            const auto Boids = Ctx.GetFragmentView<FCrowdBoidsFragment>();

            for (int32 i = 0; i < NumEntities; ++i)
            {
                const FVector AgentLocation = Transforms[i].GetTransform().GetLocation() + FVector(0.f, 0.f, 90.f);
                const FCrowdPOIFragment& POI = POIs[i];
                const FCrowdPhaseFragments& Phase = Phases[i];
				const FCrowdBoidsFragment& Boid = Boids[i];

                FColor  LineColor = FColor::White;
                FString PhaseName = TEXT("?");
				FString PersonalityName = TEXT("?");
				FString BoidGroupe = TEXT("?");

                switch (Phase.CurrentPhase)
                {
                case ECrowdPhase::Passive_Walk:
                    LineColor = FColor::Green;  PhaseName = TEXT("Walking");   break;
                case ECrowdPhase::Passive_Observation:
                    LineColor = FColor::Cyan;   PhaseName = TEXT("Observing"); break;
                case ECrowdPhase::Passive_Discution:
                    LineColor = FColor::Yellow; PhaseName = TEXT("Talking");   break;
                case ECrowdPhase::Active_Fleeing:
                    LineColor = FColor::Orange; PhaseName = TEXT("Fleeing");   break;
                case ECrowdPhase::Active_Fighting:
                    LineColor = FColor::Red;    PhaseName = TEXT("Fighting");  break;
                }

                switch (Personalities[i].Personality)
                {
                    case EPersonalityType::Calm:
                         PersonalityName = TEXT("Calm"); break;
					case EPersonalityType::Social:
                        PersonalityName = TEXT("Social"); break;
					case EPersonalityType::Leader:
                        PersonalityName = TEXT("Leader"); break;
					case EPersonalityType::Nervous:
                        PersonalityName = TEXT("Nervous"); break;
                default:
                    break;
                }

                FDebugDrawData Data;
                Data.AgentLocation = AgentLocation;
                Data.TargetLocation = POI.TargetPOILocation;
                Data.Color = LineColor;
                Data.Label = FString::Printf(TEXT("%s\n%.1fs\n%s"), *PhaseName, Phase.PhaseTimer, *PersonalityName);

                DrawList.Add(Data);
            }
        });

    // Rendu sur le Game Thread
    AsyncTask(ENamedThreads::GameThread, [World, DrawList = MoveTemp(DrawList)]()
        {
            for (const FDebugDrawData& Data : DrawList)
            {
                if (!Data.TargetLocation.IsNearlyZero())
                {
                    DrawDebugLine(
                        World,
                        Data.AgentLocation,
                        Data.TargetLocation + FVector(0.f, 0.f, 10.f),
                        Data.Color,
                        false, -1.f, 0, 2.f
                    );
                    DrawDebugSphere(
                        World,
                        Data.TargetLocation + FVector(0.f, 0.f, 10.f),
                        20.f, 6, Data.Color,
                        false, -1.f
                    );
                }

                DrawDebugString(
                    World,
                    Data.AgentLocation + FVector(0.f, 0.f, 30.f),
                    Data.Label,
                    nullptr,
                    Data.Color,
                    0.f, true
                );
            }
        });
#endif
}
