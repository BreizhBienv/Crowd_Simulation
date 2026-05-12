// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "CrowdThreatProcessor.generated.h"

/**
 * 
 */
UCLASS()
class CROWDMOVEMENT_JB_GB_API UCrowdThreatProcessor : public UMassProcessor
{
	GENERATED_BODY()
public:
	UCrowdThreatProcessor();
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& Manager, FMassExecutionContext& Context) override;
 
private:
	FMassEntityQuery EntityQuery;
	
};
