// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviourData.generated.h"

/**
 * Shared enum used by EnemyAIController, BTService_UpdateAIState,
 * and any other AI class that needs to read or write the AIState BB key.
 *
 * Defined in its own header to avoid circular includes between the
 * controller and the service.
 *
 * State ownership:
 *   EnemyAIController  → writes Chase / Search   (perception events)
 *   BTService          → writes InCombat / Chase  (distance evaluation)
 *   Both               → write Patrol             (no target fallback)
 */
UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
	Patrol UMETA(DisplayName = "Patrol"),
	Chase UMETA(DisplayName = "Chase"),
	Leashing UMETA(DisplayName = "Leashing"),
	MeleeCombat UMETA(DisplayName = "Melee Combat"),
	RangedCombat UMETA(DisplayName = "Ranged Combat")
};

USTRUCT(BlueprintType)
struct FEnemyPerceptionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (ClampMin = "100.0")) float SightRadius = 750.f;
	// Larger than SightRadius: the gap is the hysteresis before an enemy loses sight.
	UPROPERTY(EditAnywhere, meta = (ClampMin = "100.0")) float LoseSightRadius = 1000.f;
	// Half-angle around the enemy's facing. 180 sees everything around it, which suits a lane brawler.
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1.0", ClampMax = "180.0")) float PeripheralVisionAngle = 70.f;
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0")) float StimulusMaxAge = 8.f;
};

/**
 * Centralized Blackboard key names.
 * Include this header wherever BB keys are read or written.
 * Never hardcode key name strings — typos cause silent runtime failures.
 */
namespace EnemyBBKeys
{
	static const FName TargetActor = TEXT("TargetActor");
	static const FName LastKnownPos = TEXT("LastKnownPosition");
	static const FName HasLOS = TEXT("HasLineOfSight");
	static const FName AIState = TEXT("AIState");
	static const FName MoveToLocation = TEXT("MoveToLocation");
	static const FName HomeLocation = TEXT("HomeLocation");
}
