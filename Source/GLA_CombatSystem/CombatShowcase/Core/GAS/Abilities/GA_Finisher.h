// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatShowcase/Core/GAS/CombatGameplayAbility.h"
#include "GA_Finisher.generated.h"

class UContextualAnimSceneActorComponent;
class UContextualAnimSceneAsset;
class ABaseCombatCharacter;
class UGameplayEffect;
/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UGA_Finisher : public UCombatGameplayAbility
{
	GENERATED_BODY()
	public:
	UGA_Finisher();

protected:
	// Front executions only, so a flat list is enough.
	UPROPERTY(EditDefaultsOnly, Category = "Finisher") TArray<TObjectPtr<UContextualAnimSceneAsset>> ExecutionScenes;
	UPROPERTY(EditDefaultsOnly, Category = "Finisher") FName AttackerRole = "Attacker"; // must match the role names in the scene asset
	UPROPERTY(EditDefaultsOnly, Category = "Finisher") FName VictimRole = "Victim";
	UPROPERTY(EditDefaultsOnly, Category = "Finisher") TSubclassOf<UGameplayEffect> DamageEffectClass;
	// Watchdog in case the scene's end callback never arrives.
	UPROPERTY(EditDefaultsOnly, Category = "Finisher") float MaxSceneDuration = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = "Finisher|Targeting") float Range = 180.f;        // along the lane (X)
	UPROPERTY(EditDefaultsOnly, Category = "Finisher|Targeting") float LaneTolerance = 60.f; // off the lane (Y)
	UPROPERTY(EditDefaultsOnly, Category = "Finisher|Targeting") float ScanInterval = 0.1f;

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION() void HandleSceneLeft(UContextualAnimSceneActorComponent* SceneActorComponent); // VERIFY the delegate signature
	UFUNCTION() void HandleSceneTimeout();

private:
	// The one definition of "who can be executed right now". The prompt scan and the press both use it.
	ABaseCombatCharacter* FindTarget(const AActor* Owner) const;
	void ScanForPrompt();
	void SetCanExecuteTag(bool bShouldHold);

	UContextualAnimSceneAsset* PickScene();
	void ProtectVictim(ABaseCombatCharacter* InVictim);
	void ReleaseVictim();
	void KillVictim(ABaseCombatCharacter* InVictim);

	FTimerHandle ScanHandle;
	TWeakObjectPtr<ABaseCombatCharacter> ScanOwner;
	bool bCanExecuteTagHeld = false;

	TWeakObjectPtr<ABaseCombatCharacter> Victim;
	bool bVictimProtected = false;
	bool bPlayerInvincibleHeld = false;
	bool bSceneStarted = false;
	int32 LastSceneIndex = INDEX_NONE;
};
