// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BTDecorator.h"
#include "BTDecorator_HasAttackToken.generated.h"

UENUM()
enum class EAttackTokenFilter : uint8
{
	Any,
	MeleeOnly,
	RangedOnly
};

struct FBTHasAttackTokenMemory
{
	bool bLastValue = false;
};

/**
 * Passes while the enemy holds an attack token. Asks the director directly and re-checks on its own tick,
 * so the branch aborts as soon as the token is lost or granted. Use the filter to split melee and ranged branches.
 */
UCLASS()
class GLA_COMBATSYSTEM_API UBTDecorator_HasAttackToken : public UBTDecorator
{
	GENERATED_BODY()
public:
	UBTDecorator_HasAttackToken();

	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTHasAttackTokenMemory); }
	virtual FString GetStaticDescription() const override;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Token") EAttackTokenFilter Filter = EAttackTokenFilter::Any;
};
