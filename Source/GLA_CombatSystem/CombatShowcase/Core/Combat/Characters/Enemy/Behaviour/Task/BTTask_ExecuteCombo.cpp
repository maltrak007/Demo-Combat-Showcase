// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ExecuteCombo.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Characters/Enemy/EnemyBaseCharacter.h"
#include "CombatShowcase/Core/Combat/Components/CombatComboComponent.h"
#include "CombatShowcase/Core/Combat/Components/EnemyBehaviourComponent.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "CombatShowcase/Core/GAS/Abilities/GA_MeleeAbilityBase.h"


namespace
{
	ABaseCombatCharacter* GetCharacter(const UBehaviorTreeComponent& OwnerComp)
	{
		const AAIController* AIController = OwnerComp.GetAIOwner();
		return AIController ? Cast<ABaseCombatCharacter>(AIController->GetPawn()) : nullptr;
	}

	const FCombatStrikeRow* FindStrike(const UDataTable& Table, FName RowName)
	{
		return RowName.IsNone() ? nullptr : Table.FindRow<FCombatStrikeRow>(RowName, TEXT("BTTask_ExecuteCombo"), false);
	}

	// Longest chain that can start at this row, counted in strikes. The cap also guards against loops in the table.
	int32 LongestChain(const UDataTable& Table, FName RowName, int32 Cap)
	{
		const FCombatStrikeRow* Row = Cap > 0 ? FindStrike(Table, RowName) : nullptr;
		if (!Row) return 0;
		return 1 + FMath::Max(LongestChain(Table, Row->NextRowOnLight, Cap - 1),
		                      LongestChain(Table, Row->NextRowOnHeavy, Cap - 1));
	}
}

UBTTask_ExecuteCombo::UBTTask_ExecuteCombo()
{
	NodeName = TEXT("Execute Combo");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true; 
}

TArray<FName> UBTTask_ExecuteCombo::BuildPlan(const UDataTable& Table, int32 HitLimit) const
{
	TArray<FName> Openers;
	for (const FName& Opener : OpenerRows)
	{
		if (FindStrike(Table, Opener)) Openers.Add(Opener);
	}
	if (Openers.IsEmpty()) return {};

	TArray<FName> Result;
	FName Current = Openers[FMath::RandRange(0, Openers.Num() - 1)];
	while (!Current.IsNone() && Result.Num() < HitLimit)
	{
		Result.Add(Current);
		const FCombatStrikeRow* Row = FindStrike(Table, Current);
		const int32 Remaining = HitLimit - Result.Num();
		if (!Row || Remaining <= 0) break;

		TArray<FName> Options;
		for (const FName& Next : { Row->NextRowOnLight, Row->NextRowOnHeavy })
		{
			if (FindStrike(Table, Next)) Options.Add(Next);
		}
		if (Options.IsEmpty()) break;

		// Prefer continuations that can still fill the requested length, and fall back to a shorter chain.
		const TArray<FName> Reaching = Options.FilterByPredicate([&](const FName& Option)
		{
			return LongestChain(Table, Option, Remaining) >= Remaining;
		});
		const TArray<FName>& Pool = Reaching.IsEmpty() ? Options : Reaching;
		Current = Pool[FMath::RandRange(0, Pool.Num() - 1)];
	}
	return Result;
}

bool UBTTask_ExecuteCombo::StartStrike(UBehaviorTreeComponent& OwnerComp)
{
	ABaseCombatCharacter* Self = GetCharacter(OwnerComp);
	UCombatComboComponent* Stats = Self ? Self->GetCombatComboComponent() : nullptr;
	if (!Stats || !Plan.IsValidIndex(Step)) return false;

	// The melee ability plays the stats component's current row when one is set, so this selects the strike.
	Stats->AdvanceCombo(Plan[Step]);

	FGameplayEventData Data;
	Data.EventTag = CombatTags::Event_Ability_LightAttack; 
	Data.Instigator = Self;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Self, Data.EventTag, Data);

	bStrikeStarted = false;
	bStrikeEnded = false;
	bStrikeCancelled = false;
	StrikeRequestedAt = Self->GetWorld()->GetTimeSeconds();
	return true;
}

EBTNodeResult::Type UBTTask_ExecuteCombo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABaseCombatCharacter* Self = GetCharacter(OwnerComp);
	UCombatComboComponent* Stats = Self ? Self->GetCombatComboComponent() : nullptr;
	UAbilitySystemComponent* ASC = Self ? Self->GetAbilitySystemComponent() : nullptr;
	const UDataTable* Table = Stats ? Stats->GetStrikeDataTable() : nullptr;
	if (!Table || !ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Execute Combo: %s has no strike table or ability system."), *GetNameSafe(Self));
		return EBTNodeResult::Failed;
	}
	if (ASC->HasMatchingGameplayTag(CombatTags::Ability_Type_Attack)) return EBTNodeResult::Failed; // already mid-strike

	// How many strikes this turn lasts comes from the enemy's behaviour row. The node's own MaxHits stands in without one.
	int32 HitLimit = MaxHits;
	if (const AEnemyBaseCharacter* Enemy = Cast<AEnemyBaseCharacter>(Self))
	{
		if (const UEnemyBehaviourComponent* Behaviour = Enemy->GetBehaviourComponent()) HitLimit = Behaviour->RollStrikes();
	}
	Plan = BuildPlan(*Table, HitLimit);
	if (Plan.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Execute Combo: no valid opener among the OpenerRows on %s."), *Self->GetName());
		return EBTNodeResult::Failed;
	}
	UE_LOG(LogTemp, Log, TEXT("%s combo plan: %s"), *Self->GetName(),
		*FString::JoinBy(Plan, TEXT(" > "), [](const FName& Row) { return Row.ToString(); }));

	BoundASC = ASC;
	AbilityEndedHandle = ASC->OnAbilityEnded.AddUObject(this, &UBTTask_ExecuteCombo::HandleAbilityEnded);

	Step = 0;
	return StartStrike(OwnerComp) ? EBTNodeResult::InProgress : EBTNodeResult::Failed;
}

void UBTTask_ExecuteCombo::HandleAbilityEnded(const FAbilityEndedData& Data)
{
	// Only the melee ability counts, and only once we've seen it start.
	if (!bStrikeStarted || !Data.AbilityThatEnded || !Data.AbilityThatEnded->IsA<UGA_MeleeAbilityBase>()) return;
	bStrikeEnded = true;
	bStrikeCancelled = Data.bWasCancelled;
}

void UBTTask_ExecuteCombo::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ABaseCombatCharacter* Self = GetCharacter(OwnerComp);
	UAbilitySystemComponent* ASC = Self ? Self->GetAbilitySystemComponent() : nullptr;
	if (!ASC)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (!bStrikeStarted)
	{
		if (ASC->HasMatchingGameplayTag(CombatTags::Ability_Type_Attack))
		{
			bStrikeStarted = true;
		}
		else if (Self->GetWorld()->GetTimeSeconds() - StrikeRequestedAt > StartTimeout)
		{
			UE_LOG(LogTemp, Warning, TEXT("Execute Combo: %s's strike '%s' never started. Is the melee ability granted?"),
				*Self->GetName(), *Plan[Step].ToString());
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
		return;
	}

	if (!bStrikeEnded) return; // still playing

	// Cancelled means hit, parried, or interrupted: stop here, and don't start another strike while staggering.
	if (bStrikeCancelled)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (++Step >= Plan.Num())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
	if (!StartStrike(OwnerComp))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

EBTNodeResult::Type UBTTask_ExecuteCombo::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Stop chaining but let the swing in progress finish, so a state change doesn't cut an attack mid-animation.
	return EBTNodeResult::Aborted;
}

void UBTTask_ExecuteCombo::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (BoundASC.IsValid())
	{
		BoundASC->OnAbilityEnded.Remove(AbilityEndedHandle);
	}
	BoundASC.Reset();
	AbilityEndedHandle.Reset();

	// A completed ability resets the combo itself. Anything else leaves a stale row behind.
	if (TaskResult != EBTNodeResult::Succeeded)
	{
		if (const ABaseCombatCharacter* Self = GetCharacter(OwnerComp))
		{
			if (UCombatComboComponent* Stats = Self->GetCombatComboComponent()) Stats->ResetCombo();
		}
	}

	Plan.Reset();
	Step = 0;
	bStrikeStarted = bStrikeEnded = bStrikeCancelled = false;
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

FString UBTTask_ExecuteCombo::GetStaticDescription() const
{
	return FString::Printf(TEXT("up to %d hits, from %d opener row(s)"), MaxHits, OpenerRows.Num());
}