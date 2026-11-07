// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatShowcase/Core/Combat/Data/CombatData.h"
#include "CombatShowcase/Core/GAS/CombatGameplayAbility.h"
#include "GA_MeleeAbilityBase.generated.h"

class UWeaponComponent;
class UCombatComboComponent;
class UAnimMontage;

/**
 * A strike played from a row of the weapon's moveset table, with the combo chain around it.
 * GA_RangedAttack derives from it, so a ranged weapon gets the same rows, combos and continuation windows.
 * The parts that differ between a swing and a shot are the virtual functions below.
 */
UCLASS()
class GLA_COMBATSYSTEM_API UGA_MeleeAbilityBase : public UCombatGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_MeleeAbilityBase();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                                const FGameplayTagContainer* SourceTags = nullptr,
	                                const FGameplayTagContainer* TargetTags = nullptr,
	                                FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

	// Which weapons this ability serves. The melee abilities run while no ranged weapon is drawn, and
	// GA_RangedAttack overrides this to require one, so each attack event goes to exactly one of them.
	virtual bool IsWeaponKindAllowed(const UWeaponComponent* Weapons) const;

	// The row a chain starts from when no continuation row is pending.
	virtual FName GetOpenerRow(bool bHeavy) const;

	// Starts the strike itself, once the row is resolved and the ability is committed. Melee plays the montage and
	// opens the hitbox window. GA_RangedAttack waits for the fire notify and shoots a bullet instead.
	virtual void BeginStrike(const FCombatStrikeRow& Row, const FCombatStrikePayload& Payload, float PlayRate,
	                         const FGameplayEventData* TriggerEventData);

	FCombatStrikePayload BuildPayload(const FCombatStrikeRow& Row, const UCombatComboComponent* Stats) const;
	void PlayStrikeMontage(UAnimMontage* Montage, float PlayRate);

	UPROPERTY(EditDefaultsOnly, Category = "Strike")
	FName LightOpenerRow;
	UPROPERTY(EditDefaultsOnly, Category = "Strike")
	FName HeavyOpenerRow;

	UFUNCTION()
	void HandleHitDetected(AActor* HitActor, EHitOutcome Outcome);
	UFUNCTION()
	virtual void HandleMontageCompleted();
	UFUNCTION()
	void HandleMontageInterrupted();

private:
	UFUNCTION()
	void HandleHitWindowClosed(bool bHitAnything);

	FGameplayTag CurrentStrikeTag;
};
