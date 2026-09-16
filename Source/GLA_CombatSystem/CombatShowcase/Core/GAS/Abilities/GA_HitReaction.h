// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "CombatShowcase/Core/GAS/CombatGameplayAbility.h"
#include "GA_HitReaction.generated.h"


/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UGA_HitReaction : public UCombatGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_HitReaction() { FAbilityTriggerData T; T.TriggerTag = CombatTags::Event_Combat_HitReact; T.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent; AbilityTriggers.Add(T); }
protected:
	UPROPERTY(EditDefaultsOnly) 
	UAnimMontage* HitReactMontage;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override
	{
		if (!HasValidAnimInstance(ActorInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
		
		UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, HitReactMontage);
		Task->OnCompleted.AddDynamic(this, &UGA_HitReaction::HandleDone);
		Task->OnInterrupted.AddDynamic(this, &UGA_HitReaction::HandleDone);
		Task->OnCancelled.AddDynamic(this, &UGA_HitReaction::HandleDone);
		Task->ReadyForActivation();
	}
	
	UFUNCTION() 
	void HandleDone() { EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false); }
};
