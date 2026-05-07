// Fill out your copyright notice in the Description page of Project Settings.


#include "MassAi/Sph/SphComponentsProcessor.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "CrowdMovement_JB_GB/CrowdFragments.h"
#include "CrowdMovement_JB_GB/CrowdTags.h"
#include "SphAgent/SphHelper.h"
#include "SphAgent/SphObstacleHelper.h"

USphComponentsProcessor::USphComponentsProcessor()
{
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::SphComponents;
    ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::SphForces);
	bAutoRegisterWithProcessingPhases = true;
}

void USphComponentsProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);
	
	EntityQuery.Initialize(EntityManager);
	
	EntityQuery.AddRequirement<FSphFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FSphForcesFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSharedRequirement<FSharedSphCrowdSettings>(EMassFragmentAccess::ReadOnly);
	
	EntityQuery.AddTagRequirement<FTagSph>(EMassFragmentPresence::All);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void USphComponentsProcessor::Execute(FMassEntityManager& Manager, FMassExecutionContext& Context)
{
	Super::Execute(Manager, Context);
	
	EntityQuery.ForEachEntityChunk(Context,
		[](FMassExecutionContext& Ctx)
		{
			const int32 NumEntities = Ctx.GetNumEntities();
			
			const TConstArrayView<FTransformFragment>	transforms = Ctx.GetFragmentView<FTransformFragment>();
			const TConstArrayView<FSphFragment>			sphParameters = Ctx.GetFragmentView<FSphFragment>();
			const TArrayView<FSphForcesFragment>		sphForces = Ctx.GetMutableFragmentView<FSphForcesFragment>();
			const FSharedSphCrowdSettings&				settings = Ctx.GetSharedFragment<FSharedSphCrowdSettings>();
			
			for (int i = 0; i < NumEntities; i++)
			{
				const FVector2D& ri = SphHelper::ConvertVectorDimension(transforms[i].GetTransform().GetLocation());
				
				float density = 0;
				
				//Compute Entities density
				for (int j = 0; j < NumEntities; j++)
				{
					if (i == j)
						continue;
					
					const FVector2D& rj = SphHelper::ConvertVectorDimension(transforms[j].GetTransform().GetLocation());
					density += SphHelper::ComputeDensityFromNeighbour(ri, rj, sphParameters[i].h, sphParameters[j].mass);
				}
				
				//Compute Obstacles density
				float densityFromObstacles = SphHelper::ComputeDensityFromObstacles(
					ri,
					sphParameters[i].h,
					sphParameters[i].comfortDensity,
					sphParameters[i].obstacleAreaCoef,
					settings.ObstaclesSegments);
				
				sphForces[i].density = density + densityFromObstacles;
				sphForces[i].pressure = SphHelper::ComputePressure(sphForces[i].density, sphParameters[i].comfortDensity, settings.k);
			}
		});
}
