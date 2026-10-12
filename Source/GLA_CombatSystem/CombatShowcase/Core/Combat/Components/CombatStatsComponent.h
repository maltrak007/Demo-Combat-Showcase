// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Runtime/GameplayTags/Classes/GameplayTagContainer.h"
#include "CombatStatsComponent.generated.h"


class UGameplayEffect;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GLA_COMBATSYSTEM_API UCombatStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatStatsComponent() { PrimaryComponentTick.bCanEverTick = false; }

	FName GetCurrentRow() const { return CurrentRow; }
	void AdvanceCombo(FName NextRow) { CurrentRow = NextRow; }
	void ResetCombo() { CurrentRow = NAME_None; }

	// Cached by the ability right after it resolves its own row, so continuation
	// lookup never needs a second Data Table hit outside the ability itself.
	void CacheContinuations(FName OnLight, FName OnHeavy)
	{
		CachedNextOnLight = OnLight;
		CachedNextOnHeavy = OnHeavy;
	}

	FName ResolveContinuation(bool bWasHeavyRequest) const
	{
		return bWasHeavyRequest ? CachedNextOnHeavy : CachedNextOnLight;
	}

	void RequestComboContinuation(const FGameplayTag& AttackTag) { RequestedAttackTag = AttackTag; }

	FGameplayTag ConsumeComboRequest()
	{
		const FGameplayTag Result = RequestedAttackTag;
		RequestedAttackTag = FGameplayTag();
		return Result;
	}

	// CODE FOR DISPLAYING IT ON HUD && ANCHOR IT TO TRIGGER GAMEPLAY EFFECTS FOR INCREASED DAMAGE, SPEED BOOST...ETC
	int32 GetComboHitCount() const { return ComboHitCount; }
	void RegisterHitLanded() { ComboHitCount++; }
	void RegisterDamageTaken() { ComboHitCount = 0; }

	UDataTable* GetStrikeDataTable() const { return StrikeDataTable; }
	UDataTable* GetCounterStrikeDataTable() const { return CounterStrikeDataTable; }

	void SetStrikeDataTable(UDataTable* NewTable)
	{
		StrikeDataTable = NewTable;
		ResetCombo();
		CacheContinuations(NAME_None, NAME_None);
		ConsumeComboRequest();
	}

	void SetCounterStrikeDataTable(UDataTable* NewTable)
	{
		CounterStrikeDataTable = NewTable;
	}

	TSubclassOf<UGameplayEffect> GetDamageEffectClass() const { return DamageEffectClass; }
	
	TSubclassOf<UGameplayEffect> GetStaminaDamageEffectClass() const { return StaminaDamageEffectClass;}
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Strike")
	UDataTable* StrikeDataTable = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Strike")
	UDataTable* CounterStrikeDataTable = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> StaminaDamageEffectClass;

private:
	FName CurrentRow;
	FName CachedNextOnLight;
	FName CachedNextOnHeavy;
	FGameplayTag RequestedAttackTag;

	int32 ComboHitCount = 0;
};
