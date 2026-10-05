// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Block.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CombatShowcase/Core/Combat/Characters/Player/PlayerCombatCharacter.h"
#include "CombatShowcase/Core/GAS/CombatAttributeSet.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"

UGA_Block::UGA_Block()
{
	AbilityTags.AddTag(CombatTags::Ability_Type_Block);
	ActivationOwnedTags.AddTag(CombatTags::Ability_Type_Block);
	ActivationOwnedTags.AddTag(CombatTags::State_Combat_MovementLocked);
	
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
	ABaseCombatCharacter* BaseChar = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo());
	if (!BaseChar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (BaseChar->GetAttributeSet()->GetStamina() < BlockStaminaCost)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	//Player Active Block Direction
	if (APlayerCombatCharacter* PlayerChar = Cast<APlayerCombatCharacter>(BaseChar))
	{
		PlayGuardMontage(PlayerChar->GetLastRawBlockAxisValue() >= 0.f);
	}
	else
	{
		PlayGuardMontage(true);
	}
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	ASC->AddLooseGameplayTag(CombatTags::State_Stamina_RegenBlocked);
	bRegenBlockHeld = true;

	UAbilityTask_WaitGameplayEvent* DirectionTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, CombatTags::Event_Combat_BlockDirectionChanged, nullptr, /*OnlyTriggerOnce=*/false);
	DirectionTask->EventReceived.AddDynamic(this, &UGA_Block::HandleDirectionChanged);
	DirectionTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* ReactionTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, CombatTags::Event_Combat_ReactionFinished, nullptr, /*OnlyTriggerOnce=*/false);
	ReactionTask->EventReceived.AddDynamic(this, &UGA_Block::HandleReactionFinished);
	ReactionTask->ReadyForActivation();
}

void UGA_Block::HandleDirectionChanged(FGameplayEventData Payload)
{
	PlayGuardMontage(Payload.EventMagnitude >= 0.f);
}

void UGA_Block::HandleReactionFinished(FGameplayEventData Payload)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	// Resume the held pose in the same direction. Montage only: the direction tags are
	// reference-counted, so a resume must never touch them.
	StartGuardMontage(ASC->HasMatchingGameplayTag(CombatTags::State_Blocking_DirectionUp), GuardHoldSection);
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

	StartGuardMontage(bUp);
}

void UGA_Block::StartGuardMontage(bool bUp, FName StartSection)
{
	UAnimMontage* MontageToPlay = bUp ? BlockToUpMontage : BlockToDownMontage;
	if (!MontageToPlay) return;
	
	if (StartSection != NAME_None && MontageToPlay->GetSectionIndex(StartSection) == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: montage %s has no section '%s' — guard not resumed."),
			*GetName(), *MontageToPlay->GetName(), *StartSection.ToString());
		return;
	}

	UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, MontageToPlay, 1.f, StartSection);
	Task->ReadyForActivation();
}

void UGA_Block::HandleStaminaChanged(float NewStamina, float ChangeAmount, bool bIsDepleted)
{
	if (IsActive() && NewStamina < BlockStaminaCost)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
	}
}

void UGA_Block::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                           const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                           bool bWasCancelled)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(CombatTags::State_Blocking_DirectionUp);
		ASC->RemoveLooseGameplayTag(CombatTags::State_Blocking_DirectionDown);
		ASC->SetLooseGameplayTagCount(CombatTags::State_Combat_ParryWindowOpen, 0);
		if (bRegenBlockHeld)
		{
			ASC->RemoveLooseGameplayTag(CombatTags::State_Stamina_RegenBlocked);
			bRegenBlockHeld = false;
		}
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Block::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);
	if (ABaseCombatCharacter* BaseChar = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UCombatAttributeSet* Attrs = BaseChar->GetAttributeSet())
		{
			Attrs->OnStaminaChanged.AddDynamic(this, &UGA_Block::HandleStaminaChanged);
		}
	}
}
