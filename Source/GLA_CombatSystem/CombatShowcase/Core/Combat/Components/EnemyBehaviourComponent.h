// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "CombatShowcase/Core/Combat/Data/WeaponBehaviourData.h"
#include "EnemyBehaviourComponent.generated.h"

/**
 * Turns the enemy's weapon, mastery and archetype into one set of numbers for the behavior tree to read.
 * The result is cached, and rebuilt on its own when the carried weapon changes, so the tree never touches a data table.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GLA_COMBATSYSTEM_API UEnemyBehaviourComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyBehaviourComponent() { PrimaryComponentTick.bCanEverTick = false; }

	// The resolved numbers for the weapon the enemy carries right now.
	const FWeaponBehaviourRow& GetStats() const;

	// Each call rolls a fresh value inside the row's range, so two attacks from one enemy differ.
	int32 RollStrikes() const;
	int32 RollBurstShots() const;
	float RollFireDelay() const;

	// Montage play rate for a reload: a ReloadTimeScale of 2 is a rate of 0.5.
	float GetReloadPlayRate() const;

	// Forces a rebuild on the next read. Call it after changing mastery or archetype at runtime.
	void Invalidate() { bResolved = false; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behaviour") FGameplayTag MasteryTag;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behaviour") FGameplayTag ArchetypeTag;

private:
	void Resolve(const FGameplayTag& WeaponTag) const;

	mutable FWeaponBehaviourRow Resolved;
	mutable FGameplayTag ResolvedForWeapon;
	mutable bool bResolved = false;
};
