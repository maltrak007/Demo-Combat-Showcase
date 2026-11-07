// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CombatDirectorSettings.generated.h"

class UDataTable;

USTRUCT(BlueprintType)
struct FDirectorPhaseLimits
{
	GENERATED_BODY()

	// How many enemies may hold a melee token at once. The lane has two sides, so 2 is the ceiling.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "2")) int32 MaxMelee = 2;
	// How many enemies may hold the ranged token at once. There is one ranged slot.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "1")) int32 MaxRanged = 1;

	FDirectorPhaseLimits() = default;
	FDirectorPhaseLimits(int32 InMaxMelee, int32 InMaxRanged) : MaxMelee(InMaxMelee), MaxRanged(InMaxRanged) {}
};

/**
 * Project Settings > Combat Director. Tuning for UCombatDirectorSubsystem.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Combat Director"))
class GLA_COMBATSYSTEM_API UCombatDirectorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// ---- Lane ----
	// Horizontal axis of the level. The lane is world X in this project.
	UPROPERTY(Config, EditAnywhere, Category = "Lane") FVector SideAxis = FVector::ForwardVector;

	// ---- Tokens ----
	// Safety net: a token held longer than this is taken back (stuck or interrupted enemy).
	UPROPERTY(Config, EditAnywhere, Category = "Tokens", meta = (ClampMin = "1.0")) float TokenTimeout = 10.f;
	// After releasing a token, the same enemy has to wait this long before it can ask for another.
	UPROPERTY(Config, EditAnywhere, Category = "Tokens", meta = (ClampMin = "0.0")) float CooldownAfterRelease = 1.5f;
	// When an enemy's own side is taken, let it cross over to the free side.
	UPROPERTY(Config, EditAnywhere, Category = "Tokens") bool bAllowCrossSide = true;

	// ---- Intensity ----
	// When off, the token limits stay where SetMaxTokens put them.
	UPROPERTY(Config, EditAnywhere, Category = "Intensity") bool bAutoDirector = true;
	// Fraction of the target's max health lost, times this, is added to the intensity. 2 means half the bar is a full 1.0.
	UPROPERTY(Config, EditAnywhere, Category = "Intensity", meta = (ClampMin = "0.0")) float DamageToIntensity = 2.f;
	UPROPERTY(Config, EditAnywhere, Category = "Intensity", meta = (ClampMin = "0.0")) float IntensityDecayPerSecond = 0.05f;
	// Intensity at or above this sends the director into Relax.
	UPROPERTY(Config, EditAnywhere, Category = "Intensity", meta = (ClampMin = "0.0", ClampMax = "1.0")) float RelaxEnterIntensity = 0.5f;
	// Relax only ends once the intensity has fallen to this.
	UPROPERTY(Config, EditAnywhere, Category = "Intensity", meta = (ClampMin = "0.0", ClampMax = "1.0")) float RelaxExitIntensity = 0.2f;

	// ---- Phases: BuildUp -> Peak -> Relax -> BuildUp ----
	UPROPERTY(Config, EditAnywhere, Category = "Phases") FDirectorPhaseLimits BuildUpLimits = FDirectorPhaseLimits(1, 1);
	UPROPERTY(Config, EditAnywhere, Category = "Phases") FDirectorPhaseLimits PeakLimits = FDirectorPhaseLimits(2, 1);
	UPROPERTY(Config, EditAnywhere, Category = "Phases") FDirectorPhaseLimits RelaxLimits = FDirectorPhaseLimits(1, 0);

	// Seconds of calm pressure before the director escalates to Peak.
	UPROPERTY(Config, EditAnywhere, Category = "Phases", meta = (ClampMin = "0.0")) float BuildUpDuration = 8.f;
	// ---- Behaviour data ----
	// DT_BehaviourWeaponStats: one row per weapon and mastery. See FWeaponBehaviourRow.
	UPROPERTY(Config, EditAnywhere, Category = "Behaviour", meta = (RequiredAssetDataTags = "RowStructure=/Script/GLA_CombatSystem.WeaponBehaviourRow"))
	TSoftObjectPtr<UDataTable> BehaviourTable;
	// Optional: one row per archetype, scaling what the weapon row resolved to. See FArchetypeModifierRow.
	UPROPERTY(Config, EditAnywhere, Category = "Behaviour", meta = (RequiredAssetDataTags = "RowStructure=/Script/GLA_CombatSystem.ArchetypeModifierRow"))
	TSoftObjectPtr<UDataTable> ArchetypeTable;

	// Longest the director holds full pressure before easing off.
	UPROPERTY(Config, EditAnywhere, Category = "Phases", meta = (ClampMin = "0.0")) float PeakDuration = 12.f;
	// Shortest time spent in Relax.
	UPROPERTY(Config, EditAnywhere, Category = "Phases", meta = (ClampMin = "0.0")) float RelaxMinDuration = 5.f;
};
