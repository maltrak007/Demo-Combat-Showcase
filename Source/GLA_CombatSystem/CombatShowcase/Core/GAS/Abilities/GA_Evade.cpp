// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Evade.h"

#include "MotionWarpingComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/GAS/CombatAttributeSet.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "Components/CapsuleComponent.h"

UGA_Evade::UGA_Evade()
{
	AbilityTags.AddTag(CombatTags::Ability_Type_Evasion);
	ActivationOwnedTags.AddTag(CombatTags::State_Combat_MovementLocked);
	
	FAbilityTriggerData BackStepTrigger;
	BackStepTrigger.TriggerTag = CombatTags::Event_Ability_BackStep; 
	BackStepTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(BackStepTrigger);

	FAbilityTriggerData RollTrigger;
	RollTrigger.TriggerTag = CombatTags::Event_Ability_Roll; 
	RollTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(RollTrigger);
}

void UGA_Evade::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ABaseCombatCharacter* Avatar = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo());
	if (!Avatar || Avatar->GetAttributeSet()->GetStamina() < MinimumStaminaToActivate)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!HasValidAnimInstance(ActorInfo) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	const bool bIsRoll = TriggerEventData && TriggerEventData->EventTag == CombatTags::Event_Ability_Roll;
	UAnimMontage* Montage = bIsRoll ? RollMontage : BackStepMontage;
	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: no montage assigned for %s."), *GetName(), bIsRoll ? TEXT("Roll") : TEXT("BackStep"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (!HasValidAnimInstance(ActorInfo) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	Avatar->ConsumeStamina(EvadeStaminaCost);

	UAbilitySystemComponent* ASC = Avatar->GetAbilitySystemComponent();
	
	
	// Leaving the guard is part of evading. Block cleans up its own tags in its EndAbility.
	FGameplayTagContainer BlockTags;
	BlockTags.AddTag(CombatTags::Ability_Type_Block);
	ASC->CancelAbilities(&BlockTags, nullptr, this);

	// Roll goes the way the character already faces (Roll() faced the input direction),
	// back-step goes the opposite way. The lane is the world X axis.
	const float Facing = (Avatar->GetActorForwardVector().X >= 0.f) ? 1.f : -1.f;
	const float Direction = bIsRoll ? Facing : -Facing;
	const float Distance = bIsRoll ? RollDistance : BackStepDistance;
	const bool bPassThroughPawns = bIsRoll && bRollPassesThroughPawns;

	const float ClearDistance = ComputeClearDistance(Avatar, Direction, Distance, bPassThroughPawns);
	FVector Target = Avatar->GetActorLocation();
	Target.X += Direction * ClearDistance;

	if (UMotionWarpingComponent* Warp = Avatar->FindComponentByClass<UMotionWarpingComponent>())
	{
		Warp->AddOrUpdateWarpTargetFromLocation(EvadeWarpTargetName, Target);
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: %s has no MotionWarpingComponent. The evade will play unwarped."), *GetName(), *Avatar->GetName());
	}

	if (bPassThroughPawns)
	{
		UCapsuleComponent* Capsule = Avatar->GetCapsuleComponent();
		SavedPawnResponse = Capsule->GetCollisionResponseToChannel(ECC_Pawn);
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		bPawnCollisionOverridden = true;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_Evade::HandleMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Evade::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Evade::HandleMontageInterrupted);
	MontageTask->ReadyForActivation();
}

float UGA_Evade::ComputeClearDistance(ABaseCombatCharacter* Avatar, float Direction, float DesiredDistance, bool bIgnorePawns) const
{
	UWorld* World = Avatar->GetWorld();
	UCapsuleComponent* Capsule = Avatar->GetCapsuleComponent();
	if (!World || !Capsule) return DesiredDistance;

	// Lifted a few cm so the floor under the capsule never registers as an obstacle.
	// Anything lower than this is stepped over by the movement component anyway.
	constexpr float SweepLift = 5.f;
	const FVector Start = Avatar->GetActorLocation() + FVector(0.f, 0.f, SweepLift);
	const FVector End = Start + FVector(Direction, 0.f, 0.f) * DesiredDistance;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	if (!bIgnorePawns) ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EvadeSweep), false, Avatar);
	const FCollisionShape Shape = FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight());

	TArray<FHitResult> Hits;
	World->SweepMultiByObjectType(Hits, Start, End, FQuat::Identity, ObjectParams, Shape, QueryParams);

	const ECollisionChannel MyObjectType = Capsule->GetCollisionObjectType();
	for (const FHitResult& Hit : Hits)
	{
		if (Hit.bStartPenetrating || !Hit.Component.IsValid()) continue;
		// Object-type queries also report overlap-only volumes. Only things that block us count.
		if (Hit.Component->GetCollisionResponseToChannel(MyObjectType) != ECR_Block) continue;
		return FMath::Max(0.f, Hit.Time * DesiredDistance - WallMargin);
	}
	return DesiredDistance;
}

void UGA_Evade::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ABaseCombatCharacter* Avatar = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UAbilitySystemComponent* ASC = Avatar->GetAbilitySystemComponent())
		{
			// Backstop for the i-frame notify: if the montage is cut off before its NotifyEnd,
			// the loose tag would stay raised and the character would stay invincible.
			ASC->SetLooseGameplayTagCount(CombatTags::State_Combat_Invincible, 0);
		}
		if (bPawnCollisionOverridden)
		{
			Avatar->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, SavedPawnResponse);
			bPawnCollisionOverridden = false;
		}
		if (UMotionWarpingComponent* Warp = Avatar->FindComponentByClass<UMotionWarpingComponent>())
		{
			Warp->RemoveWarpTarget(EvadeWarpTargetName);
		}
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Evade::HandleMontageCompleted() { EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false); }
void UGA_Evade::HandleMontageInterrupted() { EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true); }