// Fill out your copyright notice in the Description page of Project Settings.

#include "CrowdWalkingProcessor.h"

#include "CrowdFragments.h"
#include "CrowdTags.h"
#include "MassNavigationFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "CrowdPOISubsystem.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"


UCrowdWalkingProcessor::UCrowdWalkingProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
	ExecutionOrder.ExecuteAfter.Add(TEXT("CrowdInitProcessor"));
	bAutoRegisterWithProcessingPhases = true;

	// FindPathToLocationSynchronously requiert le Game Thread
	bRequiresGameThreadExecution = true;
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
	EntityQuery.AddRequirement<FSphForcesFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.AddConstSharedRequirement<FSharedCrowdSettings>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FTagPassiveWalking>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FTagSphDebug>(EMassFragmentPresence::None);

	EntityQuery.RegisterWithProcessor(*this);
}

void UCrowdWalkingProcessor::Execute(FMassEntityManager& Manager, FMassExecutionContext& Context)
{
	UWorld* World = Manager.GetWorld();
	if (!World) return;

	UNavigationSystemV1* NavSys    = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	UCrowdPOISubsystem*  Subsystem = World->GetSubsystem<UCrowdPOISubsystem>();

	EntityQuery.ForEachEntityChunk(Context,
		[World, NavSys, Subsystem](FMassExecutionContext& Ctx)
		{
			const float DeltaTime   = Ctx.GetDeltaTimeSeconds();
			const int32 NumEntities = Ctx.GetNumEntities();

			const TConstArrayView<FCrowdFragments>      Personalities = Ctx.GetFragmentView<FCrowdFragments>();
			const TConstArrayView<FTransformFragment>   Transforms    = Ctx.GetFragmentView<FTransformFragment>();
			const TArrayView<FCrowdPhaseFragments>      Phases        = Ctx.GetMutableFragmentView<FCrowdPhaseFragments>();
			const TArrayView<FMassMoveTargetFragment>   MoveTargets   = Ctx.GetMutableFragmentView<FMassMoveTargetFragment>();
			const TArrayView<FCrowdPOIFragment>         POIs          = Ctx.GetMutableFragmentView<FCrowdPOIFragment>();
			const TArrayView<FCrowdRoleFragment>        Roles         = Ctx.GetMutableFragmentView<FCrowdRoleFragment>();
			const TArrayView<FCrowdSocialGroupFragment> SocialGroups  = Ctx.GetMutableFragmentView<FCrowdSocialGroupFragment>();
			const FSharedCrowdSettings& Settings                      = Ctx.GetConstSharedFragment<FSharedCrowdSettings>();
			const TConstArrayView<FSphForcesFragment> SphForces        = Ctx.GetFragmentView<FSphForcesFragment>();

			constexpr float WanderRadius   = 1200.f;
			constexpr float WaypointRadius = 80.f;
			constexpr float WalkTimeoutSec = 30.f;

			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FCrowdFragments&     Personality = Personalities[i];
				FCrowdPhaseFragments&      Phase       = Phases[i];
				FMassMoveTargetFragment&   MoveTarget  = MoveTargets[i];
				FCrowdPOIFragment&         POI         = POIs[i];
				FCrowdRoleFragment&        Role        = Roles[i];
				FCrowdSocialGroupFragment& SocialGroup = SocialGroups[i];

				const FVector CurrentLocation = Transforms[i].GetTransform().GetLocation();

				// Ignorer les agents non encore placés dans le monde
				if (CurrentLocation.IsNearlyZero()) continue;

		
				Role.DutyTimer += DeltaTime;
				if (Role.DutyTimer >= Role.DutyDuration)
				{
					Role.bOnDuty          = !Role.bOnDuty;
					Role.DutyTimer        = 0.f;
					POI.TargetPOILocation = FVector::ZeroVector;
					POI.TargetPOIActor    = nullptr;
					POI.bHasNavPath       = false;
					POI.NavPathPointCount = 0;
					if (SocialGroup.bIsLeader)
						SocialGroup.SharedDestination = FVector::ZeroVector;
				}

				if (POI.TargetPOILocation.IsNearlyZero())
				{
					if (!SocialGroup.bIsLeader && SocialGroup.GroupID >= 0)
					{
						// attendre la destination du leader
						if (!SocialGroup.SharedDestination.IsNearlyZero())
						{
							POI.TargetPOILocation = SocialGroup.SharedDestination;
							POI.bHasNavPath       = false;
							POI.NavPathPointCount = 0;
						}
						else continue;
					}
					else
					{
						// Solitaire ou leader chercher un POI
						bool bFoundPOI = false;
						FCrowdPOIData ChosenPOI;

						if (Subsystem)
						{
							if (Role.bOnDuty)
								bFoundPOI = Subsystem->GetRandomPOIForRole(CurrentLocation, WanderRadius, Role.Role, ChosenPOI);
							else
								bFoundPOI = Subsystem->GetRandomSocialPOI(CurrentLocation, WanderRadius, ChosenPOI);
						}

						if (bFoundPOI)
						{
							POI.TargetPOILocation   = ChosenPOI.Location;
							POI.TargetPOIActor      = ChosenPOI.SourceActor.Get();
							POI.ObservationDuration = FMath::Lerp(
								ChosenPOI.ObservationDuration * 0.5f,
								ChosenPOI.ObservationDuration * 1.5f,
								Personality.CuriosityScore);
						}
						else
						{
							
							const float Angle = FMath::FRandRange(0.f, 2.f * PI);
							const float Dist  = FMath::FRandRange(300.f, WanderRadius * 0.5f);
							const FVector WanderTarget = CurrentLocation + FVector(
								FMath::Cos(Angle) * Dist,
								FMath::Sin(Angle) * Dist,
								0.f);

							// Verifier que la cible n'est pas nulle
							if (!WanderTarget.IsNearlyZero())
								POI.TargetPOILocation = WanderTarget;
							else
								continue; // skip ce tick, reessayer au prochain
						}

						POI.bHasNavPath       = false;
						POI.NavPathPointCount = 0;

						if (SocialGroup.bIsLeader)
							SocialGroup.SharedDestination = POI.TargetPOILocation;
					}
				}

	
				if (!POI.bHasNavPath && !POI.TargetPOILocation.IsNearlyZero() && NavSys)
				{
					UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(
						World,
						CurrentLocation,
						POI.TargetPOILocation,
						nullptr);

					POI.NavPathPointCount    = 0;
					POI.CurrentNavPointIndex = 1;

					if (NavPath && NavPath->IsValid() && NavPath->PathPoints.Num() > 1)
					{
						const int32 Count = FMath::Min(
							NavPath->PathPoints.Num(),
							FCrowdPOIFragment::MaxNavPoints);

						for (int32 p = 0; p < Count; ++p)
							POI.NavPathPoints[p] = NavPath->PathPoints[p];

						POI.NavPathPointCount = Count;
					}
					else
					{
						POI.TargetPOILocation = FVector::ZeroVector;
						continue;
					}

					POI.bHasNavPath = true;
				}

				float SpeedMultiplier = 1.0f;
				switch (Personality.Personality)
				{
					case EPersonalityType::Calm:    SpeedMultiplier = 0.85f; break;
					case EPersonalityType::Social:  SpeedMultiplier = 1.0f;  break;
					case EPersonalityType::Nervous: SpeedMultiplier = 1.2f;  break;
					case EPersonalityType::Leader:  SpeedMultiplier = 1.15f; break;
				}

				if (SocialGroup.GroupID >= 0 && !SocialGroup.bIsLeader)
				{
					const float DistToShared = FVector::Dist2D(CurrentLocation, SocialGroup.SharedDestination);
					if (DistToShared > Settings.WalkTogetherRadius)
						SpeedMultiplier *= Settings.CatchUpSpeedMultiplier;
				}

				const float DesiredSpeed = Personality.SpeedBase * SpeedMultiplier;

				Phase.PhaseTimer += DeltaTime;

				if (Phase.PhaseTimer > WalkTimeoutSec && POI.bHasNavPath)
				{
					POI.TargetPOILocation = FVector::ZeroVector;
					POI.bHasNavPath       = false;
					POI.NavPathPointCount = 0;
					Phase.PhaseTimer      = 0.f;
					if (SocialGroup.bIsLeader)
						SocialGroup.SharedDestination = FVector::ZeroVector;
					continue;
				}

				if (POI.bHasNavPath && POI.CurrentNavPointIndex < POI.NavPathPointCount)
				{
					const FVector NextWaypoint = POI.NavPathPoints[POI.CurrentNavPointIndex];
					const float DistToWaypoint = FVector::Dist2D(CurrentLocation, NextWaypoint);


					if (DistToWaypoint < WaypointRadius)
					{
						POI.CurrentNavPointIndex++;


						if (POI.CurrentNavPointIndex >= POI.NavPathPointCount)
						{
							MoveTarget.Center       = CurrentLocation;
							MoveTarget.DesiredSpeed = FMassInt16Real(0.f);
							MoveTarget.IntentAtGoal = EMassMovementAction::Stand;

							POI.bIsObserving        = true;
							POI.ObservationTimer    = 0.f;
							POI.bHasNavPath         = false;
							POI.NavPathPointCount   = 0;
							POI.TargetPOILocation   = FVector::ZeroVector;

							if (SocialGroup.bIsLeader)
								SocialGroup.SharedDestination = FVector::ZeroVector;

							Phase.PreviousPhase = Phase.CurrentPhase;
							Phase.CurrentPhase  = ECrowdPhase::Passive_Observation;
							Phase.PhaseTimer    = 0.f;

							Ctx.Defer().RemoveTag<FTagPassiveWalking>(Ctx.GetEntity(i));
							Ctx.Defer().AddTag<FTagPassiveObserving>(Ctx.GetEntity(i));
							continue;
						}
					}

					const FVector ToWaypoint = (NextWaypoint - CurrentLocation).GetSafeNormal2D();
					const FVector SphPush    = SphForces[i].acceleration.GetSafeNormal2D();

					// SPH poids reduit (0.3) pour que le chemin NavMesh reste dominant
					constexpr float SphWeight    = 0.3f;
					constexpr float LookAhead    = 200.f;
					const FVector BlendedDir     = (ToWaypoint + SphPush * SphWeight).GetSafeNormal2D();
					const FVector VirtualTarget  = CurrentLocation + BlendedDir * LookAhead;

					MoveTarget.CreateNewAction(EMassMovementAction::Move, *World);
					MoveTarget.DesiredSpeed.Set(DesiredSpeed);
					MoveTarget.IntentAtGoal   = EMassMovementAction::Stand;
					MoveTarget.SlackRadius    = 80.f;
					MoveTarget.Center         = VirtualTarget;
					MoveTarget.Forward        = BlendedDir;
					// Distance reelle au POI final pour la detection d arrivee
					MoveTarget.DistanceToGoal = FVector::Dist2D(CurrentLocation, POI.TargetPOILocation);
				}
				else if (!POI.TargetPOILocation.IsNearlyZero())
				{
					// Chemin pas encore calcule ce tick
					MoveTarget.DesiredSpeed = FMassInt16Real(0.f);
					MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
				}
			}
		});
}
