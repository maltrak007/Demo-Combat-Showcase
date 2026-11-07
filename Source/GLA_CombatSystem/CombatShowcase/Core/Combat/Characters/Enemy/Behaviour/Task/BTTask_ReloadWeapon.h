// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BTTaskNode.h"
#include "BTTask_ReloadWeapon.generated.h"

/**
 * Reloads the drawn ranged weapon. Succeeds at once when the magazine is full or the weapon isn't ranged.
 * Put it in the branch that runs without a token: the reload is the opening the player gets, and the enemy
 * isn't holding an attack slot while it happens. A slower mastery reloads with a slower montage.
 */
UCLASS()
class GLA_COMBATSYSTEM_API UBTTask_ReloadWeapon : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_ReloadWeapon();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	// Stops any MoveTo in progress first.
	UPROPERTY(EditAnywhere, Category = "Reload") bool bStopMovement = true;
	// How long to wait for the reload to start before giving up.
	UPROPERTY(EditAnywhere, Category = "Reload", meta = (ClampMin = "0.05")) float StartTimeout = 0.5f;

private:
	// Per-enemy state. Safe because the node is instanced (bCreateNodeInstance).
	bool bStarted = false;
	double RequestedAt = 0.0;
};
