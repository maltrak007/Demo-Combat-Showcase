// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BTTaskNode.h"
#include "BTTask_ExecuteCombo.generated.h"

struct FAbilityEndedData;
class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UBTTask_ExecuteCombo : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_ExecuteCombo();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual FString GetStaticDescription() const override;

protected:
	// CHANGE THIS TO SUPPORT MAXHITS PER ENEMY TYPE. For now, just a single value for all enemies.
	UPROPERTY(EditAnywhere, Category = "Combo", meta = (ClampMin = "1")) int32 MaxHits = 2;
	// Rows a combo can start from. Leave the sprint attacks out: they aren't chain openers.
	UPROPERTY(EditAnywhere, Category = "Combo") TArray<FName> OpenerRows;
	// How long to wait for a requested strike to actually begin before giving up.
	UPROPERTY(EditAnywhere, Category = "Combo", meta = (ClampMin = "0.05")) float StartTimeout = 0.5f;

private:
	TArray<FName> BuildPlan(const UDataTable& Table, int32 HitLimit) const;
	bool StartStrike(UBehaviorTreeComponent& OwnerComp);
	void HandleAbilityEnded(const FAbilityEndedData& Data);

	// Per-enemy state. Safe because the node is instanced (bCreateNodeInstance).
	TArray<FName> Plan;
	int32 Step = 0;
	bool bStrikeStarted = false;
	bool bStrikeEnded = false;
	bool bStrikeCancelled = false;
	double StrikeRequestedAt = 0.0;

	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
	FDelegateHandle AbilityEndedHandle;
};
