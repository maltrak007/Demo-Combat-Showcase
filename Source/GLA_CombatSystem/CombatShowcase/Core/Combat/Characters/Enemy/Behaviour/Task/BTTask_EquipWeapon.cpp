// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_EquipWeapon.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Components/WeaponComponent.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"

namespace
{
	ABaseCombatCharacter* GetCharacter(const UBehaviorTreeComponent& OwnerComp)
	{
		const AAIController* AIController = OwnerComp.GetAIOwner();
		return AIController ? Cast<ABaseCombatCharacter>(AIController->GetPawn()) : nullptr;
	}

	bool GoalReached(const UWeaponComponent& Weapons, EWeaponGoal Goal)
	{
		return Goal == EWeaponGoal::Draw ? Weapons.IsDrawn() : !Weapons.IsDrawn();
	}
}

UBTTask_EquipWeapon::UBTTask_EquipWeapon()
{
	NodeName = TEXT("Equip Weapon");
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_EquipWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABaseCombatCharacter* Self = GetCharacter(OwnerComp);
	UWeaponComponent* Weapons = Self ? Self->GetWeaponComponent() : nullptr;
	UAbilitySystemComponent* ASC = Self ? Self->GetAbilitySystemComponent() : nullptr;
	if (!Weapons || !ASC) return EBTNodeResult::Failed;

	bAbilityStarted = false;
	RequestedAt = Self->GetWorld()->GetTimeSeconds();

	// An equip is already running (the tree re-entered the branch). Wait for it, then judge by the final state.
	if (ASC->HasMatchingGameplayTag(CombatTags::Ability_Type_Equip))
	{
		bAbilityStarted = true;
		return EBTNodeResult::InProgress;
	}

	if (GoalReached(*Weapons, Goal)) return EBTNodeResult::Succeeded; // already as wanted: nothing to do

	if (Goal == EWeaponGoal::Draw && !Weapons->GetCarried())
	{
		UE_LOG(LogTemp, Warning, TEXT("Equip Weapon: %s carries no weapon to draw."), *Self->GetName());
		return EBTNodeResult::Failed;
	}

	if (bStopMovement)
	{
		if (AAIController* AIController = OwnerComp.GetAIOwner()) AIController->StopMovement();
	}

	// GA_EquipWeapon is a toggle, so it is only requested when the state differs from the goal.
	FGameplayEventData Data;
	Data.EventTag = CombatTags::Event_Ability_EquipWeapon;
	Data.Instigator = Self;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Self, Data.EventTag, Data);

	// An equip with no montage commits inside the send, so check once before waiting.
	return GoalReached(*Weapons, Goal) ? EBTNodeResult::Succeeded : EBTNodeResult::InProgress;
}

void UBTTask_EquipWeapon::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ABaseCombatCharacter* Self = GetCharacter(OwnerComp);
	UWeaponComponent* Weapons = Self ? Self->GetWeaponComponent() : nullptr;
	UAbilitySystemComponent* ASC = Self ? Self->GetAbilitySystemComponent() : nullptr;
	if (!Weapons || !ASC)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (ASC->HasMatchingGameplayTag(CombatTags::Ability_Type_Equip))
	{
		bAbilityStarted = true; // still drawing or sheathing
		return;
	}

	if (bAbilityStarted || GoalReached(*Weapons, Goal))
	{
		// The ability has ended. If a hit cut it off before its commit notify, the weapon never changed,
		// and the goal check says so.
		FinishLatentTask(OwnerComp, GoalReached(*Weapons, Goal) ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
		return;
	}

	if (Self->GetWorld()->GetTimeSeconds() - RequestedAt > StartTimeout)
	{
		UE_LOG(LogTemp, Warning, TEXT("Equip Weapon: %s's equip ability never started. Is it granted, or blocked by another ability?"), *Self->GetName());
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

EBTNodeResult::Type UBTTask_EquipWeapon::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Let the draw finish: it commits at its own notify, and cutting it off could leave a half-done equip.
	return EBTNodeResult::Aborted;
}

FString UBTTask_EquipWeapon::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s the weapon"), Goal == EWeaponGoal::Draw ? TEXT("draw") : TEXT("sheathe"));
}


