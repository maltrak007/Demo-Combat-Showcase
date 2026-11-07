// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatShowcase/Core/Combat/Data/CombatData.h"
#include "Components/ActorComponent.h"
#include "CombatFeedbackComponent.generated.h"

class UCombatFeedbackProfile;
class UNiagaraSystem;
struct FGameplayTag;
class UGameplayEffect;
struct FCombatImpactEvent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTelegraphStarted, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTelegraphEnded);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GLA_COMBATSYSTEM_API UCombatFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatFeedbackComponent();

	void ReactToImpact(const FCombatImpactEvent& Event);

	EHitOutcome ResolveIncomingHit(EStrikeDirection StrikeDirection) const;

	FHitResolution ResolveAndApplyHit(AActor* Instigator, const FHitResult& Hit, const FCombatStrikePayload& Payload);

	void DispatchHitReaction(AActor* Instigator, EHitOutcome Outcome, EHitReactionDirection ReactionDirection);

	FVector GetLastKnockbackVelocity() const { return LastKnockbackVelocity; }

	void TriggerDeathSlowMo();
	
	void PlayWhiff(const FGameplayTag& WeaponTag, const FVector& Location);

	// The wind-up the player is meant to read, such as an enemy about to fire. The indicator animates for Duration seconds.
	void BeginTelegraph(float Duration);
	void EndTelegraph();

	UPROPERTY(BlueprintAssignable, Category = "Feedback") FOnTelegraphStarted OnTelegraphStarted;
	UPROPERTY(BlueprintAssignable, Category = "Feedback") FOnTelegraphEnded OnTelegraphEnded;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Feel|Death Slow-Mo")
	float SlowMoDilationFactor = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Feel|Death Slow-Mo")
	float SlowMoRealDuration = 1.2f;

	UPROPERTY(EditAnywhere, Category = "Feel|Feedback")
	TObjectPtr<UCombatFeedbackProfile> ProfileOverride;

private:
	void ApplyHitstop(float Duration);

	void ApplyKnockback(const FVector& Velocity);

	void TriggerCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale);

	void EndHitstop();

	void EndSlowMo();

	void SendReaction(AActor* ReactingActor, AActor* OtherParty, const FGameplayTag& Tag, float Magnitude,
	                  bool bCancelCombatAbilities, bool bCancelBlock);

	void PlayFeedback(EHitOutcome FeedbackEvent, const FGameplayTag& WeaponTag, const FVector& Location, uint8 Surface);

	void ApplyOutcomeOverlay(EHitOutcome FeedbackEvent);
	void ClearOutcomeOverlay();

	FTimerHandle SlowMoTimerHandle;
	FTimerHandle HitstopTimerHandle;
	FTimerHandle OverlayTimer;
	FVector LastKnockbackVelocity = FVector::ZeroVector;


	UPROPERTY(Transient)
	TObjectPtr<UCombatFeedbackProfile> Profile;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> OverlayMID;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> SavedOverlay;
	TWeakObjectPtr<UMeshComponent> OverlayMesh;
};
