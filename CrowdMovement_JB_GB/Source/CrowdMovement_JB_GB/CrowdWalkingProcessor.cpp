#include "CrowdWalkingProcessor.h"
 
#include "CrowdFragments.h"
#include "CrowdTags.h"
#include "MassNavigationFragments.h"
#include "MassZoneGraphNavigationFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "CrowdPOISubsystem.h"

 
 
UCrowdWalkingProcessor::UCrowdWalkingProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
	ExecutionOrder.ExecuteAfter.Add(TEXT("CrowdInitProcessor"));
	bAutoRegisterWithProcessingPhases = true;
}
 
void UCrowdWalkingProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
 
	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdPhaseFragments>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdPOIFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdRoleFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdSocialGroupFragment>(EMassFragmentAccess::ReadWrite);
 
	EntityQuery.AddConstSharedRequirement<FSharedCrowdSettings>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FTagPassiveWalking>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FTagSphDebug>(EMassFragmentPresence::None);
 
	EntityQuery.RegisterWithProcessor(*this);
}
 
void UCrowdWalkingProcessor::Execute(FMassEntityManager& Manager, FMassExecutionContext& Context)
{
	UCrowdPOISubsystem * subsystem = Manager.GetWorld()->GetSubsystem<UCrowdPOISubsystem>(); 
	
	EntityQuery.ForEachEntityChunk(Context,
		[subsystem](FMassExecutionContext& Ctx)
		{
			const float DeltaTime = Ctx.GetDeltaTimeSeconds();
			const int32 NumEntities = Ctx.GetNumEntities();
 
			const TConstArrayView<FCrowdFragments>    Personalities = Ctx.GetFragmentView<FCrowdFragments>();
			const TConstArrayView<FTransformFragment> Transforms    = Ctx.GetFragmentView<FTransformFragment>();
			const TArrayView<FCrowdPhaseFragments>    Phases        = Ctx.GetMutableFragmentView<FCrowdPhaseFragments>();
			const TArrayView<FMassMoveTargetFragment> MoveTargets   = Ctx.GetMutableFragmentView<FMassMoveTargetFragment>();
			
			const TArrayView<FCrowdPOIFragment>       POIs          = Ctx.GetMutableFragmentView<FCrowdPOIFragment>();
			const TArrayView<FCrowdRoleFragment>      Roles          = Ctx.GetMutableFragmentView<FCrowdRoleFragment>();
			const TArrayView<FCrowdSocialGroupFragment> SocialGroups = Ctx.GetMutableFragmentView<FCrowdSocialGroupFragment>();
			
			const FSharedCrowdSettings& Settings                     = Ctx.GetConstSharedFragment<FSharedCrowdSettings>();
			
			constexpr float WanderRadius     = 1200.f;
			constexpr float ArrivalThreshold = 150.f;
 
			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FCrowdFragments&    Personality = Personalities[i];
				const FTransformFragment& Transform   = Transforms[i];
				FCrowdPhaseFragments&     Phase       = Phases[i];
				FMassMoveTargetFragment&  MoveTarget  = MoveTargets[i];
				FCrowdPOIFragment&        POI         = POIs[i];
				FCrowdRoleFragment&       Role        = Roles[i];
				FCrowdSocialGroupFragment& SocialGroup = SocialGroups[i];
 
				const FVector CurrentLocation = Transform.GetTransform().GetLocation();
				
				//Agent en service / hort service
				
				Role.DutyTimer += DeltaTime;
				if (Role.DutyTimer >= Role.DutyDuration)
				{
					Role.bOnDuty = !Role.bOnDuty;
					Role.DutyTimer = 0.f;
					POI.TargetPOILocation = FVector::ZeroVector;
					POI.TargetPOIActor = nullptr;
					if (SocialGroup.bIsLeader)
					{
						SocialGroup.SharedDestination = FVector::ZeroVector;
					}
				}
				
				if (POI.TargetPOILocation.IsNearlyZero())
				{
					
					if (!SocialGroup.bIsLeader && SocialGroup.GroupID >= 0)
					{
						if (!SocialGroup.SharedDestination.IsNearlyZero())
							POI.TargetPOILocation = SocialGroup.SharedDestination;
						else
							continue; 
					}
					else
					{
						
						bool bFoundPOI = false;
						FCrowdPOIData chosenPOI;
						
						if (subsystem)
						{
							if (Role.bOnDuty)
								bFoundPOI = subsystem->GetRandomPOIForRole(CurrentLocation, WanderRadius, Role.Role, chosenPOI);
							else
								bFoundPOI = subsystem->GetRandomSocialPOI(CurrentLocation, WanderRadius, chosenPOI);
						}
						if (bFoundPOI)
						{
							POI.TargetPOILocation = chosenPOI.Location;
							POI.TargetPOIActor = chosenPOI.SourceActor.Get();
							POI.ObservationDuration = FMath::Lerp(
								chosenPOI.ObservationDuration * 0.5f,
								chosenPOI.ObservationDuration * 1.5f,
								Personality.CuriosityScore);
						}
						else
						{
							
							const float Angle = FMath::FRandRange(0.f, 2.f * PI);
							const float Dist  = FMath::FRandRange(WanderRadius * 0.2f, WanderRadius * 0.6f);
							POI.TargetPOILocation = CurrentLocation + FVector(
								FMath::Cos(Angle) * Dist,
								FMath::Sin(Angle) * Dist,
								0.f);
						}
						if (SocialGroup.bIsLeader)
						{
							SocialGroup.SharedDestination = POI.TargetPOILocation;
						}
					}
				}
				
				float SpeedMultiplier = 1.0f;
				switch (Personality.Personality)
				{
					case EPersonalityType::Calm:    
						SpeedMultiplier = 0.85f; 
					break;
					
					case EPersonalityType::Social:  
						SpeedMultiplier = 1.0f; 
					break;
					
					case EPersonalityType::Nervous: 
						SpeedMultiplier = 1.2f;  
					break;
					
					case EPersonalityType::Leader:  
						SpeedMultiplier = 1.15f; 
					break;
				}
				
				
				if (SocialGroup.GroupID >= 0 && !SocialGroup.bIsLeader)
				{
					const float DistToShared = FVector::Dist2D(CurrentLocation, SocialGroup.SharedDestination);
					if (DistToShared > Settings.WalkTogetherRadius)
					{
						SpeedMultiplier *= Settings.CatchUpSpeedMultiplier;
					}
				}
 
				const float DesiredSpeed = Personality.SpeedBase * SpeedMultiplier;
 
				
				MoveTarget.Center       = POI.TargetPOILocation;
				MoveTarget.Forward      = (POI.TargetPOILocation - CurrentLocation).GetSafeNormal2D();
				MoveTarget.DesiredSpeed = FMassInt16Real(DesiredSpeed);
				MoveTarget.IntentAtGoal = EMassMovementAction::Move;
 
				Phase.PhaseTimer += DeltaTime;
 
				
				const float DistToTarget = FVector::Dist2D(CurrentLocation, POI.TargetPOILocation);
 
				if (DistToTarget < ArrivalThreshold)
				{
				
					MoveTarget.Center       = CurrentLocation;
					MoveTarget.DesiredSpeed = FMassInt16Real(0.f);
					MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
 
					POI.bIsObserving     = true;
					POI.ObservationTimer = 0.f;
					
					Phase.PreviousPhase = Phase.CurrentPhase;
					Phase.CurrentPhase  = ECrowdPhase::Passive_Observation;
					Phase.PhaseTimer    = 0.f;
 
					
					POI.TargetPOILocation = FVector::ZeroVector;
					if (SocialGroup.bIsLeader)
					{
						SocialGroup.SharedDestination = FVector::ZeroVector;
					}
 
					Ctx.Defer().RemoveTag<FTagPassiveWalking>(Ctx.GetEntity(i));
					Ctx.Defer().AddTag<FTagPassiveObserving>(Ctx.GetEntity(i));
				}
			}
		});
}