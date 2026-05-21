#include "CrowdFleeingProcessor.h"
#include "CrowdMovement_JB_GB/CrowdFragments.h"
#include "CrowdMovement_JB_GB/CrowdTags.h"
#include "CrowdMovement_JB_GB/CrowdPOISubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassNavigationFragments.h"
#include "MassEntityManager.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

UCrowdFleeingProcessor::UCrowdFleeingProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
	ExecutionOrder.ExecuteAfter.Add(TEXT("CrowdThreatProcessor"));
	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = true; // NavMesh requiert le Game Thread
}

void UCrowdFleeingProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
	
	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FSphForcesFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdPhaseFragments>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCrowdPOIFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
 
	EntityQuery.AddConstSharedRequirement<FSharedCrowdSettings>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FTagActiveFleeing>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UCrowdFleeingProcessor::Execute(FMassEntityManager& Manager, FMassExecutionContext& Context)
{
	UWorld* World = Manager.GetWorld();
	if (!World) return;

	UNavigationSystemV1* NavSys      = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	UCrowdPOISubsystem*  POISubsystem = World->GetSubsystem<UCrowdPOISubsystem>();
 
	EntityQuery.ForEachEntityChunk(Context,
		[World, NavSys, POISubsystem](FMassExecutionContext& Ctx)
		{
			const float DeltaTime   = Ctx.GetDeltaTimeSeconds();
			const int32 NumEntities = Ctx.GetNumEntities();
 
			const TConstArrayView<FCrowdFragments>    Personalities = Ctx.GetFragmentView<FCrowdFragments>();
			const TConstArrayView<FTransformFragment> Transforms    = Ctx.GetFragmentView<FTransformFragment>();
			const TConstArrayView<FSphForcesFragment> SphForces		= Ctx.GetFragmentView<FSphForcesFragment>();
			const TArrayView<FCrowdPhaseFragments>    Phases        = Ctx.GetMutableFragmentView<FCrowdPhaseFragments>();
			const TArrayView<FCrowdPOIFragment>       POIs          = Ctx.GetMutableFragmentView<FCrowdPOIFragment>();
			const TArrayView<FMassMoveTargetFragment> MoveTargets   = Ctx.GetMutableFragmentView<FMassMoveTargetFragment>();
 
			constexpr float ArrivalThreshold = 120.f;
			constexpr float WaypointRadius   = 80.f;
			
			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FCrowdFragments&   Personality = Personalities[i];
				FCrowdPhaseFragments&    Phase       = Phases[i];
				FCrowdPOIFragment&       POI         = POIs[i];
				FMassMoveTargetFragment& MoveTarget  = MoveTargets[i];
 
				const FVector CurrentLocation = Transforms[i].GetTransform().GetLocation();
				if (CurrentLocation.IsNearlyZero()) continue;

				Phase.PhaseTimer += DeltaTime;

				
				if (POI.TargetPOILocation.IsNearlyZero())
				{
					if (POI.bHasReservation && POI.TargetPOIActor.IsValid() && POISubsystem)
					{
						POISubsystem->ReleaseReservation(POI.TargetPOIActor.Get());
						POI.bHasReservation = false;
					}

					bool bFoundPod = false;
					if (POISubsystem)
					{
						TArray<FCrowdPOIData> Pods;
						POISubsystem->GetEscapePodsOrderedByDistance(CurrentLocation, Pods);

						for (FCrowdPOIData& EscapePod : Pods)
						{
							if (!EscapePod.SourceActor.IsValid()) continue;
							if (POISubsystem->TryReserveOccupant(EscapePod.SourceActor.Get()))
							{
								POI.TargetPOILocation = EscapePod.Location;
								POI.TargetPOIActor    = EscapePod.SourceActor.Get();
								POI.bHasReservation   = true;
								POI.bHasNavPath       = false;
								POI.NavPathPointCount = 0;
								bFoundPod             = true;
								break;
							}
						}

				
						if (!bFoundPod && Pods.Num() > 0 && Pods[0].SourceActor.IsValid())
						{
							POI.TargetPOILocation = Pods[0].Location;
							POI.TargetPOIActor    = Pods[0].SourceActor.Get();
							POI.bHasReservation   = false;
							POI.bHasNavPath       = false;
							POI.NavPathPointCount = 0;
							bFoundPod             = true;
						}
					}

					if (!bFoundPod) continue;
				}

			
				if (!POI.bHasNavPath && NavSys)
				{
					UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(
						World, CurrentLocation, POI.TargetPOILocation, nullptr);

					POI.NavPathPointCount    = 0;
					POI.CurrentNavPointIndex = 1;

					if (NavPath && NavPath->IsValid() && NavPath->PathPoints.Num() > 1)
					{
						const int32 Count = FMath::Min(NavPath->PathPoints.Num(), FCrowdPOIFragment::MaxNavPoints);
						for (int32 p = 0; p < Count; ++p)
							POI.NavPathPoints[p] = NavPath->PathPoints[p];
						POI.NavPathPointCount = Count;
					}
					else
					{
		
						if (POI.bHasReservation && POI.TargetPOIActor.IsValid() && POISubsystem)
						{
							POISubsystem->ReleaseReservation(POI.TargetPOIActor.Get());
							POI.bHasReservation = false;
						}
						POI.TargetPOILocation = FVector::ZeroVector;
						POI.TargetPOIActor    = nullptr;
						continue;
					}

					POI.bHasNavPath = true;
				}


				const float FleeSpeedMultiplier = 1.5f + (1.f - Personality.CourageScore) * 0.8f;
				float DesiredSpeed = Personality.SpeedBase * FleeSpeedMultiplier;
				if (Personality.Personality == EPersonalityType::Leader)
					DesiredSpeed *= 0.85f;


				if (POI.bHasNavPath && POI.CurrentNavPointIndex < POI.NavPathPointCount)
				{
					const FVector NextWaypoint  = POI.NavPathPoints[POI.CurrentNavPointIndex];
					const float DistToWaypoint  = FVector::Dist2D(CurrentLocation, NextWaypoint);

					if (DistToWaypoint < WaypointRadius)
					{
						POI.CurrentNavPointIndex++;

						// Dernier waypoint atteint = arrivée au pod
						if (POI.CurrentNavPointIndex >= POI.NavPathPointCount)
						{
							const float DistToPod = FVector::Dist2D(CurrentLocation, POI.TargetPOILocation);
							if (DistToPod < ArrivalThreshold)
							{
								// Confirmer la réservation ou tenter d'entrer
								if (POI.bHasReservation && POI.TargetPOIActor.IsValid() && POISubsystem)
								{
									POISubsystem->ConfirmReservationAsOccupant(POI.TargetPOIActor.Get());
									POI.bHasReservation = false;
								}
								else if (POI.TargetPOIActor.IsValid() && POISubsystem)
								{
									if (!POISubsystem->TryReserveOccupant(POI.TargetPOIActor.Get()))
									{
										// Pod plein à l'arrivée chercher le suivant
										POI.TargetPOILocation = FVector::ZeroVector;
										POI.TargetPOIActor    = nullptr;
										POI.bHasNavPath       = false;
										POI.NavPathPointCount = 0;
										continue;
									}
									POISubsystem->ConfirmReservationAsOccupant(POI.TargetPOIActor.Get());
								}

								MoveTarget.Center       = CurrentLocation;
								MoveTarget.DesiredSpeed = FMassInt16Real(0.f);
								MoveTarget.IntentAtGoal = EMassMovementAction::Stand;

								POI.TargetPOILocation = FVector::ZeroVector;
								POI.TargetPOIActor    = nullptr;
								POI.bHasReservation   = false;
								POI.bHasNavPath       = false;
								POI.NavPathPointCount = 0;

								Phase.PreviousPhase   = Phase.CurrentPhase;
								Phase.CurrentPhase    = ECrowdPhase::Passive_Observation;
								Phase.PhaseTimer      = 0.f;
								Phase.ThreatAwareness = 0.f;

								Ctx.Defer().RemoveTag<FTagActiveFleeing>(Ctx.GetEntity(i));
								Ctx.Defer().AddTag<FTagPassiveObserving>(Ctx.GetEntity(i));
							}
							continue;
						}
					}

					const FVector ToWaypoint = (NextWaypoint - CurrentLocation).GetSafeNormal2D();
					const FVector SphPush    = SphForces[i].acceleration.GetSafeNormal2D();
					
					constexpr float SphWeight    = 0.65f;
					constexpr float LookAhead    = 200.f;
					const FVector BlendedDir     = (ToWaypoint + SphPush * SphWeight).GetSafeNormal2D();
					const FVector VirtualTarget  = CurrentLocation + BlendedDir * LookAhead;
					
					MoveTarget.CreateNewAction(EMassMovementAction::Move, *World);
					MoveTarget.Center         = VirtualTarget;
					MoveTarget.Forward        = BlendedDir;
					MoveTarget.DesiredSpeed.Set(DesiredSpeed);
					MoveTarget.IntentAtGoal   = EMassMovementAction::Stand;
					MoveTarget.SlackRadius    = 80.f;
					MoveTarget.DistanceToGoal = FVector::Dist2D(CurrentLocation, POI.TargetPOILocation);
					
					DrawDebugLine(World, CurrentLocation, VirtualTarget, FColor::Orange, false, -1, 0, 5);
				}
			}
		});
}