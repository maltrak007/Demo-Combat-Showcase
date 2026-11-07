// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "CombatShowcase/Core/Combat/Characters/Enemy/Behaviour/BehaviourData.h"
#include "WeaponBehaviourData.generated.h"

/**
 * One row of DT_BehaviourWeaponStats: how an enemy uses a weapon at a given skill level.
 * It holds parameters only. Which strikes exist stays in the weapon's moveset table.
 *
 * Matching: an empty WeaponTag or MasteryTag matches anything. A row naming a weapon beats a row that only names a
 * mastery, and a deeper weapon tag (Weapon.Type.Ranged.Pistol over Weapon.Type.Ranged) beats a shallower one.
 */
USTRUCT(BlueprintType)
struct FWeaponBehaviourRow : public FTableRowBase
{
	GENERATED_BODY()

	// ---- Keys ----
	UPROPERTY(EditAnywhere, Category = "Keys") FGameplayTag WeaponTag;
	UPROPERTY(EditAnywhere, Category = "Keys") FGameplayTag MasteryTag;

	// ---- General ----
	// Multiplies the walk speed the BT's Set Move Speed service asks for.
	UPROPERTY(EditAnywhere, Category = "General", meta = (ClampMin = "0.1")) float MoveSpeedScale = 1.f;
	UPROPERTY(EditAnywhere, Category = "General") bool bOverridePerception = false;
	UPROPERTY(EditAnywhere, Category = "General", meta = (EditCondition = "bOverridePerception")) FEnemyPerceptionSettings Perception;

	// ---- Melee: one attack token is one sequence of strikes ----
	UPROPERTY(EditAnywhere, Category = "Melee", meta = (ClampMin = "1")) int32 MinStrikes = 2;
	UPROPERTY(EditAnywhere, Category = "Melee", meta = (ClampMin = "1")) int32 MaxStrikes = 2;
	// Seconds the enemy waits before it can ask for another token, counted from the moment it gives this one back.
	UPROPERTY(EditAnywhere, Category = "Melee", meta = (ClampMin = "0.0")) float SequenceCooldown = 5.f;
	// Distance from the target the enemy stands at. 0 keeps the value set on the token service.
	UPROPERTY(EditAnywhere, Category = "Melee", meta = (ClampMin = "0.0")) float MeleeStandOffDistance = 0.f;

	// ---- Ranged ----
	UPROPERTY(EditAnywhere, Category = "Ranged", meta = (ClampMin = "1")) int32 MinBurstShots = 1;
	UPROPERTY(EditAnywhere, Category = "Ranged", meta = (ClampMin = "1")) int32 MaxBurstShots = 1;
	// The telegraph: seconds between the enemy raising its aim and the shot. Random in this range, and the UI indicator runs for exactly this long.
	UPROPERTY(EditAnywhere, Category = "Ranged", meta = (ClampMin = "0.0")) float MinFireDelay = 0.8f;
	UPROPERTY(EditAnywhere, Category = "Ranged", meta = (ClampMin = "0.0")) float MaxFireDelay = 1.4f;
	// Seconds between the shots of one burst. The telegraph is only for the first shot.
	UPROPERTY(EditAnywhere, Category = "Ranged", meta = (ClampMin = "0.0")) float ShotInterval = 0.35f;
	// Seconds before the next token request, counted from the end of the burst.
	UPROPERTY(EditAnywhere, Category = "Ranged", meta = (ClampMin = "0.0")) float BurstCooldown = 3.f;
	// Above 1 is a slower reload.
	UPROPERTY(EditAnywhere, Category = "Ranged", meta = (ClampMin = "0.1")) float ReloadTimeScale = 1.f;
	// The band the enemy tries to stand in. A wider band is a sloppier shooter.
	UPROPERTY(EditAnywhere, Category = "Ranged", meta = (ClampMin = "0.0")) float PreferredRangeMin = 450.f;
	UPROPERTY(EditAnywhere, Category = "Ranged", meta = (ClampMin = "0.0")) float PreferredRangeMax = 800.f;
};

/**
 * One row of the archetype modifier table. Scales whatever the weapon row resolved to.
 */
USTRUCT(BlueprintType)
struct FArchetypeModifierRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Keys") FGameplayTag ArchetypeTag;

	UPROPERTY(EditAnywhere, Category = "Modifier", meta = (ClampMin = "0.1")) float CooldownScale = 1.f;
	UPROPERTY(EditAnywhere, Category = "Modifier", meta = (ClampMin = "0.1")) float MoveSpeedScale = 1.f;
	UPROPERTY(EditAnywhere, Category = "Modifier", meta = (ClampMin = "0.1")) float FireDelayScale = 1.f;
	UPROPERTY(EditAnywhere, Category = "Modifier", meta = (ClampMin = "0.1")) float ReloadTimeScale = 1.f;
	// Added to both ends of the melee strike range. Negative shortens it.
	UPROPERTY(EditAnywhere, Category = "Modifier") int32 StrikeBonus = 0;
};
