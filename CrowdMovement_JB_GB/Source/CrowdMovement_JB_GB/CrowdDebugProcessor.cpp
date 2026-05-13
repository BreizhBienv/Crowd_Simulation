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
// 0 = désactivé
// 1 = phase + personnalité + rôle
// 2 = mode 1 + GroupID + ThreatAwareness + ligne vers cible


static TAutoConsoleVariable<int32> CVarCrowdDebug(
TEXT("mass.crowd.debug"),
0,
TEXT("Debug visuel des agents.\n")
TEXT("0=off  1=labels  2=labels+lines+threat"),
ECVF_Cheat
);

static TAutoConsoleVariable<float> CVarCrowdDebugRadius(
	TEXT("mass.crowd.debug.radius"), 2000.f,
	TEXT("Rayon de debug autour de la caméra (cm)"),
	ECVF_Cheat
);

static TAutoConsoleVariable<int32> CVarCrowdDebugMaxAgents(
	TEXT("mass.crowd.debug.maxagents"), 200,
	TEXT("Nombre max d agents debug affiches"),
	ECVF_Cheat
);
 

UCrowdDebugProcessor::UCrowdDebugProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
	ExecutionOrder.ExecuteAfter.Add(TEXT("CrowdWalkingProcessor"));
	ExecutionOrder.ExecuteAfter.Add(TEXT("CrowdObservationProcessor"));
	ExecutionOrder.ExecuteAfter.Add(TEXT("CrowdTalkingProcessor"));
	ExecutionOrder.ExecuteAfter.Add(TEXT("CrowdFleeingProcessor"));
 
	bRequiresGameThreadExecution = true;
 
#if !UE_BUILD_SHIPPING
	bAutoRegisterWithProcessingPhases = true;
#else
	bAutoRegisterWithProcessingPhases = false;
#endif
}

void UCrowdDebugProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>&EntityManager)
{
#if !UE_BUILD_SHIPPING
	EntityQuery.Initialize(EntityManager);
 
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdPhaseFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdPOIFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdRoleFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdSocialGroupFragment>(EMassFragmentAccess::ReadOnly);
 
	EntityQuery.RegisterWithProcessor(*this);
#endif
}

void UCrowdDebugProcessor::Execute(FMassEntityManager & Manager, FMassExecutionContext & Context)
{
#if !UE_BUILD_SHIPPING
	const int32 DebugLevel = CVarCrowdDebug.GetValueOnGameThread();
	if (DebugLevel == 0) return;
 
	UWorld* World = Manager.GetWorld();
	if (!World) return;
	
	FVector CameraLocation = FVector::ZeroVector;
	bool bHasCamera = false;
	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		if (PC->PlayerCameraManager)
		{
			CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
			bHasCamera = true;
		}
	}
 
	const float DebugRadiusSq = FMath::Square(CVarCrowdDebugRadius.GetValueOnGameThread());
	const int32 MaxAgents     = CVarCrowdDebugMaxAgents.GetValueOnGameThread();
 
	struct FDebugDrawData
	{
		FVector Location;
		FVector TargetLocation;
		FColor  Color;
		FString Label;
		float   ThreatAwareness;
	};
 
	TArray<FDebugDrawData> DrawList;
	DrawList.Reserve(MaxAgents);
 
	EntityQuery.ForEachEntityChunk(Context,
		[&](FMassExecutionContext& Ctx)
		{
			if (DrawList.Num() >= MaxAgents) return;
 
			const int32 NumEntities = Ctx.GetNumEntities();
 
			const TConstArrayView<FTransformFragment>        Transforms    = Ctx.GetFragmentView<FTransformFragment>();
			const TConstArrayView<FCrowdFragments>           Personalities = Ctx.GetFragmentView<FCrowdFragments>();
			const TConstArrayView<FCrowdPhaseFragments>      Phases        = Ctx.GetFragmentView<FCrowdPhaseFragments>();
			const TConstArrayView<FCrowdPOIFragment>         POIs          = Ctx.GetFragmentView<FCrowdPOIFragment>();
			const TConstArrayView<FCrowdRoleFragment>        Roles         = Ctx.GetFragmentView<FCrowdRoleFragment>();
			const TConstArrayView<FCrowdSocialGroupFragment> Socials       = Ctx.GetFragmentView<FCrowdSocialGroupFragment>();
 
			for (int32 i = 0; i < NumEntities && DrawList.Num() < MaxAgents; ++i)
			{
				const FVector AgentLocation = Transforms[i].GetTransform().GetLocation();
				
				if (bHasCamera &&
					FVector::DistSquared2D(AgentLocation, CameraLocation) > DebugRadiusSq)
					continue;
 
				const FCrowdPhaseFragments&      Phase       = Phases[i];
				const FCrowdFragments&           Personality = Personalities[i];
				const FCrowdRoleFragment&        Role        = Roles[i];
				const FCrowdSocialGroupFragment& Social      = Socials[i];
 
				// Couleur par phase
				FColor LineColor;
				FString PhaseName;
				switch (Phase.CurrentPhase)
				{
					case ECrowdPhase::Passive_Walk:        LineColor = FColor::Green;  PhaseName = TEXT("Walk");    break;
					case ECrowdPhase::Passive_Observation: LineColor = FColor::Cyan;   PhaseName = TEXT("Obs");     break;
					case ECrowdPhase::Passive_Discution:   LineColor = FColor::Yellow; PhaseName = TEXT("Talk");    break;
					case ECrowdPhase::Active_Fleeing:      LineColor = FColor::Orange; PhaseName = TEXT("FLEE");    break;
					case ECrowdPhase::Active_Fighting:     LineColor = FColor::Red;    PhaseName = TEXT("FIGHT");   break;
					default:                               LineColor = FColor::White;  PhaseName = TEXT("?");       break;
				}
 
				// Rôle
				FString RoleName;
				switch (Role.Role)
				{
					case ECrewRole::Pilot:    RoleName = TEXT("Pilot");  break;
					case ECrewRole::Engineer: RoleName = TEXT("Engin."); break;
					case ECrewRole::Medic:    RoleName = TEXT("Medic");  break;
					case ECrewRole::Guard:    RoleName = TEXT("Guard");  break;
					case ECrewRole::Crew:     RoleName = TEXT("Crew");   break;
					default:                  RoleName = TEXT("?");      break;
				}
 
				// Groupe
				FString GroupInfo = (Social.GroupID >= 0)
					? FString::Printf(TEXT("G%d%s"), Social.GroupID, Social.bIsLeader ? TEXT("*") : TEXT(""))
					: TEXT("Solo");
 
				// Label selon niveau
				FString Label;
				if (DebugLevel >= 2)
				{
					Label = FString::Printf(TEXT("[%s] %s\n%s | Threat:%.2f"),
						*RoleName, *PhaseName, *GroupInfo, Phase.ThreatAwareness);
				}
				else
				{
					Label = FString::Printf(TEXT("[%s] %s\n%s"), *RoleName, *PhaseName, *GroupInfo);
				}
 
				FDebugDrawData Data;
				Data.Location        = AgentLocation + FVector(0.f, 0.f, 90.f);
				Data.TargetLocation  = POIs[i].TargetPOILocation;
				Data.Color           = LineColor;
				Data.Label           = Label;
				Data.ThreatAwareness = Phase.ThreatAwareness;
				DrawList.Add(Data);
			}
		});
 

	for (const FDebugDrawData& Data : DrawList)
	{
		if (DebugLevel >= 2 && !Data.TargetLocation.IsNearlyZero())
		{
			DrawDebugLine(World, Data.Location,
				Data.TargetLocation + FVector(0.f, 0.f, 10.f),
				Data.Color, false, -1.f, 0, 2.f);
 
			DrawDebugSphere(World, Data.TargetLocation + FVector(0.f, 0.f, 10.f),
				20.f, 6, Data.Color, false, -1.f);
		}
 
		if (DebugLevel >= 2 && Data.ThreatAwareness > 0.01f)
		{
			const FVector BarBase = Data.Location + FVector(0.f, 0.f, 55.f);
			const FVector BarEnd  = BarBase + FVector(Data.ThreatAwareness * 60.f, 0.f, 0.f);
			DrawDebugLine(World, BarBase, BarEnd, FColor::Red, false, -1.f, 0, 4.f);
		}
 
		DrawDebugString(World, Data.Location + FVector(0.f, 0.f, 20.f),
			Data.Label, nullptr, Data.Color, 0.f, true);
	}
#endif
}
