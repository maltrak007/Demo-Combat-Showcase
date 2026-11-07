// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerCombatCharacter.h"
#include "Engine/LocalPlayer.h"

void ACombatPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
}

void ACombatPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (auto* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACombatPlayerController::HandleMoveInput);
		EIC->BindAction(LightAttackAction, ETriggerEvent::Triggered, this, &ACombatPlayerController::HandleLightAttack);
		EIC->BindAction(HeavyAttackAction, ETriggerEvent::Triggered, this, &ACombatPlayerController::HandleHeavyAttack);
		EIC->BindAction(BlockAction, ETriggerEvent::Started, this, &ACombatPlayerController::HandleBlockStart);
		EIC->BindAction(BlockAction, ETriggerEvent::Completed, this, &ACombatPlayerController::HandleBlockStop);
		EIC->BindAction(RollAction, ETriggerEvent::Started, this, &ACombatPlayerController::HandleRoll);
		EIC->BindAction(FinisherAction, ETriggerEvent::Started, this, &ACombatPlayerController::HandleFinisher);
		EIC->BindAction(BlockDirectionAction, ETriggerEvent::Triggered, this, &ACombatPlayerController::HandleBlockDirection);
		EIC->BindAction(EquipWeaponAction, ETriggerEvent::Started, this, &ACombatPlayerController::HandleEquipWeapon);
		// Guarded, so the project still runs before the two aim input assets are created and assigned.
		if (AimAction)
		{
			EIC->BindAction(AimAction, ETriggerEvent::Started, this, &ACombatPlayerController::HandleAimStart);
			EIC->BindAction(AimAction, ETriggerEvent::Completed, this, &ACombatPlayerController::HandleAimStop);
		}
		if (AimPitchAction)
		{
			EIC->BindAction(AimPitchAction, ETriggerEvent::Triggered, this, &ACombatPlayerController::HandleAimPitch);
		}
	}
}

void ACombatPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ControlledCharacter = Cast<APlayerCombatCharacter>(InPawn); // cached once, not per input event
}

void ACombatPlayerController::HandleMoveInput(const FInputActionValue& Value)   { if (ControlledCharacter) ControlledCharacter->HandleMove(Value.Get<float>()); }
void ACombatPlayerController::HandleLightAttack(const FInputActionValue&)      { if (ControlledCharacter) ControlledCharacter->LightAttack(); }
void ACombatPlayerController::HandleHeavyAttack(const FInputActionValue&)      { if (ControlledCharacter) ControlledCharacter->HeavyAttack(); }
void ACombatPlayerController::HandleBlockStart(const FInputActionValue&)       { if (ControlledCharacter) ControlledCharacter->StartBlock(); }
void ACombatPlayerController::HandleBlockStop(const FInputActionValue&)        { if (ControlledCharacter) ControlledCharacter->StopBlock(); }
void ACombatPlayerController::HandleRoll(const FInputActionValue&)             { if (ControlledCharacter) ControlledCharacter->Roll(); }
void ACombatPlayerController::HandleFinisher(const FInputActionValue&)         { if (ControlledCharacter) ControlledCharacter->Finisher(); }
void ACombatPlayerController::HandleBlockDirection(const FInputActionValue& Value) { if (ControlledCharacter) ControlledCharacter->UpdateBlockDirection(Value.Get<float>()); }
void ACombatPlayerController::HandleAimStart(const FInputActionValue&)   { if (ControlledCharacter) ControlledCharacter->StartAim(); }
void ACombatPlayerController::HandleAimStop(const FInputActionValue&)    { if (ControlledCharacter) ControlledCharacter->StopAim(); }
void ACombatPlayerController::HandleAimPitch(const FInputActionValue& Value) { if (ControlledCharacter) ControlledCharacter->AddAimInput(Value.Get<float>()); }
void ACombatPlayerController::HandleEquipWeapon(const FInputActionValue&) {if (ControlledCharacter) {ControlledCharacter->EquipWeapon(); } }
