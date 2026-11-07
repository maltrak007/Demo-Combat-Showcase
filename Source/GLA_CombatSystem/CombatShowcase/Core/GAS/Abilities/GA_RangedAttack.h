// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatShowcase/Core/Combat/Data/CombatData.h"
#include "CombatShowcase/Core/GAS/Abilities/GA_MeleeAbilityBase.h"
#include "GA_RangedAttack.generated.h"

class ABaseCombatCharacter;
struct FRangedWeaponSpec;

/**
 * A strike with the drawn ranged weapon. It is a melee ability whose hit is a bullet, so it plays rows from the
 * weapon's moveset table and chains them into combos the same way. It works for the player and for enemies.
 *
 * Triggers: the two attack events, inherited from the melee base, plus Event.Ability.Reload.
 *   Event.Ability.LightAttack / HeavyAttack   play the weapon's shot row, or the continuation row of a combo.
 *       With an empty magazine they reload instead. EventMagnitude is the telegraph in seconds: 0 fires at once,
 *       above 0 the shooter raises its aim first and the target's UI animates for that long.
 *   Event.Ability.Reload                      reloads. EventMagnitude is the montage play rate, 0 meaning normal speed.
 *
 * The bullet leaves on the montage's Event.Combat.Fire notify. While the character is aiming (GA_Aim) it flies
 * where the muzzle points, limited to the weapon's aim angle, and the shot never continues into a combo.
 */
UCLASS()
class GLA_COMBATSYSTEM_API UGA_RangedAttack : public UGA_MeleeAbilityBase
{
	GENERATED_BODY()
public:
	UGA_RangedAttack();

protected:
	virtual bool IsWeaponKindAllowed(const UWeaponComponent* Weapons) const override;
	virtual FName GetOpenerRow(bool bHeavy) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	virtual void BeginStrike(const FCombatStrikeRow& Row, const FCombatStrikePayload& Payload, float PlayRate,
	                         const FGameplayEventData* TriggerEventData) override;
	virtual void HandleMontageCompleted() override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;

	UFUNCTION() void HandleTelegraphDone();
	UFUNCTION() void HandleFireEvent(FGameplayEventData Payload);
	UFUNCTION() void HandleReloadCompleted();
	UFUNCTION() void HandleReloadInterrupted();

private:
	void PlayShotMontage();
	void StartReload(float PlayRate);
	void FireShot();
	void ClearTelegraph();

	// The direction of the bullet in the lane plane: along the facing, or where the muzzle points while aiming.
	FVector ResolveShotDirection(const ABaseCombatCharacter& Avatar, const FRangedWeaponSpec& Spec) const;

	// Copies of the row and payload for the shot in progress, taken when the strike begins.
	UPROPERTY(Transient)
	FCombatStrikeRow ShotRow;
	UPROPERTY(Transient)
	FCombatStrikePayload ShotPayload;

	float ShotPlayRate = 1.f;
	float PendingTelegraph = 0.f;
	bool bShotFired = false;
	bool bTelegraphing = false;
};
