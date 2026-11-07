// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ReloadWeapon.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Characters/Enemy/EnemyBaseCharacter.h"
#include "CombatShowcase/Core/Combat/Components/EnemyBehaviourComponent.h"
#include "CombatShowcase/Core/Combat/Components/WeaponComponent.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"

namespace
{
	ABaseCombatCharacter* GetCharacter(const UBehaviorTreeComponent& OwnerComp)
	{
		const AAIController* AIController = OwnerComp.GetAIOwner();
		return AIController ? Cast<ABaseCombatCharacter>(AIController->GetPawn()) : nullptr;
	}
}

UBTTask_ReloadWeapon::UBTTask_ReloadWeapon()
{
	NodeName = TEXT("Reload Weapon");
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_ReloadWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABaseCombatCharacter* Self = GetCharacter(OwnerComp);
	UWeaponComponent* Weapons = Self ? Self->GetWeaponComponent() : nullptr;
	UAbilitySystemComponent* ASC = Self ? Self->GetAbilitySystemComponent() : nullptr;
	if (!Weapons || !ASC) return EBTNodeResult::Failed;

	if (!Weapons->GetRangedSpec() || Weapons->IsMagazineFull()) return EBTNodeResult::Succeeded; // nothing to reload
	if (!Weapons->IsRangedDrawn()) return EBTNodeResult::Failed;                                   // drawing is Equip Weapon's job
	if (ASC->HasMatchingGameplayTag(CombatTags::Ability_Type_Attack)) return EBTNodeResult::Failed;

	if (bStopMovement)
	{
		if (AAIController* AIController = OwnerComp.GetAIOwner()) AIController->StopMovement();
	}

	// EventMagnitude is the montage play rate. A slower mastery reloads at a lower rate, which is the opening it leaves.
	float PlayRate = 1.f;
	if (const AEnemyBaseCharacter* Enemy = Cast<AEnemyBaseCharacter>(Self))
	{
		if (const UEnemyBehaviourComponent* Behaviour = Enemy->GetBehaviourComponent()) PlayRate = Behaviour->GetReloadPlayRate();
	}

	FGameplayEventData Data;
	Data.EventTag = CombatTags::Event_Ability_Reload;
	Data.Instigator = Self;
	Data.EventMagnitude = PlayRate;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Self, Data.EventTag, Data);

	bStarted = false;
	RequestedAt = Self->GetWorld()->GetTimeSeconds();
	return EBTNodeResult::InProgress;
}

void UBTTask_ReloadWeapon::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ABaseCombatCharacter* Self = GetCharacter(OwnerComp);
	UWeaponComponent* Weapons = Self ? Self->GetWeaponComponent() : nullptr;
	UAbilitySystemComponent* ASC = Self ? Self->GetAbilitySystemComponent() : nullptr;
	if (!Weapons || !ASC)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (ASC->HasMatchingGameplayTag(CombatTags::Ability_Type_Attack))
	{
		bStarted = true; // still reloading
		return;
	}

	// The ability has ended. A hit that cut it off leaves the magazine empty, and this says so.
	if (bStarted || Weapons->IsMagazineFull())
	{
		FinishLatentTask(OwnerComp, Weapons->IsMagazineFull() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
		return;
	}

	if (Self->GetWorld()->GetTimeSeconds() - RequestedAt > StartTimeout)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reload Weapon: %s's reload never started. Is GA_RangedAttack granted?"), *Self->GetName());
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

EBTNodeResult::Type UBTTask_ReloadWeapon::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// A reload cut short refills nothing, so a branch change can stop it cleanly.
	if (const ABaseCombatCharacter* Self = GetCharacter(OwnerComp))
	{
		if (UAbilitySystemComponent* ASC = Self->GetAbilitySystemComponent())
		{
			FGameplayTagContainer AttackTags;
			AttackTags.AddTag(CombatTags::Ability_Type_Attack);
			ASC->CancelAbilities(&AttackTags);
		}
	}
	return EBTNodeResult::Aborted;
}

FString UBTTask_ReloadWeapon::GetStaticDescription() const
{
	return TEXT("reload the ranged weapon, at the mastery's pace");
}
