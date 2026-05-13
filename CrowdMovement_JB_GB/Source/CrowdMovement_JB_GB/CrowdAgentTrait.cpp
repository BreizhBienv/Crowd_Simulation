// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdAgentTrait.h"
#include"MassEntityTemplateRegistry.h"
#include"CrowdFragments.h"
#include"CrowdTags.h"

void UCrowdAgentTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FCrowdFragments>();
	BuildContext.AddFragment<FCrowdPhaseFragments>();
	BuildContext.AddFragment<FCrowdPOIFragment>();
	BuildContext.AddFragment<FCrowdRoleFragment>();
	BuildContext.AddFragment<FCrowdSocialGroupFragment>();
 
	BuildContext.AddConstSharedFragment(FSharedStruct::Make(SharedSettings));
 

	BuildContext.AddTag<FTagPassiveWalking>();
 

	BuildContext.AddTag<FTagNeedsInit>();
	
	{
		FCrowdFragments DefaultData;
		DefaultData.Personality         = DefaultPersonality;
		DefaultData.CourageScore        = (courageRange.X + courageRange.Y) * 0.5f;
		DefaultData.CuriosityScore      = (curiosityRange.X + curiosityRange.Y) * 0.5f;
		DefaultData.SocialAffinityScore = (socialAffinityRange.X + socialAffinityRange.Y) * 0.5f;
		DefaultData.SpeedBase           = (speedRange.X + speedRange.Y) * 0.5f;
		BuildContext.AddFragment_GetRef<FCrowdFragments>() = DefaultData;
	}
	
	{
		FCrowdRoleFragment RoleData;
		RoleData.Role        = DefaultRole;
		RoleData.bOnDuty     = true;
		RoleData.DutyTimer   = 0.f;
		RoleData.DutyDuration = 120.f;
		BuildContext.AddFragment_GetRef<FCrowdRoleFragment>() = RoleData;
	}
	
	BuildContext.AddFragment<FSphFragment>();
	BuildContext.AddFragment<FSphForcesFragment>();
	
	{		
		FSharedSphCrowdSettings localSphSettings = SphCrowdSettings;
		localSphSettings.ObstaclesSegments = World.GetSubsystem<USphObstacleSubSystem>()->GetObstacleSegments();
		
		BuildContext.AddSharedFragment(FSharedStruct::Make(localSphSettings));
	}
	
	//Sph	
	if (const FSphSettingsFragment* settings = BuildContext.GetFragment<FSphSettingsFragment>())
	{	
		if (settings->randomizeSphValues)
		{
			FSphFragment sphData;
			sphData.h = FMath::RandRange(settings->hRange.X, settings->hRange.Y);
			sphData.mass = FMath::RandRange(settings->massRange.X, settings->massRange.Y);
			sphData.comfortDensity = FMath::RandRange(settings->comfortRange.X, settings->comfortRange.Y);
			sphData.obstacleAreaCoef = FMath::RandRange(settings->coefRange.X, settings->coefRange.Y);
			BuildContext.AddFragment_GetRef<FSphFragment>() = sphData;
		}
	}
	
	{	
		BuildContext.AddSharedFragment(FSharedStruct::Make(LocationToTarget));
	}
}
