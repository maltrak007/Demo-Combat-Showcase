// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BTTaskNode.h"
#include "BTTask_ReleaseAttackToken.generated.h"

/**
 * Hands the attack token back to the director. Always succeeds.
 * Place it at the end of the attack sequence, and again as a fallback sibling, so a failed move or a
 * cancelled strike still gives the slot back.
 */
UCLASS()
class GLA_COMBATSYSTEM_API UBTTask_ReleaseAttackToken : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_ReleaseAttackToken();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	// Keeps the same enemy from asking again right away.
	UPROPERTY(EditAnywhere, Category = "Token") bool bApplyCooldown = true;
	// Use the enemy's own cooldown from its behaviour row (the sequence cooldown for a melee token, the burst cooldown
	// for a ranged one). Off, or with no behaviour row, the director's default cooldown applies.
	UPROPERTY(EditAnywhere, Category = "Token", meta = (EditCondition = "bApplyCooldown")) bool bUseBehaviourCooldown = true;
};
