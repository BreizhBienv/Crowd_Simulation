// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdBoidProcessor.h"
#include "CrowdFragments.h"
#include "CrowdTags.h"
#include "MassNavigationFragments.h"
#include "MassZoneGraphNavigationFragments.h"
#include "MassExecutionContext.h"
#include "MassActorSubsystem.h"          
#include "MassCommonFragments.h" 
#include "MassCommonTypes.h"
#include "MassEntityTypes.h"
#include "MassMovementFragments.h"  

UCrowdBoidProcessor::UCrowdBoidProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
	ExecutionOrder.ExecuteBefore.Add(TEXT("CrowdWalkingProcessor"));
	bAutoRegisterWithProcessingPhases = true;
}

void UCrowdBoidProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdFragments>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCrowdBoidsFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FSharedCrowdSettings>(EMassFragmentPresence::All);

	EntityQuery.AddTagRequirement<FTagPassiveWalking>(EMassFragmentPresence::All);

	EntityQuery.RegisterWithProcessor(*this);
}

void UCrowdBoidProcessor::Execute(FMassEntityManager & Manager, FMassExecutionContext & Context)
{
	struct FAgentSnapshot
	{
		FVector         Location;
		FVector         Velocity;
		EPersonalityType Personality;
		int32           GroupID;
		float           CompatibilityRadius;
		FMassEntityHandle Handle;
	};

	struct FGroupData 
	{ 
		FVector Centroid; 
		FVector AvgVelocity; 
	};


	TArray<FAgentSnapshot> allAgents;
	allAgents.Reserve(Context.GetNumEntities());

	TMap<int32, TArray<int32>> GroupMap;
	TMap<int32, FGroupData> GroupDataMap;
	const float SeparationWeight = 0.5f;
	const float AlignementWeight = 0.5f;
	const float cohesionWeight = 0.5f;

	EntityQuery.ForEachEntityChunk(Context,
		[&allAgents](FMassExecutionContext& Ctx)
		{
			const int32 NumEntities = Ctx.GetNumEntities();

			const auto Transforms = Ctx.GetFragmentView<FTransformFragment>();
			const auto Personalities = Ctx.GetFragmentView<FCrowdFragments>();
			const auto Boids = Ctx.GetFragmentView<FCrowdBoidsFragment>();
			const auto MoveTargets = Ctx.GetFragmentView<FMassMoveTargetFragment>();

			for (int32 i = 0; i < NumEntities; i++)
			{
				FAgentSnapshot agent;
				agent.Location = Transforms[i].GetTransform().GetLocation();
				agent.GroupID = Boids[i].GroupID;
				agent.Personality = Personalities[i].Personality;
				agent.CompatibilityRadius = Boids[i].CompatibilityRadius;
				agent.Handle = Ctx.GetEntity(i);

				agent.Velocity = MoveTargets[i].Forward * (float)MoveTargets[i].DesiredSpeed.Get();

				allAgents.Add(agent);
			}
		});

		if(allAgents.IsEmpty()){ return;}

		static int32 groupeID = 0;
		for (int32 i = 0; i < allAgents.Num(); i++)
		{
			if (allAgents[i].GroupID != -1) continue;

			for (int32 j = 0; j < allAgents.Num(); j++)
			{
				if (allAgents[j].GroupID != -1) continue;

				const float Dist = FVector::Dist(allAgents[i].Location, allAgents[j].Location);
				const float Raduis = FMath::Min(allAgents[i].CompatibilityRadius, allAgents[j].CompatibilityRadius);

				if (Dist > Raduis) continue;


				bool isLeader = allAgents[i].Personality == EPersonalityType::Leader || allAgents[j].Personality == EPersonalityType::Leader;

				const int32 personalityScroreDiff = FMath::Abs((int32)allAgents[i].Personality - (int32)allAgents[j].Personality);
				int32 personalityScoreThreshold = 1;

				if (!isLeader && personalityScroreDiff > personalityScoreThreshold) continue;

				int32 NextGroupID = 0;

				groupeID = (allAgents[i].GroupID != -1) ? allAgents[i].GroupID : ++NextGroupID;
				allAgents[i].GroupID = groupeID;
				allAgents[j].GroupID = groupeID;

			}


			for (int32 i = 0; i < allAgents.Num(); ++i)
			{
				if (allAgents[i].GroupID != -1)
					GroupMap.FindOrAdd(allAgents[i].GroupID).Add(i);
			}


			for (auto& [GID, Members] : GroupMap)
			{
				FVector SumPos(0), SumVel(0);
				for (int32 idx : Members)
				{
					SumPos += allAgents[idx].Location;
					SumVel += allAgents[idx].Velocity;
				}
				const float Inv = 1.f / Members.Num();
				GroupDataMap.Add(GID, { SumPos * Inv, SumVel * Inv });
			}

			EntityQuery.ForEachEntityChunk(Context,
				[&allAgents, &GroupDataMap](FMassExecutionContext& Ctx)
				{

				});

			FVector separationVector = FVector::ZeroVector;
			FVector alignementVector = FVector::ZeroVector;
			FVector cohesionVector = FVector::ZeroVector;
			for (int i = 0; i < allAgents.Num(); i++)
			{
				for (int y = 0; y < allAgents.Num(); y++)
				{
					if (i == y) continue;
					if (allAgents[i].GroupID != allAgents[y].GroupID) continue;
					const float Dist = FVector::Dist(allAgents[i].Location, allAgents[y].Location);
					const float Raduis = FMath::Min(allAgents[i].CompatibilityRadius, allAgents[y].CompatibilityRadius);
					if (Dist > Raduis) continue;
					//separation
					for (FAgentSnapshot agent : allAgents)
					{
						if (allAgents[i].GroupID != allAgents[y].GroupID) continue;
						separationVector += (allAgents[i].Location - allAgents[y].Location).GetSafeNormal() / Dist;

					}
					
					allAgents[i].Velocity += separationVector * SeparationWeight;

					//alignmen t
					for (FAgentSnapshot agent : allAgents)
					{
						if (allAgents[i].GroupID != allAgents[y].GroupID) continue;
						alignementVector += allAgents[y].Velocity;
					}
					alignementVector /= allAgents.Num() - 1;
					alignementVector -= allAgents[i].Velocity;

					alignementVector.Normalize();

					allAgents[i].Velocity += alignementVector * AlignementWeight;

					//cohesion
					for (FAgentSnapshot agent : allAgents)
					{
						if (allAgents[i].GroupID != allAgents[y].GroupID) continue;
						cohesionVector += allAgents[y].Location;
					}

					cohesionVector /= allAgents.Num() - 1;
					cohesionVector -= allAgents[i].Location;

					cohesionVector.Normalize();

					allAgents[i].Velocity += cohesionVector * cohesionWeight;
				}
			}

	});
}
