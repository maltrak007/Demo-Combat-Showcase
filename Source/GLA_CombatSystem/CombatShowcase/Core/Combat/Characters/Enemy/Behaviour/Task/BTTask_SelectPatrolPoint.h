// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BTTaskNode.h"
#include "BTTask_SelectPatrolPoint.generated.h"

/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UBTTask_SelectPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SelectPatrolPoint();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	// Vector: where to go next.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ResultKey;
	// Int: which point the enemy is heading to. This task is its only writer.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IndexKey;
	
	// Lane distance at which the enemy counts as having reached its current point.
	// Keep it larger than MoveTo's Acceptable Radius, or the route never advances.
	UPROPERTY(EditAnywhere, Category = "Patrol", meta = (ClampMin = "1.0"))
	float ArrivalRadius = 120.f;
	UPROPERTY(EditAnywhere, Category = "Patrol")
	FVector ProjectionExtent = FVector(50.f, 50.f, 250.f);
};
