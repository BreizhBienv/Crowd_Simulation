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
	BuildContext.AddFragment<FCrowdBoidsFragment>();

	BuildContext.AddConstSharedFragment(FSharedStruct::Make(SharedSettings));

	BuildContext.AddTag<FTagPassiveWalking>();

	FCrowdFragments PersonalityData;

	if (bRandomizePersonality)
	{
		const int32 RandType = FMath::RandRange(0, 3);
		//choose random between 4 personality 
		PersonalityData.Personality= static_cast<EPersonalityType>(RandType);

		PersonalityData.CourageScore = FMath::FRandRange(courageRange.X, courageRange.Y);
		PersonalityData.CuriosityScore = FMath::FRandRange(curiosityRange.X, curiosityRange.Y);
		PersonalityData.SocialAffinityScore = FMath::FRandRange(socialAffinityRange.X, socialAffinityRange.Y);

		PersonalityData.SpeedBase = FMath::FRandRange(speedRange.X, speedRange.Y);
	}
	else
	{
		PersonalityData.Personality = DefaultPersonality;

		PersonalityData.CourageScore = (courageRange.X + courageRange.Y) * 0.5;
		PersonalityData.CuriosityScore = (curiosityRange.X + curiosityRange.Y) * 0.5;
		PersonalityData.SocialAffinityScore =(socialAffinityRange.X + socialAffinityRange.Y) * 0.5;

		PersonalityData.SpeedBase = (speedRange.X + speedRange.Y) * 0.5 ;
	}

	BuildContext.AddFragment_GetRef<FCrowdFragments>() = PersonalityData;
}
