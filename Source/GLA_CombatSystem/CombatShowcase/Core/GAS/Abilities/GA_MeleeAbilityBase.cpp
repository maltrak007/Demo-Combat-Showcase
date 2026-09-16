// Fill out your copyright notice in the Description page of Project Settings.

#include "GA_MeleeAbilityBase.h"

#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "CombatShowcase/Core/Combat/Data/CombatData.h"
#include "CombatShowcase/Core/GAS/AbilityTasks/AbilityTask_MultiBoneHitTrace.h"


void UGA_MeleeAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!HasValidAnimInstance(ActorInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!StrikeDataTable || !ComboRowNames.IsValidIndex(CurrentComboIndex))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FCombatStrikeRow* Row = StrikeDataTable->FindRow<FCombatStrikeRow>(ComboRowNames[CurrentComboIndex], TEXT("GA_MeleeAbilityBase"));
	
	if (!Row || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Row->Montage);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_MeleeAbilityBase::HandleMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_MeleeAbilityBase::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_MeleeAbilityBase::HandleMontageInterrupted);
	MontageTask->ReadyForActivation();

	UAbilityTask_MultiBoneHitTrace* HitboxTask = UAbilityTask_MultiBoneHitTrace::CreateMultiBoneHitTrace(this, Row->Hitboxes, DamageEffectClass, Row->Damage, Row->ImpactFeel);
	HitboxTask->OnMultiHitDetected.AddDynamic(this, &UGA_MeleeAbilityBase::HandleHitDetected);
	HitboxTask->ReadyForActivation();

	CurrentComboIndex = 0; // placeholder — Phase 3/4 territory
}

void UGA_MeleeAbilityBase::HandleHitDetected(AActor* HitActor)
{
	// Reserved for ability-level reactions later (combo counters, etc.).
	// Hitstop/knockback/camera shake are already fully handled inside the
	// AbilityTask via FeedbackComponent — nothing belongs here for those.
}

void UGA_MeleeAbilityBase::HandleMontageCompleted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGA_MeleeAbilityBase::HandleMontageInterrupted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}