// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BTTaskNode.h"
#include "BTTask_ExecuteRangedAttack.generated.h"

struct FAbilityEndedData;
class UAbilitySystemComponent;
class ABaseCombatCharacter;

/**
 * Fires one burst with the drawn ranged weapon: the shots, and the telegraph before the first one, come from the
 * enemy's behaviour row. Each shot is one GA_RangedAttack. Fails at once when the magazine is empty, so keep a
 * Reload Weapon in the branch that runs without a token.
 */
UCLASS()
class GLA_COMBATSYSTEM_API UBTTask_ExecuteRangedAttack : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_ExecuteRangedAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual FString GetStaticDescription() const override;

protected:
	// The three below only apply to an enemy without a behaviour component.
	UPROPERTY(EditAnywhere, Category = "Fallback", meta = (ClampMin = "1")) int32 FallbackShots = 1;
	UPROPERTY(EditAnywhere, Category = "Fallback", meta = (ClampMin = "0.0")) float FallbackFireDelay = 0.8f;
	UPROPERTY(EditAnywhere, Category = "Fallback", meta = (ClampMin = "0.0")) float FallbackShotInterval = 0.35f;
	// How long to wait for a requested shot to actually begin before giving up.
	UPROPERTY(EditAnywhere, Category = "Ranged", meta = (ClampMin = "0.05")) float StartTimeout = 0.5f;

private:
	void RequestShot(ABaseCombatCharacter* Self, float TelegraphSeconds);
	void HandleAbilityEnded(const FAbilityEndedData& Data);

	// Per-enemy state. Safe because the node is instanced (bCreateNodeInstance).
	int32 ShotsPlanned = 0;
	int32 ShotsFired = 0;
	float ShotInterval = 0.f;
	double NextShotAt = 0.0;
	double RequestedAt = 0.0;
	bool bWaitingForInterval = false;
	bool bShotStarted = false;
	bool bShotEnded = false;
	bool bShotCancelled = false;

	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
	FDelegateHandle AbilityEndedHandle;
};
