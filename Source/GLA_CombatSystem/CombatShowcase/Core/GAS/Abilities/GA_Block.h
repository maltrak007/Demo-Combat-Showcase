// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatShowcase/Core/GAS/CombatGameplayAbility.h"
#include "GA_Block.generated.h"

/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UGA_Block : public UCombatGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Block();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;

	UPROPERTY(EditDefaultsOnly, Category = "Block")
	UAnimMontage* BlockToUpMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Block")
	UAnimMontage* BlockToDownMontage;

	UFUNCTION()
	void HandleDirectionChanged(FGameplayEventData Payload);
	void PlayGuardMontage(bool bUp);
};
