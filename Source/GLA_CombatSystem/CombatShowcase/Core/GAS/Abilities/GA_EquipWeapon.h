// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatShowcase/Core/GAS/CombatGameplayAbility.h"
#include "GA_EquipWeapon.generated.h"

/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UGA_EquipWeapon : public UCombatGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_EquipWeapon();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION() void HandleCommitEvent(FGameplayEventData Payload);
	UFUNCTION() void HandleMontageCompleted();
	UFUNCTION() void HandleMontageInterrupted();

private:
	void Commit();
	bool bDrawing = false;
	bool bCommitted = false;

};
