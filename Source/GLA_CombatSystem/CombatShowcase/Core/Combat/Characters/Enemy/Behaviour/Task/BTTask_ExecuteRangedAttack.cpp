// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ExecuteRangedAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Characters/Enemy/EnemyBaseCharacter.h"
#include "CombatShowcase/Core/Combat/Components/EnemyBehaviourComponent.h"
#include "CombatShowcase/Core/Combat/Components/WeaponComponent.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "CombatShowcase/Core/GAS/Abilities/GA_RangedAttack.h"

namespace
{
	ABaseCombatCharacter* GetCharacter(const UBehaviorTreeComponent& OwnerComp)
	{
		const AAIController* AIController = OwnerComp.GetAIOwner();
		return AIController ? Cast<ABaseCombatCharacter>(AIController->GetPawn()) : nullptr;
	}
}

UBTTask_ExecuteRangedAttack::UBTTask_ExecuteRangedAttack()
{
	NodeName = TEXT("Execute Ranged Attack");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true;
}

void UBTTask_ExecuteRangedAttack::RequestShot(ABaseCombatCharacter* Self, float TelegraphSeconds)
{
	// EventMagnitude is the telegraph. The ability raises the aim and plays the target's indicator for that long.
	FGameplayEventData Data;
	Data.EventTag = CombatTags::Event_Ability_LightAttack;
	Data.Instigator = Self;
	Data.EventMagnitude = TelegraphSeconds;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Self, Data.EventTag, Data);

	bShotStarted = false;
	bShotEnded = false;
	bShotCancelled = false;
	bWaitingForInterval = false;
	RequestedAt = Self->GetWorld()->GetTimeSeconds();
}

EBTNodeResult::Type UBTTask_ExecuteRangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABaseCombatCharacter* Self = GetCharacter(OwnerComp);
	UWeaponComponent* Weapons = Self ? Self->GetWeaponComponent() : nullptr;
	UAbilitySystemComponent* ASC = Self ? Self->GetAbilitySystemComponent() : nullptr;
	if (!Weapons || !ASC || !Weapons->IsRangedDrawn())
	{
		UE_LOG(LogTemp, Warning, TEXT("Execute Ranged Attack: %s has no drawn ranged weapon."), *GetNameSafe(Self));
		return EBTNodeResult::Failed;
	}
	if (!Weapons->HasAmmo()) return EBTNodeResult::Failed;                                  // the waiting branch reloads
	if (ASC->HasMatchingGameplayTag(CombatTags::Ability_Type_Attack)) return EBTNodeResult::Failed; // already mid-attack

	int32 Shots = FallbackShots;
	float FirstDelay = FallbackFireDelay;
	ShotInterval = FallbackShotInterval;
	if (const AEnemyBaseCharacter* Enemy = Cast<AEnemyBaseCharacter>(Self))
	{
		if (const UEnemyBehaviourComponent* Behaviour = Enemy->GetBehaviourComponent())
		{
			Shots = Behaviour->RollBurstShots();
			FirstDelay = Behaviour->RollFireDelay();
			ShotInterval = Behaviour->GetStats().ShotInterval;
		}
	}
	ShotsPlanned = FMath::Clamp(Shots, 1, Weapons->GetAmmo()); // never plan a shot the magazine can't give
	ShotsFired = 0;

	BoundASC = ASC;
	AbilityEndedHandle = ASC->OnAbilityEnded.AddUObject(this, &UBTTask_ExecuteRangedAttack::HandleAbilityEnded);

	// Only the first shot of a burst gets the telegraph. The rest follow at the burst's shot interval.
	RequestShot(Self, FirstDelay);
	return EBTNodeResult::InProgress;
}

void UBTTask_ExecuteRangedAttack::HandleAbilityEnded(const FAbilityEndedData& Data)
{
	if (!Data.AbilityThatEnded || !Data.AbilityThatEnded->IsA<UGA_RangedAttack>()) return;
	bShotStarted = true; // an ability that ended before the tick saw it start still counts as having run
	bShotEnded = true;
	bShotCancelled = Data.bWasCancelled;
}

void UBTTask_ExecuteRangedAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ABaseCombatCharacter* Self = GetCharacter(OwnerComp);
	UAbilitySystemComponent* ASC = Self ? Self->GetAbilitySystemComponent() : nullptr;
	if (!ASC)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	const double Now = Self->GetWorld()->GetTimeSeconds();

	if (bWaitingForInterval)
	{
		if (Now >= NextShotAt) RequestShot(Self, 0.f);
		return;
	}

	if (!bShotStarted)
	{
		if (ASC->HasMatchingGameplayTag(CombatTags::Ability_Type_Attack))
		{
			bShotStarted = true;
		}
		else if (Now - RequestedAt > StartTimeout)
		{
			UE_LOG(LogTemp, Warning, TEXT("Execute Ranged Attack: %s's shot never started. Is GA_RangedAttack granted?"), *Self->GetName());
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
		return;
	}

	if (!bShotEnded) return; // still winding up or playing

	// Cancelled means the shooter was hit or interrupted: the burst stops here.
	if (bShotCancelled)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (++ShotsFired >= ShotsPlanned)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
	bWaitingForInterval = true;
	NextShotAt = Now + ShotInterval;
}

EBTNodeResult::Type UBTTask_ExecuteRangedAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Unlike a melee swing, a shot that is still winding up has to stop: the token it was fired under is gone.
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

void UBTTask_ExecuteRangedAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (BoundASC.IsValid())
	{
		BoundASC->OnAbilityEnded.Remove(AbilityEndedHandle);
	}
	BoundASC.Reset();
	AbilityEndedHandle.Reset();

	ShotsPlanned = ShotsFired = 0;
	bWaitingForInterval = bShotStarted = bShotEnded = bShotCancelled = false;
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

FString UBTTask_ExecuteRangedAttack::GetStaticDescription() const
{
	return TEXT("a burst from the behaviour row:\ntelegraph, then the shots");
}
