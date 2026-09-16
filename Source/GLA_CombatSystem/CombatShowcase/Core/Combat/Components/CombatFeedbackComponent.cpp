// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatFeedbackComponent.h"
#include "CombatShowcase/Core/Combat/Data/CombatData.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UCombatFeedbackComponent::UCombatFeedbackComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UCombatFeedbackComponent::ReactToImpact(const FCombatImpactEvent& Event)
{
	ApplyHitstop(Event.ImpactFeel.HitstopDuration);
	if (Event.Target == GetOwner()) 
	{
		LastKnockbackVelocity = Event.KnockbackVelocity;
		ApplyKnockback(Event.KnockbackVelocity);
	}
	TriggerCameraShake(Event.ImpactFeel.ImpactShake, Event.ImpactFeel.ShakeScale);
}

void UCombatFeedbackComponent::ApplyHitstop(float Duration)
{
	if (Duration <= 0.f) return;
	GetOwner()->CustomTimeDilation = 0.05f; // near-frozen, not exactly 0 to avoid animation edge cases
	GetWorld()->GetTimerManager().SetTimer(HitstopTimerHandle, this, &UCombatFeedbackComponent::EndHitstop, Duration, false);
	// Timer runs on unscaled world time — it isn't itself slowed by the dilation it just applied
}

void UCombatFeedbackComponent::EndHitstop()
{
	GetOwner()->CustomTimeDilation = 1.f;
}

void UCombatFeedbackComponent::ApplyKnockback(const FVector& Velocity)
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->LaunchCharacter(Velocity, true, true);
	}
}

void UCombatFeedbackComponent::TriggerCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale)
{
	if (!ShakeClass) return;

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) return;

	if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
	{
		PC->ClientStartCameraShake(ShakeClass, Scale);
	}
}

void UCombatFeedbackComponent::TriggerDeathSlowMo()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::SetGlobalTimeDilation(World, SlowMoDilationFactor);
	// Scaled by 1/DilationFactor — the timer itself runs at the dilated rate too,
	// so without this it would take far longer than SlowMoRealDuration to fire.
	const float AdjustedTimerDuration = SlowMoRealDuration * SlowMoDilationFactor;
	World->GetTimerManager().SetTimer(SlowMoTimerHandle, this, &UCombatFeedbackComponent::EndSlowMo, AdjustedTimerDuration, false);
}

void UCombatFeedbackComponent::EndSlowMo()
{
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGlobalTimeDilation(World, 1.f);
	}
}