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
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* BlockDirectionAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* EquipWeaponAction;
	// Hold to aim a drawn ranged weapon (a Digital bool action).
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* AimAction;
	// Moves the aim up and down while aiming (an Axis1D action: W/S, the stick, or the mouse with a scale).
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* AimPitchAction;
	
	UPROPERTY()
	TObjectPtr<APlayerCombatCharacter> ControlledCharacter;

	void HandleMoveInput(const FInputActionValue& Value);
	void HandleLightAttack(const FInputActionValue& Value);
	void HandleHeavyAttack(const FInputActionValue& Value);
	void HandleBlockStart(const FInputActionValue& Value);
	void HandleBlockStop(const FInputActionValue& Value);
	void HandleRoll(const FInputActionValue& Value);
	void HandleFinisher(const FInputActionValue&);
	void HandleBlockDirection(const FInputActionValue& Value);
	void HandleEquipWeapon(const FInputActionValue& Value);
	void HandleAimStart(const FInputActionValue& Value);
	void HandleAimStop(const FInputActionValue& Value);
	void HandleAimPitch(const FInputActionValue& Value);
};
