// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_MeleeAbilityBase.h"
#include "CombatShowcase/Core/GAS/CombatGameplayAbility.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "GA_LightAttack.generated.h"

/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UGA_LightAttack : public UGA_MeleeAbilityBase
{
	GENERATED_BODY()
	
public:
	UGA_LightAttack() { FAbilityTriggerData T; T.TriggerTag = CombatTags::Event_Ability_LightAttack; T.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent; AbilityTriggers.Add(T); }
};
