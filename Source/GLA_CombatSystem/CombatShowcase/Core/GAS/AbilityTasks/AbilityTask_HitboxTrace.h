// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_HitboxTrace.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHitboxTraceHitDelegate, AActor*, HitActor);
/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UAbilityTask_HitboxTrace : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FHitboxTraceHitDelegate OnHitDetected;

	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UAbilityTask_HitboxTrace* CreateHitboxTrace(UGameplayAbility* OwningAbility, FName StartSocket, FName EndSocket, float TraceRadius, TSubclassOf<UGameplayEffect> DamageEffectClass, float DamageAmount);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

protected:
	FName SocketStart, SocketEnd;
	float Radius = 20.f;
	float Damage = 10.f;
	TSubclassOf<UGameplayEffect> EffectClass;
	bool bWindowOpen = false;
	TSet<TWeakObjectPtr<AActor>> AlreadyHitActors;
	FDelegateHandle OpenHandle, CloseHandle;

	void HandleHitboxOpen(const FGameplayEventData* Payload);
	void HandleHitboxClose(const FGameplayEventData* Payload);
	void PerformTrace();
};
