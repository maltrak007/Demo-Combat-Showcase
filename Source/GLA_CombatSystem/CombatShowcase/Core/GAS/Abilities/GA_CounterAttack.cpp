// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_CounterAttack.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "CombatShowcase/Core/Combat/Characters/Player/PlayerCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Components/CombatStatsComponent.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "CombatShowcase/Core/GAS/AbilityTasks/AbilityTask_MultiBoneHitTrace.h"

UGA_CounterAttack::UGA_CounterAttack()
{
	AbilityTags.AddTag(CombatTags::Ability_Type_Attack);
	ActivationOwnedTags.AddTag(CombatTags::Ability_Type_Attack);
	ActivationOwnedTags.AddTag(CombatTags::State_Combat_MovementLocked);
	
	FAbilityTriggerData CounterstrikeTrigger;
	CounterstrikeTrigger.TriggerTag = CombatTags::Event_Ability_Counter;
	CounterstrikeTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(CounterstrikeTrigger);
}

void UGA_CounterAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!HasValidAnimInstance(ActorInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	const TArray<FName> RowNames = CounterstrikeDataTable->GetRowNames();
	if (RowNames.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: counter table is empty."), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	const FName RowToPlay = RowNames[FMath::RandRange(0, RowNames.Num() - 1)];
	
	if (!CounterstrikeDataTable)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	const FCombatStrikeRow* Row = CounterstrikeDataTable->FindRow<FCombatStrikeRow>(RowToPlay, TEXT("GA_CounterAttack"));
	if (!Row || !Row->Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: row '%s' is missing or has no montage."), *GetName(), *RowToPlay.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (APlayerCombatCharacter* Player = Cast<APlayerCombatCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UCombatStatsComponent* Stats = Player->GetCombatStatsComponent())
		{
			Stats->ResetCombo();
			Stats->CacheContinuations(NAME_None, NAME_None);
			Stats->ConsumeComboRequest();
		}
	}
	
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Row->Montage);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_CounterAttack::HandleMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_CounterAttack::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_CounterAttack::HandleMontageInterrupted);
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
	HitboxTask->OnMultiHitDetected.AddDynamic(this, &UGA_CounterAttack::HandleHitDetected);
	HitboxTask->ReadyForActivation();
}

void UGA_CounterAttack::HandleHitDetected(AActor* HitActor, EHitOutcome Outcome)
{
	if (Outcome != EHitOutcome::Unblocked) return;
	
	if (APlayerCombatCharacter* AttackerChar = Cast<APlayerCombatCharacter>(GetAvatarActorFromActorInfo()))
	{
		AttackerChar->GetCombatStatsComponent()->RegisterHitLanded();
	}
}

void UGA_CounterAttack::HandleMontageCompleted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGA_CounterAttack::HandleMontageInterrupted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}
