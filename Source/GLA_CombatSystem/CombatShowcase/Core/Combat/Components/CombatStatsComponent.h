// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Runtime/GameplayTags/Classes/GameplayTagContainer.h"
#include "CombatStatsComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GLA_COMBATSYSTEM_API UCombatStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FName GetCurrentRow() const { return CurrentRow; }
	void AdvanceCombo(FName NextRow) { CurrentRow = NextRow; }
	void ResetCombo() { CurrentRow = NAME_None; }
	
	// Cached by the ability right after it resolves its own row, so continuation
	// lookup never needs a second Data Table hit outside the ability itself.
	void CacheContinuations(FName OnLight, FName OnHeavy) { CachedNextOnLight = OnLight; CachedNextOnHeavy = OnHeavy; }
	FName ResolveContinuation(bool bWasHeavyRequest) const { return bWasHeavyRequest ? CachedNextOnHeavy : CachedNextOnLight; }
	
	void RequestComboContinuation(const FGameplayTag& AttackTag) { RequestedAttackTag = AttackTag; }
	FGameplayTag ConsumeComboRequest() { const FGameplayTag Result = RequestedAttackTag; RequestedAttackTag = FGameplayTag(); return Result; }
	
	// CODE FOR DISPLAYING IT ON HUD && ANCHOR IT TO TRIGGER GAMEPLAY EFFECTS FOR INCREASED DAMAGE, SPEED BOOST...ETC
	int32 GetComboHitCount() const { return ComboHitCount; }
	void RegisterHitLanded() { ComboHitCount++; }
	void RegisterDamageTaken() { ComboHitCount = 0; }
	
	// WINDOW TO BLOCK NON-ATTACK INPUTS DURING COMBO ANIMATION 
	bool IsBlockWindowOpen() const { return bBlockWindowOpen; }
	void SetBlockWindowOpen(bool bOpen) { bBlockWindowOpen = bOpen; }

private:
	FName CurrentRow;
	FName CachedNextOnLight;
	FName CachedNextOnHeavy;
	FGameplayTag RequestedAttackTag;
	
	int32 ComboHitCount = 0;
	bool bBlockWindowOpen = false;
	
};
