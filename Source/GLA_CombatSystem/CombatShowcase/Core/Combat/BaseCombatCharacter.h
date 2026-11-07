// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "Data/CombatData.h"
#include "GameFramework/Character.h"
#include "BaseCombatCharacter.generated.h"

class UWeaponComponent;
class UContextualAnimSceneActorComponent;
class UMotionWarpingComponent;
class UWidgetComponent;
class UCombatComboComponent;
class UCombatFeedbackComponent;
class UGameplayEffect;
class UGameplayAbility;
class UCombatAttributeSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAimingChanged, bool, bAiming);

UCLASS()
class GLA_COMBATSYSTEM_API ABaseCombatCharacter : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
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

	void FaceDirection(float SignedDirection);
	bool IsMovementLocked() const;
	bool IsExecutable() const;

	// WEAPON RELATED
	UMeshComponent* GetEquippedWeaponMesh() const { return EquippedWeaponMesh; }
	void SetEquippedWeaponMesh(UMeshComponent* NewWeaponMesh) { EquippedWeaponMesh = NewWeaponMesh; }
	
	// STAMINA RELATED FUNCTIONS 
	void ConsumeStamina(float Amount);

	void GrantStaminaBurst(float Amount);
	
	UCombatFeedbackComponent* GetFeedbackComponent() const { return FeedbackComponent; }

	UCombatComboComponent* GetCombatComboComponent() const { return CombatComboComponent; }
	
	UWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }

	// AIMING with a ranged weapon. GA_Aim owns the state, and the shot ability and the animation blueprint read it here.
	UFUNCTION(BlueprintPure, Category = "Combat|Aim")
	bool IsAiming() const { return bAiming; }

	// Degrees above (positive) or below (negative) the horizontal. Zero when not aiming.
	UFUNCTION(BlueprintPure, Category = "Combat|Aim")
	float GetAimPitch() const { return AimPitchDegrees; }

	void SetAiming(bool bNewAiming);
	void SetAimPitch(float NewPitchDegrees) { AimPitchDegrees = NewPitchDegrees; }

	// Fires when aiming starts or ends. The animation blueprint binds it to switch to the aim pose.
	UPROPERTY(BlueprintAssignable, Category = "Combat|Aim")
	FOnAimingChanged OnAimingChanged;
	
	virtual FGenericTeamId GetGenericTeamId() const override;
	
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;
	
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatFeedbackComponent> FeedbackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatComboComponent> CombatComboComponent;	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UWeaponComponent> WeaponComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UWidgetComponent> StatWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UContextualAnimSceneActorComponent> ContextualAnimSceneActorComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|AI")
	FGenericTeamId TeamId;
	
	virtual void PossessedBy(AController* NewController) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UMeshComponent> EquippedWeaponMesh = nullptr;
	
	UFUNCTION()
	void HandleHealthChanged(float NewHealth, float DamageAmount, bool bIsDead);

	virtual void TriggerDeath();

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
	float StaminaRegenDelayNormal = 2.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina")
	float StaminaRegenDelayExhausted = 4.f;

private:
	void InitializeAttributes();

	void GrantStartingAbilities();

	FTimerHandle StaminaRegenDelayHandle;
	void ResumeStaminaRegen();

	void UpdateExecutableState();
	bool bExecutableTagHeld = false;

	bool bAiming = false;
	float AimPitchDegrees = 0.f;
};
