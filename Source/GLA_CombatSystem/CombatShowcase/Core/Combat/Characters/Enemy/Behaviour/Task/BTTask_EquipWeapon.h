// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BTTaskNode.h"
#include "BTTask_EquipWeapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponGoal : uint8
{
	Draw,
	Sheathe
};

/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UBTTask_EquipWeapon : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_EquipWeapon();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Equip") EWeaponGoal Goal = EWeaponGoal::Draw;
	// Stops any MoveTo in progress first. The movement lock tag doesn't apply to AI movement.
	UPROPERTY(EditAnywhere, Category = "Equip") bool bStopMovement = true;
	// How long to wait for the ability to start before giving up.
	UPROPERTY(EditAnywhere, Category = "Equip", meta = (ClampMin = "0.05")) float StartTimeout = 0.5f;

private:
	// Per-enemy state. Safe because the node is instanced (bCreateNodeInstance).
	bool bAbilityStarted = false;
	double RequestedAt = 0.0;
};
