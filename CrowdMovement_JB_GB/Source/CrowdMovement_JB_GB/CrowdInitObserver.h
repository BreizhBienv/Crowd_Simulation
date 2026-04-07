// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassEntityQuery.h"
#include "CrowdInitObserver.generated.h"

/**
 * 
 */
UCLASS()
class CROWDMOVEMENT_JB_GB_API UCrowdInitObserver :public UMassObserverProcessor
{
	GENERATED_BODY()
	
public:

	UCrowdInitObserver();

protected:

	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& Manager, FMassExecutionContext& Contexte) override;

private:
	FMassEntityQuery EntityQuery;

};
