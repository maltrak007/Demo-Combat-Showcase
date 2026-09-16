// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CombatGameplayAbility.generated.h"


/**
 * Base class for all combat gameplay abilities.
 */
UCLASS(Abstract)
class GLA_COMBATSYSTEM_API UCombatGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UCombatGameplayAbility()
	{
		InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
		NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	}
	
	bool HasValidAnimInstance(const FGameplayAbilityActorInfo* ActorInfo) const;
};
