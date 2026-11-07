// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatShowcase/Core/GAS/CombatGameplayAbility.h"
#include "GA_Aim.generated.h"

/**
 * Held while the player aims a drawn ranged weapon. It plants the character (movement is locked) and marks the
 * character as aiming; the input then moves the aim up and down within the weapon's MaxAimAngle. A light attack
 * pressed meanwhile is a single precise shot (GA_RangedAttack), with no combo.
 *
 * Starts on Event.Ability.Aim and ends when the input is released (the player cancels it by its Aim ability tag),
 * when the weapon is sheathed, or when anything that cancels attacks cancels it.
 */
UCLASS()
class GLA_COMBATSYSTEM_API UGA_Aim : public UCombatGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Aim();

	// Only with a ranged weapon in hand.
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                                const FGameplayTagContainer* SourceTags = nullptr,
	                                const FGameplayTagContainer* TargetTags = nullptr,
	                                FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;

	// Sheathing the weapon ends the aim.
	UFUNCTION() void HandleDrawnChanged(bool bDrawn);
};
