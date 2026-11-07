// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_EquipWeapon.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Components/WeaponComponent.h"
#include "CombatShowcase/Core/Combat/Data/PDT_Weapon.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"

UGA_EquipWeapon::UGA_EquipWeapon()
{
	AbilityTags.AddTag(CombatTags::Ability_Type_Equip);
	ActivationOwnedTags.AddTag(CombatTags::Ability_Type_Equip);
	ActivationOwnedTags.AddTag(CombatTags::State_Combat_MovementLocked);
	// A finisher scene was picked for the weapon in hand. Swapping it mid-scene would break the scene.
	ActivationBlockedTags.AddTag(CombatTags::Ability_Type_Finisher);

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = CombatTags::Event_Ability_EquipWeapon;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UGA_EquipWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ABaseCombatCharacter* Character = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo());
	UWeaponComponent* Weapons = Character ? Character->GetWeaponComponent() : nullptr;
	const UPDT_Weapon* Carried = Weapons ? Weapons->GetCarried() : nullptr;
	if (!Carried || !HasValidAnimInstance(ActorInfo) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bDrawing = !Weapons->IsDrawn(); // a toggle: whichever it isn't right now
	bCommitted = false;
	UAnimMontage* Montage = bDrawing ? Carried->GetDrawMontage() : Carried->GetSheatheMontage();
	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no %s montage. Equipping instantly."), *GetNameSafe(Carried), bDrawing ? TEXT("draw") : TEXT("sheathe"));
		Commit();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UAbilityTask_WaitGameplayEvent* CommitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CombatTags::Event_Combat_WeaponCommit, nullptr, true);
	CommitTask->EventReceived.AddDynamic(this, &UGA_EquipWeapon::HandleCommitEvent);
	CommitTask->ReadyForActivation();

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, Montage, 1.f, NAME_None, true, 1.f, 0.f, /*bAllowInterruptAfterBlendOut*/ true);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_EquipWeapon::HandleMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_EquipWeapon::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_EquipWeapon::HandleMontageInterrupted);
	MontageTask->ReadyForActivation();
}

void UGA_EquipWeapon::Commit()
{
	if (bCommitted) return;
	bCommitted = true;
	ABaseCombatCharacter* Character = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo());
	UWeaponComponent* Weapons = Character ? Character->GetWeaponComponent() : nullptr;
	if (!Weapons) return;
	if (bDrawing) Weapons->Draw(); else Weapons->Sheathe();
}

void UGA_EquipWeapon::HandleCommitEvent(FGameplayEventData Payload) { Commit(); }

void UGA_EquipWeapon::HandleMontageCompleted()
{
	if (!bCommitted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: the montage has no WeaponCommit notify. Committing at the end."), *GetName());
		Commit(); // never leave the weapon un-drawn because a notify is missing
	}
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGA_EquipWeapon::HandleMontageInterrupted()
{
	// Interrupted before the notify: nothing changed. Interrupted after it: the swap already happened.
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}