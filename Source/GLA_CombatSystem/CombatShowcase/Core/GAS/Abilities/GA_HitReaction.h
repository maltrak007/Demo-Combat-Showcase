// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatShowcase/Core/GAS/CombatGameplayAbility.h"
#include "GA_HitReaction.generated.h"


enum class EHitReactionDirection : uint8;
/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UGA_HitReaction : public UCombatGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_HitReaction();
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Reaction|Unblocked") TMap<EHitReactionDirection, UAnimMontage*> UnblockedReactions;
	UPROPERTY(EditDefaultsOnly, Category = "Reaction|Blocked") TArray<UAnimMontage*> GuardHitReactions;
	UPROPERTY(EditDefaultsOnly, Category = "Reaction|Parried") UAnimMontage* ParryVictimReaction;
	UPROPERTY(EditDefaultsOnly, Category = "Reaction|Parried") UAnimMontage* ParrySuccessReaction;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION() 
	void HandleDone();
	
	UFUNCTION() 
	void HandleBlendOut();
	
	UFUNCTION() 
	void HandleParrySuccessCompleted();
	
private:
	TWeakObjectPtr<const AActor> ParriedAttacker;
};
