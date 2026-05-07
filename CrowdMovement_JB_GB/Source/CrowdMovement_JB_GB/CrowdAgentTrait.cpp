// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdAgentTrait.h"
#include"MassEntityTemplateRegistry.h"
#include"CrowdFragments.h"
#include"CrowdTags.h"
#include"MassCommonFragments.h"

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
		RoleData.DutyDuration = FMath::FRandRange(90.f, 150.f);
		BuildContext.AddFragment_GetRef<FCrowdRoleFragment>() = RoleData;
	}
 
}
