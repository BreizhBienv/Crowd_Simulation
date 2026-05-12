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
	if (DebugLevel== 0)
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
		FVector Location;
		FVector TargetLocation;
		FColor  Color;
		FString Label;
		float   ThreatAwareness;
		bool    bFleeing;
	};

	TArray<FDebugDrawData> DrawList;


    EntityQuery.ForEachEntityChunk(Context,
        [&DrawList, DebugLevel](FMassExecutionContext& Ctx)
        {
 			const int32 NumEntities = Ctx.GetNumEntities();
 
			const TConstArrayView<FTransformFragment>        Transforms  = Ctx.GetFragmentView<FTransformFragment>();
			const TConstArrayView<FCrowdFragments>           Personalities = Ctx.GetFragmentView<FCrowdFragments>();
			const TConstArrayView<FCrowdPhaseFragments>      Phases      = Ctx.GetFragmentView<FCrowdPhaseFragments>();
			const TConstArrayView<FCrowdPOIFragment>         POIs        = Ctx.GetFragmentView<FCrowdPOIFragment>();
			const TConstArrayView<FCrowdRoleFragment>        Roles       = Ctx.GetFragmentView<FCrowdRoleFragment>();
			const TConstArrayView<FCrowdSocialGroupFragment> Socials     = Ctx.GetFragmentView<FCrowdSocialGroupFragment>();
 
			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FVector AgentLocation = Transforms[i].GetTransform().GetLocation() + FVector(0.f, 0.f, 90.f);
				const FCrowdFragments&           Personality = Personalities[i];
				const FCrowdPhaseFragments&      Phase       = Phases[i];
				const FCrowdPOIFragment&         POI         = POIs[i];
				const FCrowdRoleFragment&        Role        = Roles[i];
				const FCrowdSocialGroupFragment& Social      = Socials[i];
 
				
				FColor LineColor;
				FString PhaseName;
				switch (Phase.CurrentPhase)
				{
					case ECrowdPhase::Passive_Walk:
						LineColor = FColor::Green;        PhaseName = TEXT("Walk");      break;
					case ECrowdPhase::Passive_Observation:
						LineColor = FColor::Cyan;         PhaseName = TEXT("Observe");   break;
					case ECrowdPhase::Passive_Discution:
						LineColor = FColor::Yellow;       PhaseName = TEXT("Talk");      break;
					case ECrowdPhase::Active_Fleeing:
						LineColor = FColor::Orange;       PhaseName = TEXT("FLEE");      break;
					case ECrowdPhase::Active_Fighting:
						LineColor = FColor::Red;          PhaseName = TEXT("FIGHT");     break;
					default:
						LineColor = FColor::White;        PhaseName = TEXT("?");         break;
				}
 
				
				FString RoleName;
				switch (Role.Role)
				{
					case ECrewRole::Pilot:    RoleName = TEXT("Pilot");    break;
					case ECrewRole::Engineer: RoleName = TEXT("Engin.");   break;
					case ECrewRole::Medic:    RoleName = TEXT("Medic");    break;
					case ECrewRole::Guard:    RoleName = TEXT("Guard");    break;
					case ECrewRole::Crew:     RoleName = TEXT("Crew");     break;
					default:                  RoleName = TEXT("?");        break;
				}
 
				
				FString PersonalityName;
				switch (Personality.Personality)
				{
					case EPersonalityType::Calm:    PersonalityName = TEXT("Calm");    break;
					case EPersonalityType::Social:  PersonalityName = TEXT("Social");  break;
					case EPersonalityType::Leader:  PersonalityName = TEXT("Leader");  break;
					case EPersonalityType::Nervous: PersonalityName = TEXT("Nervous"); break;
					default:                        PersonalityName = TEXT("?");       break;
				}
 
				
				FString GroupInfo;
				if (Social.GroupID >= 0)
				{
					GroupInfo = FString::Printf(TEXT("G%d%s"),
						Social.GroupID,
						Social.bIsLeader ? TEXT("*") : TEXT(""));
				}
				else
				{
					GroupInfo = TEXT("Solo");
				}
 
				
				FString Label;
				if (DebugLevel >= 2)
				{
					
					Label = FString::Printf(
						TEXT("[%s] %s\n%s | %s\nThreat: %.2f"),
						*RoleName,
						*PhaseName,
						*PersonalityName,
						*GroupInfo,
						Phase.ThreatAwareness
					);
				}
				else
				{
					
					Label = FString::Printf(
						TEXT("[%s] %s\n%s"),
						*RoleName,
						*PhaseName,
						*GroupInfo
					);
				}
 
				FDebugDrawData Data;
				Data.Location        = AgentLocation;
				Data.TargetLocation  = POI.TargetPOILocation;
				Data.Color           = LineColor;
				Data.Label           = Label;
				Data.ThreatAwareness = Phase.ThreatAwareness;
				Data.bFleeing        = (Phase.CurrentPhase == ECrowdPhase::Active_Fleeing);
				DrawList.Add(Data);
			}
		});
	
	for (const FDebugDrawData& Data : DrawList)
	{

		if (DebugLevel >= 2 && !Data.TargetLocation.IsNearlyZero())
		{
			DrawDebugLine(
				World,
				Data.Location,
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
		
		if (DebugLevel >= 2 && Data.ThreatAwareness > 0.01f)
		{
			const FVector BarBase  = Data.Location + FVector(0.f, 0.f, 55.f);
			const FVector BarEnd   = BarBase + FVector(Data.ThreatAwareness * 60.f, 0.f, 0.f);
			DrawDebugLine(World, BarBase, BarEnd, FColor::Red, false, -1.f, 0, 4.f);
		}
		
		DrawDebugString(
			World,
			Data.Location + FVector(0.f, 0.f, 20.f),
			Data.Label,
			nullptr,
			Data.Color,
			0.f,
			true
		);
	}
#endif
}
