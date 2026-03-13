// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SphAgentManager.generated.h"

class ASphAgent;

USTRUCT(Blueprintable)
struct FHashGridData
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	FIntVector2 cellIndex;
	
	UPROPERTY(VisibleAnywhere)
	TArray<ASphAgent*> Agents;
};

UCLASS()
class CROWDMOVEMENT_JB_GB_API ASphAgentManager : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	TArray<ASphAgent*> Agents;
	
	UPROPERTY(VisibleAnywhere, Category="Hash Grid")
	TMap<uint32, FHashGridData> HashGrid;
	
	UPROPERTY(EditAnywhere, Category="Hash Grid")
	FIntVector2 CellSize = FIntVector2(500, 500);
	
public:
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bDebugHashGrid = true;
	
	// Sets default values for this actor's properties
	ASphAgentManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void RegisterAgentsInHashGrid();
	void UpdateAgents();
	FVector FromCellToWorldCoord(FIntVector2 CellId) const;
	
	void DebugHashGrid();
};
