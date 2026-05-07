// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdInitObserver.h"
#include "CrowdFragments.h"
#include "crowdTags.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"

static std::atomic<int32> GNextGroupID{ 0 };

UCrowdInitObserver::UCrowdInitObserver()
{
	

	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
	ExecutionOrder.ExecuteBefore.Add(TEXT("CrowdWalkingProcessor"));
	bAutoRegisterWithProcessingPhases = true;
}

void UCrowdInitObserver::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);

	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdPOIFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdRoleFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdSocialGroupFragment>(EMassFragmentAccess::ReadWrite);
	
	EntityQuery.AddTagRequirement<FTagNeedsInit>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UCrowdInitObserver::Execute(FMassEntityManager & Manager, FMassExecutionContext & Contexte)
{
	EntityQuery.ForEachEntityChunk(Contexte,
		[](FMassExecutionContext& Ctx)
		{
			const int32 NumEntities = Ctx.GetNumEntities();
 
			const TArrayView<FCrowdFragments>			Personalities = Ctx.GetMutableFragmentView<FCrowdFragments>();
			const TArrayView<FCrowdPOIFragment>			POIs          = Ctx.GetMutableFragmentView<FCrowdPOIFragment>();
			const TArrayView<FCrowdRoleFragment>		Roles         = Ctx.GetMutableFragmentView<FCrowdRoleFragment>();
			const TArrayView<FCrowdSocialGroupFragment> Socials       = Ctx.GetMutableFragmentView<FCrowdSocialGroupFragment>();
 
			for (int32 i = 0; i < NumEntities; i++)
			{
				FCrowdFragments&          Personality = Personalities[i];
				FCrowdPOIFragment&        POI         = POIs[i];
				FCrowdRoleFragment&       Role        = Roles[i];
				FCrowdSocialGroupFragment& Social     = Socials[i];
				
				Personality.Personality         = static_cast<EPersonalityType>(FMath::RandRange(0, 3));
				Personality.CourageScore        = FMath::FRandRange(0.1f, 0.9f);
				Personality.CuriosityScore      = FMath::FRandRange(0.1f, 0.9f);
				Personality.SocialAffinityScore = FMath::FRandRange(0.2f, 0.8f);
				Personality.SpeedBase           = FMath::FRandRange(120.f, 200.f);
				
				POI.ObservationDuration = FMath::Lerp(3.f, 10.f, Personality.CuriosityScore);
				POI.ObservationTimer    = 0.f;
				POI.bIsObserving        = false;
				POI.TargetPOILocation   = FVector::ZeroVector;
				POI.TargetPOIActor      = nullptr;
				
				Role.DutyTimer    = 0.f;
				Role.DutyDuration = FMath::FRandRange(90.f, 180.f);
				Role.bOnDuty      = true;
				
				Social.GroupID            = -1;
				Social.bIsLeader          = false;
				Social.GroupLeader        = FMassEntityHandle();
				Social.SharedDestination  = FVector::ZeroVector;
				Social.TalkTimer          = 0.f;
				Social.TalkDuration       = 0.f;
			}
			
			constexpr int32 MinGroupSize = 2;
			constexpr int32 MaxGroupSize = 4;
			constexpr float GroupJoinChance  = 0.65f;
			
			int32 i = 0;
			while (i < NumEntities)
			{
				FCrowdSocialGroupFragment& LeaderSocial = Socials[i];
				
				if (FMath::FRand() < GroupJoinChance)
				{
					const int32 NewGroupID   = GNextGroupID.fetch_add(1);
					const int32 GroupSize    = FMath::RandRange(MinGroupSize, MaxGroupSize);
					const FMassEntityHandle LeaderHandle = Ctx.GetEntity(i);
					
					LeaderSocial.GroupID = NewGroupID;
					LeaderSocial.GroupLeader = LeaderHandle;
					LeaderSocial.bIsLeader = true;
					
					int32 MembersAdded = 1;
					int32 j = i + 1;
					while (j < NumEntities && MembersAdded < GroupSize)
					{
						FCrowdSocialGroupFragment& MemberSocial = Socials[j];
						
						if (Personalities[j].SocialAffinityScore >= 0.3f)
						{
							MemberSocial.GroupID    = NewGroupID;
							MemberSocial.bIsLeader  = false;
							MemberSocial.GroupLeader = LeaderHandle;
							MembersAdded++;
						}
						j++;
					}
					
					i = j;
				}
				else
					i++;

			}
			for (int32 k = 0; k < NumEntities; k++)
			{
				Ctx.Defer().RemoveTag<FTagNeedsInit>(Ctx.GetEntity(k));
			}
			
		});
}
