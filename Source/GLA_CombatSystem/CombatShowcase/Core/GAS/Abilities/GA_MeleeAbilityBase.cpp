// Fill out your copyright notice in the Description page of Project Settings.

#include "GA_MeleeAbilityBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CombatShowcase/Core/Combat/Characters/Player/PlayerCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Components/CombatStatsComponent.h"
#include "CombatShowcase/Core/Combat/Data/CombatData.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "CombatShowcase/Core/GAS/AbilityTasks/AbilityTask_MultiBoneHitTrace.h"

UGA_MeleeAbilityBase::UGA_MeleeAbilityBase()
{
	AbilityTags.AddTag(CombatTags::Ability_Type_Attack);
	ActivationOwnedTags.AddTag(CombatTags::Ability_Type_Attack);
	ActivationOwnedTags.AddTag(CombatTags::State_Combat_MovementLocked);

	FAbilityTriggerData LightTrigger;
	LightTrigger.TriggerTag = CombatTags::Event_Ability_LightAttack;
	LightTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(LightTrigger);

	FAbilityTriggerData HeavyTrigger;
	HeavyTrigger.TriggerTag = CombatTags::Event_Ability_HeavyAttack;
	HeavyTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(HeavyTrigger);
}

void UGA_MeleeAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!HasValidAnimInstance(ActorInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	APlayerCombatCharacter* Avatar = Cast<APlayerCombatCharacter>(GetAvatarActorFromActorInfo());
	UCombatStatsComponent* Stats = Avatar ? Avatar->GetCombatStatsComponent() : nullptr;

	FName RowToPlay;
	
	if (Stats && !Stats->GetCurrentRow().IsNone())
	{
		RowToPlay = Stats->GetCurrentRow(); 
	}
	else
	{
		const bool bHeavyPress = TriggerEventData && TriggerEventData->EventTag == CombatTags::Event_Ability_HeavyAttack;
		RowToPlay = bHeavyPress ? HeavyOpenerRow : LightOpenerRow;
	}

	if (!StrikeDataTable || RowToPlay.IsNone())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FCombatStrikeRow* Row = StrikeDataTable->FindRow<FCombatStrikeRow>(RowToPlay, TEXT("GA_MeleeAbilityBase"));

	if (!Row || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (Stats)
	{
		Stats->CacheContinuations(Row->NextRowOnLight, Row->NextRowOnHeavy);
	}
	
	if (Avatar)
	{
		Avatar->ConsumeStamina(Row->StaminaCost);
	}
	
	const float PlayRate = (Avatar && Avatar->GetStaminaPercent() < 0.15f) ? 0.85f : Row->Montage->RateScale;
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Row->Montage,PlayRate);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_MeleeAbilityBase::HandleMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_MeleeAbilityBase::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_MeleeAbilityBase::HandleMontageInterrupted);
	MontageTask->ReadyForActivation();
	
	FCombatStrikePayload Payload;
	Payload.Damage = Row->Damage;
	Payload.StaminaDamage = Row->StaminaDamage;
	Payload.Direction = Row->Direction;
	Payload.ReactionDirection = Row->ReactionDirection;
	Payload.ImpactFeel = Row->ImpactFeel;
	Payload.HPEffectClass = DamageEffectClass;
	Payload.STAEffectClass = StaminaDamageEffectClass;
	
	UAbilityTask_MultiBoneHitTrace* HitboxTask = UAbilityTask_MultiBoneHitTrace::CreateMultiBoneHitTrace(this, Row->Hitboxes, Payload);
	HitboxTask->OnMultiHitDetected.AddDynamic(this, &UGA_MeleeAbilityBase::HandleHitDetected);
	HitboxTask->ReadyForActivation();
}

void UGA_MeleeAbilityBase::HandleHitDetected(AActor* HitActor, EHitOutcome Outcome)
{
	if (Outcome != EHitOutcome::Unblocked) return;
	// Reserved for ability-level reactions later (combo counters, etc.).
	if (APlayerCombatCharacter* AttackerChar = Cast<APlayerCombatCharacter>(GetAvatarActorFromActorInfo()))
	{
		AttackerChar->GetCombatStatsComponent()->RegisterHitLanded();
	}
}

void UGA_MeleeAbilityBase::HandleMontageCompleted()
{
	if (APlayerCombatCharacter* PlayerChar = Cast<APlayerCombatCharacter>(GetAvatarActorFromActorInfo()))
	{
		PlayerChar->GetCombatStatsComponent()->ResetCombo();
	}
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}


void UGA_MeleeAbilityBase::HandleMontageInterrupted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}
