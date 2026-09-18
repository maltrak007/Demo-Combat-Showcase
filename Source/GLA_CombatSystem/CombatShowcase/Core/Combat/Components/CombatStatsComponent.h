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
	int32 GetComboIndex() const { return ComboIndex; }
	void AdvanceCombo();   // increments index
	void ResetCombo();     // back to 0 — called on a failed continuation
	
	// CODE FOR DISPLAYING IT ON HUD && ANCHOR IT TO TRIGGER GAMEPLAY EFFECTS FOR INCREASED DAMAGE, SPEED BOOST...ETC
	int32 GetComboHitCount() const { return ComboHitCount; }
	void RegisterHitLanded();     // successive hit landed
	void RegisterDamageTaken();   // resets the streak — called the moment the character takes damage
	
	// WINDOW TO BLOCK NON-ATTACK INPUTS DURING COMBO ANIMATION 
	bool IsBlockWindowOpen() const { return bBlockWindowOpen; }
	void SetBlockWindowOpen(bool bOpen) { bBlockWindowOpen = bOpen; }
	
	void RequestComboContinuation(const FGameplayTag& AttackTag) { RequestedAttackTag = AttackTag; }
	FGameplayTag ConsumeComboRequest() { const FGameplayTag Result = RequestedAttackTag; RequestedAttackTag = FGameplayTag(); return Result; }

private:
	int32 ComboIndex = 0;
	int32 ComboHitCount = 0;
	bool bBlockWindowOpen = false;
	
	FGameplayTag RequestedAttackTag;
};
