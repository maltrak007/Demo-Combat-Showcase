// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "CombatShowcase/Core/Combat/Data/CombatData.h"
#include "AbilityTask_MultiBoneHitTrace.generated.h"

struct FCombatImpactFeel;
struct FCombatHitboxDef;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiBoneHitTraceDelegate, AActor*, HitActor);
/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UAbilityTask_MultiBoneHitTrace : public UAbilityTask
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable) 
	FMultiBoneHitTraceDelegate OnMultiHitDetected;

	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UAbilityTask_MultiBoneHitTrace* CreateMultiBoneHitTrace(UGameplayAbility* OwningAbility, const TArray<FCombatHitboxDef>& InHitboxes, TSubclassOf<UGameplayEffect> DamageEffectClass, float DamageAmount, FCombatImpactFeel ImpactFeel);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

protected:
	TArray<FCombatHitboxDef> Hitboxes;
	TSubclassOf<UGameplayEffect> EffectClass;
	float Damage = 10.f;
	bool bWindowOpen = false;
	FCombatImpactFeel ImpactFeel;

	TMap<FName, FVector> LastBonePositions;
	TMap<uint16, TSet<TWeakObjectPtr<AActor>>> HitGroupsConsumed;
	FDelegateHandle OpenHandle, CloseHandle;

	void HandleHitboxOpen(const FGameplayEventData* Payload);
	void HandleHitboxClose(const FGameplayEventData* Payload);
	void PerformTrace();
	bool ResolveHitboxLocation(const FCombatHitboxDef& Box, AActor* Avatar, FVector& OutLocation) const;
	void ApplyDamage(AActor* Target, const FHitResult& Hit);
};
