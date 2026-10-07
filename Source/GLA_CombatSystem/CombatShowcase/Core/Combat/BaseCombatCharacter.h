// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Data/CombatData.h"
#include "GameFramework/Character.h"
#include "BaseCombatCharacter.generated.h"

class UContextualAnimSceneActorComponent;
class UMotionWarpingComponent;
class UWidgetComponent;
class UCombatStatsComponent;
class UCombatFeedbackComponent;
class UGameplayEffect;
class UGameplayAbility;
class UCombatAttributeSet;

UCLASS()
class GLA_COMBATSYSTEM_API ABaseCombatCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCombatCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	UCombatAttributeSet* GetAttributeSet() const { return AttributeSet; }

	// UI FUNCTIONS & STATES
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	float GetStaminaPercent() const;

	bool IsMovementLocked() const;
	bool IsExecutable() const;

	// STAMINA RELATED FUNCTIONS 
	void ConsumeStamina(float Amount);

	void GrantStaminaBurst(float Amount);

	// WEAPON RELATED
	UStaticMeshComponent* GetEquippedWeaponMesh() const { return EquippedWeaponMesh; }

	void SetEquippedWeaponMesh(UStaticMeshComponent* NewWeaponMesh) { EquippedWeaponMesh = NewWeaponMesh; }

	UCombatFeedbackComponent* GetFeedbackComponent() const { return FeedbackComponent; }

	UCombatStatsComponent* GetCombatStatsComponent() const { return CombatStatsComponent; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UCombatAttributeSet* AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TArray<TSubclassOf<UGameplayAbility>> StartingAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> DefaultAttributesEffect;

	// Set by whatever equips a weapon later. Null = bare-handed. Not building the equip
	// system itself here — this is just the seam it plugs into.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UStaticMeshComponent* EquippedWeaponMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UCombatFeedbackComponent* FeedbackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatStatsComponent> CombatStatsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UWidgetComponent> StatWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UContextualAnimSceneActorComponent> ContextualAnimSceneActorComponent;

	virtual void PossessedBy(AController* NewController) override;

	void FaceDirection(float SignedDirection);

	UFUNCTION()
	void HandleHealthChanged(float NewHealth, float DamageAmount, bool bIsDead);

	void TriggerDeath();

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float DeathImpulseScale = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Execution", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ExecutionThresholdPercent = 0.f;

	// STAMINA SECTION
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina")
	TSubclassOf<UGameplayEffect> StaminaCostEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina")
	TSubclassOf<UGameplayEffect> StaminaRegenEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina")
	TSubclassOf<UGameplayEffect> StaminaBurstEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina")
	float StaminaRegenDelayNormal = 1.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina")
	float StaminaRegenDelayExhausted = 3.f;

private:
	void InitializeAttributes();

	void GrantStartingAbilities();

	FTimerHandle StaminaRegenDelayHandle;
	void ResumeStaminaRegen();

	void UpdateExecutableState();
	bool bExecutableTagHeld = false;
};
