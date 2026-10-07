// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatFeedbackComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "CombatStatsComponent.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Data/CombatData.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h" 


// Sets default values for this component's properties
UCombatFeedbackComponent::UCombatFeedbackComponent()
{

	PrimaryComponentTick.bCanEverTick = false;
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

EHitOutcome UCombatFeedbackComponent::ResolveIncomingHit(EStrikeDirection StrikeDirection) const
{
	IAbilitySystemInterface* ASCOwner = Cast<IAbilitySystemInterface>(GetOwner());
	UAbilitySystemComponent* ASC = ASCOwner ? ASCOwner->GetAbilitySystemComponent() : nullptr;
	if (!ASC) return EHitOutcome::Unblocked;
	
	if (ASC->HasMatchingGameplayTag(CombatTags::State_Combat_Invincible))
		return EHitOutcome::Invincible;

	if (!ASC->HasMatchingGameplayTag(CombatTags::Ability_Type_Block))
		return EHitOutcome::Unblocked;

	const FGameplayTag& GuardDirectionTag = (StrikeDirection == EStrikeDirection::Up)
		? CombatTags::State_Blocking_DirectionUp
		: CombatTags::State_Blocking_DirectionDown;

	if (!ASC->HasMatchingGameplayTag(GuardDirectionTag))
		return EHitOutcome::Unblocked;

	return ASC->HasMatchingGameplayTag(CombatTags::State_Combat_ParryWindowOpen)
		? EHitOutcome::Parried
		: EHitOutcome::Blocked;
}

FHitResolution UCombatFeedbackComponent::ResolveAndApplyHit(AActor* Instigator, const FHitResult& Hit,
	const FCombatStrikePayload& Payload)
{
	FHitResolution Resolution;
	Resolution.Outcome = ResolveIncomingHit(Payload.Direction);

	switch (Resolution.Outcome)
	{
	case EHitOutcome::Unblocked:
		Resolution.FinalDamage = Payload.Damage;
		Resolution.FinalStaminaDamage = 0.f;
		Resolution.bAppliesKnockback = true;
		break;
	case EHitOutcome::Blocked:
		Resolution.FinalDamage = 0.f;
		Resolution.FinalStaminaDamage = Payload.StaminaDamage;
		Resolution.bAppliesKnockback = false;
		break;
	case EHitOutcome::Parried:
		Resolution.FinalDamage = 0.f;
		// Negative = a restore. Flipped positive again below via -Resolution.FinalStaminaDamage,
		// same Add-op sign convention every other case uses.
		// TODO: revisit if stamina-restore upgrades get added later — flat *-1.5 won't scale with them.
		Resolution.FinalStaminaDamage = Payload.StaminaDamage * -1.5f;
		Resolution.bAppliesKnockback = false;
		break;
	case EHitOutcome::Invincible:
		Resolution.FinalDamage = 0.f;
		Resolution.FinalStaminaDamage = 0.f;
		Resolution.bAppliesKnockback = false;
		break;
	}

	FCombatImpactEvent Event;
	Event.Target = GetOwner();
	Event.Instigator = Instigator;
	Event.ImpactPoint = Hit.ImpactPoint;
	Event.ImpactFeel = Payload.ImpactFeel;
	Event.OutCome = Resolution;

	const float DirSign = (GetOwner()->GetActorLocation().X >= Instigator->GetActorLocation().X) ? 1.f : -1.f;
	Event.KnockbackVelocity = Resolution.bAppliesKnockback
		? FVector(DirSign, 0.f, 0.f).GetSafeNormal() * Payload.ImpactFeel.KnockbackMagnitude
		: FVector::ZeroVector;

	// Feel on both sides — same symmetric dispatch the Task used to do directly.
	ReactToImpact(Event);
	if (ABaseCombatCharacter* AttackerChar = Cast<ABaseCombatCharacter>(Instigator))
	{
		if (UCombatFeedbackComponent* AttackerFC = AttackerChar->GetFeedbackComponent())
		{
			AttackerFC->ReactToImpact(Event);
		}
	}

	// GE application needs the ATTACKER's ASC as source — fetched via Instigator, since this
	// component only reliably knows its own owner's ASC, never the attacker's directly.
	IAbilitySystemInterface* SourceInterface = Cast<IAbilitySystemInterface>(Instigator);
	UAbilitySystemComponent* SourceASC = SourceInterface ? SourceInterface->GetAbilitySystemComponent() : nullptr;
	IAbilitySystemInterface* TargetInterface = Cast<IAbilitySystemInterface>(GetOwner());
	UAbilitySystemComponent* TargetASC = TargetInterface ? TargetInterface->GetAbilitySystemComponent() : nullptr;

	if (!SourceASC || !TargetASC) return Resolution;

	switch (Resolution.Outcome)
	{
	case EHitOutcome::Unblocked:
		if (Payload.HPEffectClass)
		{
			FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
			Context.AddInstigator(Instigator, Instigator);
			Context.AddHitResult(Hit);
			FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(Payload.HPEffectClass, 1.f, Context);
			if (Spec.IsValid())
			{
				Spec.Data->SetSetByCallerMagnitude(CombatTags::Data_Damage, -Resolution.FinalDamage);
				SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
			}
		}
		break;

	case EHitOutcome::Blocked:
	case EHitOutcome::Parried:
		// Both drain or restore through the same stamina effect — only the sign and
		// magnitude baked into Resolution.FinalStaminaDamage differ between them.
		if (Payload.STAEffectClass)
		{
			FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
			Context.AddInstigator(Instigator, Instigator);
			FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(Payload.STAEffectClass, 1.f, Context);
			if (Spec.IsValid())
			{
				Spec.Data->SetSetByCallerMagnitude(CombatTags::Data_StaminaConsumed, -Resolution.FinalStaminaDamage);
				SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
			}
		}
		break;

	case EHitOutcome::Invincible:
		break;
	}
	
	DispatchHitReaction(Instigator, Resolution.Outcome, Payload.ReactionDirection);
	return Resolution;
}

void UCombatFeedbackComponent::DispatchHitReaction(AActor* Instigator, EHitOutcome Outcome, EHitReactionDirection ReactionDirection)
{
	AActor* Owner = GetOwner();

	switch (Outcome)
	{
	case EHitOutcome::Unblocked:
		SendReaction(Owner, Instigator, CombatTags::Event_Combat_HitReact_Unblocked,
			static_cast<float>(static_cast<uint8>(ReactionDirection)), true, false);
		break;

	case EHitOutcome::Blocked:
		{
			UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
			const bool bGuardBroke = ASC && !ASC->HasMatchingGameplayTag(CombatTags::Ability_Type_Block);
			SendReaction(Owner, Instigator,
				bGuardBroke ? CombatTags::Event_Combat_HitReact_GuardBreak : CombatTags::Event_Combat_HitReact_Blocked,
				0.f, bGuardBroke, false);
			break;
		}

	case EHitOutcome::Parried:
		SendReaction(Instigator, Owner, CombatTags::Event_Combat_HitReact_Parried, 0.f, true, false);
		if (ABaseCombatCharacter* Parried = Cast<ABaseCombatCharacter>(Instigator))
		{
			if (UCombatStatsComponent* Stats = Parried->GetCombatStatsComponent()) Stats->ResetCombo();
		}
		SendReaction(Owner, Instigator, CombatTags::Event_Combat_HitReact_ParrySuccess, 0.f, false, true);
		break;

	case EHitOutcome::Invincible:
		break;
	}
}

void UCombatFeedbackComponent::ApplyHitstop(float Duration)
{
	if (Duration <= 0.f) return;
	GetOwner()->CustomTimeDilation = 0.05f;
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

void UCombatFeedbackComponent::SendReaction(AActor* ReactingActor, AActor* OtherParty, const FGameplayTag& Tag,
	float Magnitude, bool bCancelCombatAbilities, bool bCancelBlock)
{
	if (!ReactingActor) return;

	if (ABaseCombatCharacter* ReactingChar = Cast<ABaseCombatCharacter>(ReactingActor))
	{
		if (ReactingChar->GetHealthPercent() <= 0.f) return; 
	}

	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ReactingActor))
	{
		FGameplayTagContainer CancelTags;
		CancelTags.AddTag(CombatTags::Ability_Type_HitReaction); 
		if (bCancelCombatAbilities)
		{
			CancelTags.AddTag(CombatTags::Ability_Type_Attack);
			CancelTags.AddTag(CombatTags::Ability_Type_Evasion);
		}
		if (bCancelBlock) CancelTags.AddTag(CombatTags::Ability_Type_Block);
		ASC->CancelAbilities(&CancelTags);
	}

	FGameplayEventData Data;
	Data.EventTag = Tag;
	Data.EventMagnitude = Magnitude;
	Data.Instigator = OtherParty; 
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(ReactingActor, Tag, Data);
}
