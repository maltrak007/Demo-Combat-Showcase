// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatShowcase/Core/GAS/CombatGameplayAbility.h"

#include "GA_Evade.generated.h"

class ABaseCombatCharacter;
/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UGA_Evade : public UCombatGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Evade();
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Evade") float MinimumStaminaToActivate = 20.f;
	UPROPERTY(EditDefaultsOnly, Category = "Evade") float EvadeStaminaCost = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Evade|Roll") UAnimMontage* RollMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Evade|Roll") float RollDistance = 400.f;
	UPROPERTY(EditDefaultsOnly, Category = "Evade|Roll") bool bRollPassesThroughPawns = true;

	// Renamed from SideStepDistance. Re-enter the value on your Blueprint.
	UPROPERTY(EditDefaultsOnly, Category = "Evade|BackStep") UAnimMontage* BackStepMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Evade|BackStep") float BackStepDistance = 250.f;

	// Must match the Warp Target Name on the Motion Warping notify in both montages.
	UPROPERTY(EditDefaultsOnly, Category = "Evade|Warp") FName EvadeWarpTargetName = "Evade";
	UPROPERTY(EditDefaultsOnly, Category = "Evade|Warp") float WallMargin = 2.f;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION() void HandleMontageCompleted();
	UFUNCTION() void HandleMontageInterrupted();

private:
	float ComputeClearDistance(ABaseCombatCharacter* Avatar, float Direction, float DesiredDistance, bool bIgnorePawns) const;

	bool bPawnCollisionOverridden = false;
	TEnumAsByte<ECollisionResponse> SavedPawnResponse = ECR_Block;
};
