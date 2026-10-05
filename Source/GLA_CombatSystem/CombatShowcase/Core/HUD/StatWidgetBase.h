// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "StatWidgetBase.generated.h"

class ABaseCombatCharacter;
/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UStatWidgetBase : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	TWeakObjectPtr<ABaseCombatCharacter> OwningCharacter;

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void InitializeForCharacter(ABaseCombatCharacter* InCharacter);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Stats")
	void OnStatsReady(); 
	
	
};
