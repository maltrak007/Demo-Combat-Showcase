// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Aim.h"

#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Components/WeaponComponent.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"

UGA_Aim::UGA_Aim()
{
	AbilityTags.AddTag(CombatTags::Ability_Type_Aim);

	// Aiming plants the character. Anything else that locks movement (a swing, a roll, a reaction, an equip, a guard)
	// also keeps the aim from starting, which is the one rule that covers all of them.
	ActivationOwnedTags.AddTag(CombatTags::State_Combat_MovementLocked);
	ActivationBlockedTags.AddTag(CombatTags::State_Combat_MovementLocked);
	ActivationBlockedTags.AddTag(CombatTags::State_Combat_Stunned);

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = CombatTags::Event_Ability_Aim;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

bool UGA_Aim::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                 FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;

	const ABaseCombatCharacter* Avatar = Cast<ABaseCombatCharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
	const UWeaponComponent* Weapons = Avatar ? Avatar->GetWeaponComponent() : nullptr;
	return Weapons && Weapons->IsRangedDrawn();
}

void UGA_Aim::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                              const FGameplayAbilityActivationInfo ActivationInfo,
                              const FGameplayEventData* TriggerEventData)
{
	ABaseCombatCharacter* Avatar = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo());
	UWeaponComponent* Weapons = Avatar ? Avatar->GetWeaponComponent() : nullptr;
	if (!Weapons || !Weapons->IsRangedDrawn() || !HasValidAnimInstance(ActorInfo)
		|| !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Weapons->OnDrawnChanged.AddUniqueDynamic(this, &UGA_Aim::HandleDrawnChanged);
	Avatar->SetAiming(true); // the animation blueprint and the shot ability read this
}

void UGA_Aim::HandleDrawnChanged(bool bDrawn)
{
	if (!bDrawn && IsActive())
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
	}
}

void UGA_Aim::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                         const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                         bool bWasCancelled)
{
	if (ABaseCombatCharacter* Avatar = Cast<ABaseCombatCharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr))
	{
		if (UWeaponComponent* Weapons = Avatar->GetWeaponComponent())
		{
			Weapons->OnDrawnChanged.RemoveDynamic(this, &UGA_Aim::HandleDrawnChanged);
		}
		Avatar->SetAiming(false); // also levels the aim for next time
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
