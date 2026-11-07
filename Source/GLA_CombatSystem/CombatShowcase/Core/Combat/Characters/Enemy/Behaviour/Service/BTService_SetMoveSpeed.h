// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BTService.h"
#include "BTService_SetMoveSpeed.generated.h"

/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UBTService_SetMoveSpeed : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SetMoveSpeed();
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// Moves to the archetype row once it exists.
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = "1.0")) float MoveSpeed = 300.f;
};
