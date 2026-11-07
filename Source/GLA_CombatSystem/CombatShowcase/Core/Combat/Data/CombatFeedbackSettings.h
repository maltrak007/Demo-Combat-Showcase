// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/DeveloperSettings/Public/Engine/DeveloperSettings.h"
#include "CombatFeedbackSettings.generated.h"

class UCombatFeedbackProfile;
/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Combat Feedback"))
class GLA_COMBATSYSTEM_API UCombatFeedbackSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	// Used by every character that doesn't set its own override.
	UPROPERTY(Config, EditAnywhere, Category = "Feedback") TSoftObjectPtr<UCombatFeedbackProfile> DefaultProfile;
};
