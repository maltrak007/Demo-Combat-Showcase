// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BTTaskNode.h"
#include "BTTask_Evasion.generated.h"

UENUM(BlueprintType)
enum class EEvasionAction : uint8
{
	Roll,
	StepBack
};

/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UBTTask_Evasion : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Evasion();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Evade") EEvasionAction Action = EEvasionAction::StepBack;
};
