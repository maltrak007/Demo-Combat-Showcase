// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_AttackTarget.generated.h"

/**
 * The actor the querying enemy is attacking: its blackboard TargetActor, or the first player pawn when that is empty.
 */
UCLASS()
class GLA_COMBATSYSTEM_API UEnvQueryContext_AttackTarget : public UEnvQueryContext
{
	GENERATED_BODY()
public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
