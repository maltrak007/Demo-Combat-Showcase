// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "PlayerCombatCharacter.generated.h"

class UCombatStatsComponent;

UCLASS()
class GLA_COMBATSYSTEM_API APlayerCombatCharacter : public ABaseCombatCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCombatCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void HandleMove(float AxisValue);
	void LightAttack();
	void HeavyAttack();
	void StartBlock();
	void StopBlock();
	void Finisher();
	void Roll();
	void SendCombatEvent(const FGameplayTag& EventTag);

	void TryAttack(const FGameplayTag& AttackEventTag);
	void TryConsumeComboRequest();

	void UpdateBlockDirection(float AxisValue);
	float GetLastRawBlockAxisValue() const { return LastRawBlockAxisValue; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	float LastRawBlockAxisValue = 0.f;
};
