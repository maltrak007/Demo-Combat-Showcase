// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimNotifyState_GameplayEventWindow.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "AnimNotifyState_BlockWindow.generated.h"

/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UAnimNotifyState_BlockWindow : public UAnimNotifyState_GameplayEventWindow
{
	GENERATED_BODY()
public:
	UAnimNotifyState_BlockWindow() { OpenEventTag = CombatTags::Event_Combat_BlockWindowOpen; CloseEventTag = CombatTags::Event_Combat_BlockWindowClose; }
};
