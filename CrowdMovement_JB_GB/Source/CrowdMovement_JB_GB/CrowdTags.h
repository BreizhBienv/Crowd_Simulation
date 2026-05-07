// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityTypes.h"
#include "CrowdTags.generated.h"

/**
 * 
 */
USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FTagPassiveWalking : public FMassTag
{
	GENERATED_BODY()
	
};

USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FTagPassiveObserving: public FMassTag
{
	GENERATED_BODY()

};

USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FTagPassiveDiscution : public FMassTag
{
	GENERATED_BODY()

};

USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FTagActiveAlert : public FMassTag
{
	GENERATED_BODY()

};


USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FTagActiveFleeing : public FMassTag
{
	GENERATED_BODY()

};

USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FTagActiveFighting : public FMassTag
{
	GENERATED_BODY()

};

USTRUCT()
struct CROWDMOVEMENT_JB_GB_API FTagNeedsInit : public FMassTag
{
	GENERATED_BODY()

};
