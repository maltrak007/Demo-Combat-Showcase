// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBehaviourComponent.h"

#include "Engine/DataTable.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Subsystem/CombatDirectorSettings.h"
#include "WeaponComponent.h"

const FWeaponBehaviourRow& UEnemyBehaviourComponent::GetStats() const
{
	const ABaseCombatCharacter* Owner = Cast<ABaseCombatCharacter>(GetOwner());
	const UWeaponComponent* Weapons = Owner ? Owner->GetWeaponComponent() : nullptr;
	const FGameplayTag WeaponTag = Weapons ? Weapons->GetCarriedWeaponTag() : FGameplayTag();

	// The weapon can be assigned after the enemy is possessed, so it is checked on every read, not once.
	if (!bResolved || ResolvedForWeapon != WeaponTag)
	{
		Resolve(WeaponTag);
	}
	return Resolved;
}

void UEnemyBehaviourComponent::Resolve(const FGameplayTag& WeaponTag) const
{
	const UCombatDirectorSettings* Settings = GetDefault<UCombatDirectorSettings>();
	Resolved = FWeaponBehaviourRow(); // the row's defaults stand in for anything the tables don't cover
	ResolvedForWeapon = WeaponTag;
	bResolved = true;

	const UDataTable* Table = Settings->BehaviourTable.LoadSynchronous();
	if (!Table)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: no behaviour table. Set one in Project Settings > Combat Director. Using defaults."),
			*GetNameSafe(GetOwner()));
		return;
	}

	// Most specific row wins. A row that names a weapon outranks one that only names a mastery.
	const FWeaponBehaviourRow* Best = nullptr;
	int32 BestScore = -1;
	Table->ForeachRow<FWeaponBehaviourRow>(TEXT("EnemyBehaviour"), [&](const FName&, const FWeaponBehaviourRow& Row)
	{
		int32 Score = 0;
		if (Row.WeaponTag.IsValid())
		{
			if (!WeaponTag.IsValid() || !WeaponTag.MatchesTag(Row.WeaponTag)) return;
			Score += 100 + Row.WeaponTag.GetGameplayTagParents().Num(); // deeper tag, more specific
		}
		if (Row.MasteryTag.IsValid())
		{
			if (!MasteryTag.IsValid() || !MasteryTag.MatchesTag(Row.MasteryTag)) return;
			Score += 50;
		}
		if (Score > BestScore) { Best = &Row; BestScore = Score; }
	});

	if (Best)
	{
		Resolved = *Best;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: no behaviour row matches weapon '%s' and mastery '%s'. Using defaults."),
			*GetNameSafe(GetOwner()), *WeaponTag.ToString(), *MasteryTag.ToString());
	}

	// The archetype scales whatever was found.
	if (!ArchetypeTag.IsValid()) return;
	const UDataTable* ArchetypeTable = Settings->ArchetypeTable.LoadSynchronous();
	if (!ArchetypeTable) return;

	const FArchetypeModifierRow* Modifier = nullptr;
	int32 ModifierDepth = -1;
	ArchetypeTable->ForeachRow<FArchetypeModifierRow>(TEXT("EnemyArchetype"), [&](const FName&, const FArchetypeModifierRow& Row)
	{
		if (!Row.ArchetypeTag.IsValid() || !ArchetypeTag.MatchesTag(Row.ArchetypeTag)) return;
		const int32 Depth = Row.ArchetypeTag.GetGameplayTagParents().Num();
		if (Depth > ModifierDepth) { Modifier = &Row; ModifierDepth = Depth; }
	});
	if (!Modifier) return;

	Resolved.SequenceCooldown *= Modifier->CooldownScale;
	Resolved.BurstCooldown *= Modifier->CooldownScale;
	Resolved.MoveSpeedScale *= Modifier->MoveSpeedScale;
	Resolved.MinFireDelay *= Modifier->FireDelayScale;
	Resolved.MaxFireDelay *= Modifier->FireDelayScale;
	Resolved.ReloadTimeScale *= Modifier->ReloadTimeScale;
	Resolved.MinStrikes = FMath::Max(1, Resolved.MinStrikes + Modifier->StrikeBonus);
	Resolved.MaxStrikes = FMath::Max(1, Resolved.MaxStrikes + Modifier->StrikeBonus);
}

int32 UEnemyBehaviourComponent::RollStrikes() const
{
	const FWeaponBehaviourRow& Stats = GetStats();
	return FMath::RandRange(Stats.MinStrikes, FMath::Max(Stats.MinStrikes, Stats.MaxStrikes));
}

int32 UEnemyBehaviourComponent::RollBurstShots() const
{
	const FWeaponBehaviourRow& Stats = GetStats();
	return FMath::RandRange(Stats.MinBurstShots, FMath::Max(Stats.MinBurstShots, Stats.MaxBurstShots));
}

float UEnemyBehaviourComponent::RollFireDelay() const
{
	const FWeaponBehaviourRow& Stats = GetStats();
	return FMath::FRandRange(Stats.MinFireDelay, FMath::Max(Stats.MinFireDelay, Stats.MaxFireDelay));
}

float UEnemyBehaviourComponent::GetReloadPlayRate() const
{
	return 1.f / FMath::Max(0.1f, GetStats().ReloadTimeScale);
}
