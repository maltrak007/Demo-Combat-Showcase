// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatShowcase/Core/Combat/Data/CombatData.h"
#include "CombatShowcase/Core/GAS/CombatGameplayAbility.h"
#include "GA_CounterAttack.generated.h"

/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UGA_CounterAttack : public UCombatGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_CounterAttack();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Counter") UDataTable* CounterstrikeDataTable = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "Counter") TSubclassOf<UGameplayEffect> DamageEffectClass;
	UPROPERTY(EditDefaultsOnly, Category = "Counter") TSubclassOf<UGameplayEffect> StaminaDamageEffectClass;
	
	UFUNCTION() void HandleHitDetected(AActor* HitActor,EHitOutcome Outcome);
	UFUNCTION() void HandleMontageCompleted();
	UFUNCTION() void HandleMontageInterrupted();
};
