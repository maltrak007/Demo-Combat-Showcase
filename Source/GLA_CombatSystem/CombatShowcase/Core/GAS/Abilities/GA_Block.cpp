// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Block.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CombatShowcase/Core/Combat/Characters/Player/PlayerCombatCharacter.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"

UGA_Block::UGA_Block()
{
	AbilityTags.AddTag(CombatTags::Ability_Type_Block);
	ActivationOwnedTags.AddTag(CombatTags::Ability_Type_Block);

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = CombatTags::Event_Ability_Block;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UGA_Block::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo,
                                const FGameplayEventData* TriggerEventData)
{
	if (!HasValidAnimInstance(ActorInfo) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APlayerCombatCharacter* PlayerChar = Cast<APlayerCombatCharacter>(GetAvatarActorFromActorInfo());
	if (!PlayerChar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// One-time forced read at activation — establishes the initial direction regardless
	// of whether it "changed" from anything, since nothing's set yet to compare against.
	PlayGuardMontage(PlayerChar->GetLastRawBlockAxisValue() >= 0.f);

	UAbilityTask_WaitGameplayEvent* DirectionTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, CombatTags::Event_Combat_BlockDirectionChanged, nullptr, /*OnlyTriggerOnce=*/false);
	DirectionTask->EventReceived.AddDynamic(this, &UGA_Block::HandleDirectionChanged);
	DirectionTask->ReadyForActivation();
}

void UGA_Block::HandleDirectionChanged(FGameplayEventData Payload)
{
	PlayGuardMontage(Payload.EventMagnitude >= 0.f);
}

void UGA_Block::PlayGuardMontage(bool bUp)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	if (bUp)
	{
		ASC->AddLooseGameplayTag(CombatTags::State_Blocking_DirectionUp);
		ASC->RemoveLooseGameplayTag(CombatTags::State_Blocking_DirectionDown);
	}
	else
	{
		ASC->AddLooseGameplayTag(CombatTags::State_Blocking_DirectionDown);
		ASC->RemoveLooseGameplayTag(CombatTags::State_Blocking_DirectionUp);
	}

	UAnimMontage* MontageToPlay = bUp ? BlockToUpMontage : BlockToDownMontage;
	UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, MontageToPlay);
	Task->ReadyForActivation();
	// fire-and-forget — the held pose afterward is driven by the direction tag, not by waiting on this task
}

void UGA_Block::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                           const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                           bool bWasCancelled)
{
	// Cleanup runs regardless of how this ends — StopBlock's CancelAbilities, an interrupt, anything —
	// since EndAbility is the one common terminal point.
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(CombatTags::State_Blocking_DirectionUp);
		ASC->RemoveLooseGameplayTag(CombatTags::State_Blocking_DirectionDown);
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
