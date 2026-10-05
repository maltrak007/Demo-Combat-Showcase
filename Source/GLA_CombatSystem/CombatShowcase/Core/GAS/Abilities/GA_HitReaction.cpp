// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_HitReaction.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "CombatShowcase/Core/Combat/Data/CombatData.h"

UGA_HitReaction::UGA_HitReaction()
{
	ActivationOwnedTags.AddTag(CombatTags::State_Combat_MovementLocked);
	auto AddTrigger = [this](const FGameplayTag& Tag)
	{
		FAbilityTriggerData T;
		T.TriggerTag = Tag;
		T.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(T);
	};
	AddTrigger(CombatTags::Event_Combat_HitReact_Unblocked);
	AddTrigger(CombatTags::Event_Combat_HitReact_Blocked);
	AddTrigger(CombatTags::Event_Combat_HitReact_Parried);
	AddTrigger(CombatTags::Event_Combat_HitReact_ParrySuccess);
	AddTrigger(CombatTags::Event_Combat_HitReact_GuardBreak);
}

void UGA_HitReaction::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!HasValidAnimInstance(ActorInfo) || !TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ParriedAttacker.Reset();
	const FGameplayTag& EventTag = TriggerEventData->EventTag;
	const bool bIsParrySuccess = EventTag == CombatTags::Event_Combat_HitReact_ParrySuccess;
	UAnimMontage* MontageToPlay = nullptr;

	if (EventTag == CombatTags::Event_Combat_HitReact_Unblocked)
	{
		const EHitReactionDirection Direction = static_cast<EHitReactionDirection>(FMath::RoundToInt(TriggerEventData->EventMagnitude));

		if (UAnimMontage* const* Found = UnblockedReactions.Find(Direction))
		{
			MontageToPlay = *Found;
		}
		if (!MontageToPlay)
		{
			if (UAnimMontage* const* Fallback = UnblockedReactions.Find(EHitReactionDirection::Generic))
			{
				MontageToPlay = *Fallback;
			}
		}
	}
	else if (EventTag == CombatTags::Event_Combat_HitReact_Blocked)
	{
		if (GuardHitReactions.Num() > 0) MontageToPlay = GuardHitReactions[FMath::RandRange(0, GuardHitReactions.Num() - 1)];
	}
	else if (EventTag == CombatTags::Event_Combat_HitReact_Parried || EventTag == CombatTags::Event_Combat_HitReact_GuardBreak)
	{
		MontageToPlay = ParryVictimReaction; // the guard-break stagger deliberately reuses this animation
	}
	else if (bIsParrySuccess)
	{
		MontageToPlay = ParrySuccessReaction;
		ParriedAttacker = TriggerEventData->Instigator; // the counter's target
	}

	if (!MontageToPlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: no reaction montage resolved for %s. Check the montages on the Blueprint."),
			*GetName(), *EventTag.ToString());
		if (bIsParrySuccess)
		{
			HandleParrySuccessCompleted(); // keep the chain alive while the montage is still missing
			return;
		}
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
	this, NAME_None, MontageToPlay, 1.f, NAME_None, true,  1.f,  0.f,  true);
	Task->OnBlendOut.AddDynamic(this, &UGA_HitReaction::HandleBlendOut);
	if (bIsParrySuccess)
	{
		Task->OnCompleted.AddDynamic(this, &UGA_HitReaction::HandleParrySuccessCompleted);
	}
	else
	{
		Task->OnCompleted.AddDynamic(this, &UGA_HitReaction::HandleDone);
	}
	Task->OnInterrupted.AddDynamic(this, &UGA_HitReaction::HandleDone);
	Task->OnCancelled.AddDynamic(this, &UGA_HitReaction::HandleDone);
	Task->ReadyForActivation();
}

void UGA_HitReaction::HandleDone()
{
	if (!IsActive()) return;
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGA_HitReaction::HandleBlendOut()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	FGameplayEventData Data;
	Data.EventTag = CombatTags::Event_Combat_ReactionFinished;
	Data.Instigator = Avatar;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Avatar, Data.EventTag, Data);
}

void UGA_HitReaction::HandleParrySuccessCompleted()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	const AActor* Target = ParriedAttacker.Get();
	ParriedAttacker.Reset();
	
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);

	if (!Avatar) return;
	FGameplayEventData Data;
	Data.EventTag = CombatTags::Event_Ability_Counter;
	Data.Instigator = Avatar;
	Data.Target = Target;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Avatar, Data.EventTag, Data);
}
