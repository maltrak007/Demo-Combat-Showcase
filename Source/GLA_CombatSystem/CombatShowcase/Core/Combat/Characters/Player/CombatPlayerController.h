// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CombatPlayerController.generated.h"

class APlayerCombatCharacter;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API ACombatPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* MoveAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* LightAttackAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* HeavyAttackAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* BlockAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* RollAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* FinisherAction;

	UPROPERTY()
	TObjectPtr<APlayerCombatCharacter> ControlledCharacter;

	void HandleMoveInput(const FInputActionValue& Value);
	void HandleLightAttack(const FInputActionValue& Value);
	void HandleHeavyAttack(const FInputActionValue& Value);
	void HandleBlockStart(const FInputActionValue& Value);
	void HandleBlockStop(const FInputActionValue& Value);
	void HandleRoll(const FInputActionValue& Value);
	void HandleFinisher(const FInputActionValue&);
};
