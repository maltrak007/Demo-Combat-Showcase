// Fill out your copyright notice in the Description page of Project Settings.

#include "GA_MeleeAbilityBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CombatShowcase/Core/Combat/Characters/Player/PlayerCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Components/CombatComboComponent.h"
#include "CombatShowcase/Core/Combat/Components/CombatFeedbackComponent.h"
#include "CombatShowcase/Core/Combat/Components/WeaponComponent.h"
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

bool UGA_MeleeAbilityBase::IsWeaponKindAllowed(const UWeaponComponent* Weapons) const
{
	return !(Weapons && Weapons->IsRangedDrawn());
}

FName UGA_MeleeAbilityBase::GetOpenerRow(bool bHeavy) const
{
	return bHeavy ? HeavyOpenerRow : LightOpenerRow;
}

bool UGA_MeleeAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                              FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;

	const ABaseCombatCharacter* Avatar = Cast<ABaseCombatCharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
	return IsWeaponKindAllowed(Avatar ? Avatar->GetWeaponComponent() : nullptr);
}

FCombatStrikePayload UGA_MeleeAbilityBase::BuildPayload(const FCombatStrikeRow& Row, const UCombatComboComponent* Stats) const
{
	FCombatStrikePayload Payload;
	Payload.Damage = Row.Damage;
	Payload.StaminaDamage = Row.StaminaDamage;
	Payload.Direction = Row.Direction;
	Payload.ReactionDirection = Row.ReactionDirection;
	Payload.ImpactFeel = Row.ImpactFeel;
	Payload.HPEffectClass = Stats ? Stats->GetDamageEffectClass() : nullptr;
	Payload.STAEffectClass = Stats ? Stats->GetStaminaDamageEffectClass() : nullptr;
	Payload.StrikeTag = Row.StrikeTag;
	return Payload;
}

void UGA_MeleeAbilityBase::PlayStrikeMontage(UAnimMontage* Montage, float PlayRate)
{
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, PlayRate);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_MeleeAbilityBase::HandleMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_MeleeAbilityBase::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_MeleeAbilityBase::HandleMontageInterrupted);
	MontageTask->ReadyForActivation();
}

void UGA_MeleeAbilityBase::BeginStrike(const FCombatStrikeRow& Row, const FCombatStrikePayload& Payload, float PlayRate,
                                       const FGameplayEventData* TriggerEventData)
{
	PlayStrikeMontage(Row.Montage, PlayRate);

	UAbilityTask_MultiBoneHitTrace* HitboxTask = UAbilityTask_MultiBoneHitTrace::CreateMultiBoneHitTrace(this, Row.Hitboxes, Payload);
	HitboxTask->OnMultiHitDetected.AddDynamic(this, &UGA_MeleeAbilityBase::HandleHitDetected);
	HitboxTask->OnWindowClosed.AddDynamic(this, &UGA_MeleeAbilityBase::HandleHitWindowClosed);
	HitboxTask->ReadyForActivation();
}

void UGA_MeleeAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!HasValidAnimInstance(ActorInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	ABaseCombatCharacter* Avatar = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo());
	UCombatComboComponent* Stats = Avatar ? Avatar->GetCombatComboComponent() : nullptr;

	FName RowToPlay;

	if (Stats && !Stats->GetCurrentRow().IsNone())
	{
		RowToPlay = Stats->GetCurrentRow();
	}
	else
	{
		const bool bHeavyPress = TriggerEventData && TriggerEventData->EventTag == CombatTags::Event_Ability_HeavyAttack;
		RowToPlay = GetOpenerRow(bHeavyPress);
	}

	const UDataTable* Table = Stats ? Stats->GetStrikeDataTable() : nullptr;
	if (!Table || RowToPlay.IsNone())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FCombatStrikeRow* Row = Table->FindRow<FCombatStrikeRow>(RowToPlay, TEXT("GA_MeleeAbilityBase"));

	if (!Row || !Row->Montage || !CommitAbility(Handle, ActorInfo, ActivationInfo))
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

	const float PlayRate = (Avatar && Avatar->GetStaminaPercent() < 0.15f) ? 0.85f : Row->MontagePlayRate;
	CurrentStrikeTag = Row->StrikeTag;
	BeginStrike(*Row, BuildPayload(*Row, Stats), PlayRate, TriggerEventData);
}

void UGA_MeleeAbilityBase::HandleHitDetected(AActor* HitActor, EHitOutcome Outcome)
{
	if (Outcome != EHitOutcome::Unblocked) return;

	if (APlayerCombatCharacter* AttackerChar = Cast<APlayerCombatCharacter>(GetAvatarActorFromActorInfo()))
	{
		AttackerChar->GetCombatComboComponent()->RegisterHitLanded();
	}
}

void UGA_MeleeAbilityBase::HandleMontageCompleted()
{
	if (ABaseCombatCharacter* Character = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->GetCombatComboComponent()->ResetCombo();
	}
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}


void UGA_MeleeAbilityBase::HandleMontageInterrupted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}

void UGA_MeleeAbilityBase::HandleHitWindowClosed(bool bHitAnything)
{
	if (bHitAnything) return;

	const ABaseCombatCharacter* Avatar = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo());
	UCombatFeedbackComponent* Feedback = Avatar ? Avatar->GetFeedbackComponent() : nullptr;
	if (!Feedback) return;

	const UMeshComponent* Weapon = Avatar->GetEquippedWeaponMesh();
	Feedback->PlayWhiff(CurrentStrikeTag, Weapon ? Weapon->GetComponentLocation() : Avatar->GetActorLocation());
}
