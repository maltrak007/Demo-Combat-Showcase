// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatFeedbackComponent.generated.h"

struct FCombatImpactEvent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GLA_COMBATSYSTEM_API UCombatFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatFeedbackComponent();

	void ReactToImpact(const FCombatImpactEvent& Event);
	
	FVector GetLastKnockbackVelocity() const { return LastKnockbackVelocity; }
	
	void TriggerDeathSlowMo();
	
protected:
	UPROPERTY(EditAnywhere, Category = "Feel|Death Slow-Mo")
	float SlowMoDilationFactor = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Feel|Death Slow-Mo")
	float SlowMoRealDuration = 1.2f;
	
private:
	void ApplyHitstop(float Duration);
	void ApplyKnockback(const FVector& Velocity);
	void TriggerCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale);
	void EndHitstop();
	void EndSlowMo();
	
	FTimerHandle SlowMoTimerHandle;
	FTimerHandle HitstopTimerHandle;
	
	FVector LastKnockbackVelocity = FVector::ZeroVector;
};
