// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "GoToLocationProcessor.generated.h"

/**
 * 
 */
UCLASS()
class CROWDMOVEMENT_JB_GB_API UGoToLocationProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
	FMassEntityQuery EntityQuery;
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& Manager, FMassExecutionContext& Context) override;
	
public:
	UGoToLocationProcessor();
};
