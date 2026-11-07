// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BTTaskNode.h"
#include "BTTask_FindLanePoint.generated.h"

class UBehaviorTree;
/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UBTTask_FindLanePoint : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_FindLanePoint();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	// Center of the area. If unset, the pawn's current location is used.
	UPROPERTY(EditAnywhere, Category = "Blackboard") FBlackboardKeySelector OriginKey;
	UPROPERTY(EditAnywhere, Category = "Blackboard") FBlackboardKeySelector ResultKey;

	// Half-width of the area along the lane (X). Move this to the archetype row once it exists.
	UPROPERTY(EditAnywhere, Category = "Area", meta = (ClampMin = "50.0")) float HalfWidth = 600.f;
	// Rejects tiny moves, so the enemy doesn't shuffle in place.
	UPROPERTY(EditAnywhere, Category = "Area", meta = (ClampMin = "0.0")) float MinDistanceFromCurrent = 150.f;
	UPROPERTY(EditAnywhere, Category = "Area", meta = (ClampMin = "1")) int32 MaxAttempts = 6;
	// Tolerance when snapping the point onto the navmesh: X/Y horizontal, Z vertical.
	UPROPERTY(EditAnywhere, Category = "Area") FVector ProjectionExtent = FVector(50.f, 50.f, 250.f);
};
