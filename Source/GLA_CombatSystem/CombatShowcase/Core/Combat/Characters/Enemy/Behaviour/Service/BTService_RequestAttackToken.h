// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BTService.h"
#include "BTService_RequestAttackToken.generated.h"

enum class EAttackCategory : uint8;

/**
 * Asks the combat director for an attack token on a timer and publishes the answer to the Blackboard.
 * Put it on the combat branch's root composite, so it runs for as long as the enemy is in combat.
 * Leaving the branch gives the token back.
 *
 * Writes:
 *   HasTokenKey        (Bool)   whether the enemy holds a token
 *   SideSignKey        (Float)  -1 left slot, +1 right slot, 0 ranged or none
 *   AttackLocationKey  (Vector) the melee slot beside the target. Left untouched for ranged tokens.
 */
UCLASS()
class GLA_COMBATSYSTEM_API UBTService_RequestAttackToken : public UBTService
{
	GENERATED_BODY()
public:
	UBTService_RequestAttackToken();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// Melee or ranged is decided by the carried weapon's tag. Override to use another rule.
	virtual EAttackCategory ResolveCategory(const APawn* Pawn) const;

	UPROPERTY(EditAnywhere, Category = "Blackboard") FBlackboardKeySelector TargetKey;
	UPROPERTY(EditAnywhere, Category = "Blackboard") FBlackboardKeySelector HasTokenKey;
	UPROPERTY(EditAnywhere, Category = "Blackboard") FBlackboardKeySelector SideSignKey;
	UPROPERTY(EditAnywhere, Category = "Blackboard") FBlackboardKeySelector AttackLocationKey;
	// Optional. The enemy's preferred shooting band from its behaviour row, for an EQS query to read.
	UPROPERTY(EditAnywhere, Category = "Blackboard") FBlackboardKeySelector RangeMinKey;
	UPROPERTY(EditAnywhere, Category = "Blackboard") FBlackboardKeySelector RangeMaxKey;

	// How far from the target the melee enemy stands, along the lane. Move this to the weapon asset when reach differs per weapon.
	UPROPERTY(EditAnywhere, Category = "Melee", meta = (ClampMin = "0.0")) float MeleeStandOffDistance = 130.f;
};
