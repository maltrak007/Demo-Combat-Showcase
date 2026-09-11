// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_MeleeAbilityBase.h"
#include "CombatShowcase/Core/GAS/CombatGameplayAbility.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "GA_HeavyAttack.generated.h"

/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UGA_HeavyAttack : public UGA_MeleeAbilityBase
{
	GENERATED_BODY()
public:
	UGA_HeavyAttack() { FAbilityTriggerData T; T.TriggerTag = CombatTags::Event_Ability_HeavyAttack; T.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent; AbilityTriggers.Add(T); }
};
