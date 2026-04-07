// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassEntityQuery.h"
#include "CrowdObservationProcessor.generated.h"


UCLASS()
class CROWDMOVEMENT_JB_GB_API UCrowdObservationProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()
	
public:
	UCrowdObservationProcessor();

protected:

	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& Manager, FMassExecutionContext& Contexte) override;

private:
	FMassEntityQuery EntityQuery;


};
