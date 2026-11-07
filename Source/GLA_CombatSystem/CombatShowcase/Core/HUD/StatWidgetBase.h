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

	// An attacker is winding up. Play the indicator animation over Duration seconds, so it lands on the moment of the shot.
	UFUNCTION(BlueprintImplementableEvent, Category = "Stats")
	void OnShotTelegraphStarted(float Duration);

	// The wind-up is over, because the shot was fired or because it was interrupted. Hide the indicator.
	UFUNCTION(BlueprintImplementableEvent, Category = "Stats")
	void OnShotTelegraphEnded();

	virtual void NativeDestruct() override;

private:
	UFUNCTION() void HandleTelegraphStarted(float Duration);
	UFUNCTION() void HandleTelegraphEnded();

protected: 
	
	
};
