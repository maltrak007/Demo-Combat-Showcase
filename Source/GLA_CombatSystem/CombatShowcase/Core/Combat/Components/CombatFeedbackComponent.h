// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatShowcase/Core/Combat/Data/CombatData.h"
#include "Components/ActorComponent.h"
#include "CombatFeedbackComponent.generated.h"

struct FGameplayTag;
class UGameplayEffect;
struct FCombatImpactEvent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GLA_COMBATSYSTEM_API UCombatFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatFeedbackComponent();

	void ReactToImpact(const FCombatImpactEvent& Event);
	
	EHitOutcome ResolveIncomingHit(EStrikeDirection StrikeDirection) const;
	
	FHitResolution ResolveAndApplyHit(AActor* Instigator, const FHitResult& Hit, const FCombatStrikePayload& Payload);
	
	void DispatchHitReaction(AActor* Instigator, EHitOutcome Outcome, EHitReactionDirection ReactionDirection);

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
	void SendReaction(AActor* ReactingActor, AActor* OtherParty, const FGameplayTag& Tag, float Magnitude, bool bCancelCombatAbilities, bool bCancelBlock);
	
	FTimerHandle SlowMoTimerHandle;
	FTimerHandle HitstopTimerHandle;
	
	FVector LastKnockbackVelocity = FVector::ZeroVector;
};
