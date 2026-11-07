// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BTTaskNode.h"
#include "BTTask_LookAtTarget.generated.h"

/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UBTTask_LookAtTarget : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_LookAtTarget();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	// An Actor key (TargetActor) or a Vector key (LastKnownPosition, MoveToLocation).
	UPROPERTY(EditAnywhere, Category = "Blackboard") FBlackboardKeySelector TargetKey;

	// Below this horizontal gap the enemy keeps its facing, so a target directly above or below doesn't make it flicker.
	UPROPERTY(EditAnywhere, Category = "Look", meta = (ClampMin = "0.0")) float MinDistanceX = 10.f;

	// The player can't turn mid-swing or mid-reaction, so enemies shouldn't either.
	UPROPERTY(EditAnywhere, Category = "Look") bool bRespectMovementLock = true;};
