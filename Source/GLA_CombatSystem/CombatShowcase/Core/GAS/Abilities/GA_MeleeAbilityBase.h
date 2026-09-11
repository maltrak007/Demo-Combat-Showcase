// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatShowcase/Core/GAS/CombatGameplayAbility.h"
#include "GA_MeleeAbilityBase.generated.h"

/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UGA_MeleeAbilityBase : public UCombatGameplayAbility
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Strike") UDataTable* StrikeDataTable = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "Strike") TArray<FName> ComboRowNames;
	UPROPERTY(EditDefaultsOnly, Category = "Strike") TSubclassOf<UGameplayEffect> DamageEffectClass;

	int32 CurrentComboIndex = 0;

	UFUNCTION() void HandleHitDetected(AActor* HitActor);
	UFUNCTION() void HandleMontageCompleted();
	UFUNCTION() void HandleMontageInterrupted();
};
