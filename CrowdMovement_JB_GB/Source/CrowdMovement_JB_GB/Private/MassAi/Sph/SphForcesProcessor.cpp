	// Fill out your copyright notice in the Description page of Project Settings.


#include "MassAi/Sph/SphForcesProcessor.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "CrowdMovement_JB_GB/CrowdFragments.h"
#include "CrowdMovement_JB_GB/CrowdTags.h"
#include "SphAgent/SphHelper.h"

USphForcesProcessor::USphForcesProcessor()
{
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::SphComponents);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::SphForces;
    ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::UpdateWorldFromMass); //May be wrong : the goal is to execute before the data from mass is sent to the world
	bAutoRegisterWithProcessingPhases = true;
}

void USphForcesProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);
	
	EntityQuery.Initialize(EntityManager);
	
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FSphFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FSphForcesFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSharedRequirement<FSharedSphCrowdSettings>(EMassFragmentAccess::ReadOnly);
	
	EntityQuery.AddTagRequirement<FTagSph>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void USphForcesProcessor::Execute(FMassEntityManager& Manager, FMassExecutionContext& Context)
{
	//Super::Execute(Manager, Context);
	
	UWorld* World = Manager.GetWorld();
	EntityQuery.ForEachEntityChunk(Context,
		[World](FMassExecutionContext& Ctx)
		{
			const int32 NumEntities = Ctx.GetNumEntities();
			
			const TConstArrayView<FTransformFragment>	transforms = Ctx.GetFragmentView<FTransformFragment>();
			const TConstArrayView<FSphFragment>			sphParameters = Ctx.GetFragmentView<FSphFragment>();
			const TArrayView<FSphForcesFragment>		sphForces = Ctx.GetMutableFragmentView<FSphForcesFragment>();
			const TArrayView<FMassVelocityFragment>		VelocityFragments = Ctx.GetMutableFragmentView<FMassVelocityFragment>();
			const FSharedSphCrowdSettings&				settings = Ctx.GetSharedFragment<FSharedSphCrowdSettings>();
			
			for (int i = 0; i < NumEntities; i++)
			{
				const FVector2D& ri = SphHelper::ConvertVectorDimension(transforms[i].GetTransform().GetLocation());
				const FVector& vi = VelocityFragments[i].Value;
				const float h = sphParameters[i].h;
				const float pi = sphForces[i].pressure;
				
				
				FVector2D fPressureNeighbours = FVector2D::ZeroVector, fViscosity = FVector2D::ZeroVector;
				FVector2D fPressureObstacles = SphHelper::ComputePressureFromObstacles(
					ri, h, pi, sphParameters[i].obstacleAreaCoef, settings.ObstaclesSegments);
				
				for (int j = 0; j < NumEntities; j++)
				{
					if (i == j)
						continue;
					
					const FVector2D& rj = SphHelper::ConvertVectorDimension(transforms[j].GetTransform().GetLocation());
					const FVector& vj = VelocityFragments[j].Value;
					const float pj = sphForces[j].pressure;
					const float densityj = sphForces[j].density;
					const float mj = sphParameters[j].mass;
					
					fPressureNeighbours += SphHelper::ComputePressureFromNeighbour(ri, pi, h, rj, pj, densityj, mj);
					fViscosity += SphHelper::ComputeViscosityFromNeighbour(ri, vi, h, rj, vj, densityj, mj);
				}
				
				const FVector acceleration = SphHelper::ComputeAcceleration(
					 -fPressureNeighbours + fPressureObstacles, 
					settings.muViscosity * fViscosity, 
					sphForces[i].density);
				
				//VelocityFragments[i].Value += acceleration;
				sphForces[i].acceleration = acceleration;
				
				//Debug
				const FVector normalizedAcceleration = acceleration.GetSafeNormal();
				const FVector normalizedVelocity = VelocityFragments[i].Value.GetSafeNormal();
				DrawDebugLine(World, transforms[i].GetTransform().GetLocation(), transforms[i].GetTransform().GetLocation() + normalizedAcceleration * 100.f, FColor::Yellow, false, -1, 0, 5);
				DrawDebugLine(World, transforms[i].GetTransform().GetLocation(), transforms[i].GetTransform().GetLocation() + normalizedVelocity * 100.f, FColor::Magenta, false, -1, 0, 5);
			}
		});
}

