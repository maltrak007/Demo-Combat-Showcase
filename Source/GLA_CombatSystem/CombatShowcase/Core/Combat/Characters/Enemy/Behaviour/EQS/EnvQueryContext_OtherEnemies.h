// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_OtherEnemies.generated.h"

/**
 * Every living enemy except the one asking. Used with a Distance test to keep enemies from stacking on one spot.
 */
UCLASS()
class GLA_COMBATSYSTEM_API UEnvQueryContext_OtherEnemies : public UEnvQueryContext
{
	GENERATED_BODY()
public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
