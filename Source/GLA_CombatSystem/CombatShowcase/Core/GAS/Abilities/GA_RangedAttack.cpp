// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_RangedAttack.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Characters/Player/PlayerCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Components/CombatComboComponent.h"
#include "CombatShowcase/Core/Combat/Components/CombatFeedbackComponent.h"
#include "CombatShowcase/Core/Combat/Components/WeaponComponent.h"
#include "CombatShowcase/Core/Combat/Data/PDT_Weapon.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"

static TAutoConsoleVariable<bool> CVarDrawShots(
	TEXT("Combat.DrawShots"), false,
	TEXT("Draw each bullet's path from the muzzle for a second."), ECVF_Cheat);

UGA_RangedAttack::UGA_RangedAttack()
{
	// The attack tags, the movement lock and the two attack triggers all come from the melee base.
	FAbilityTriggerData ReloadTrigger;
	ReloadTrigger.TriggerTag = CombatTags::Event_Ability_Reload;
	ReloadTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(ReloadTrigger);
}

bool UGA_RangedAttack::IsWeaponKindAllowed(const UWeaponComponent* Weapons) const
{
	return Weapons && Weapons->IsRangedDrawn();
}

FName UGA_RangedAttack::GetOpenerRow(bool bHeavy) const
{
	// Both attack buttons start the weapon's shot row. Combos branch from there through the row's own continuation names.
	const ABaseCombatCharacter* Avatar = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo());
	const UWeaponComponent* Weapons = Avatar ? Avatar->GetWeaponComponent() : nullptr;
	const FRangedWeaponSpec* Spec = Weapons ? Weapons->GetRangedSpec() : nullptr;
	return Spec ? Spec->ShotRow : NAME_None;
}

void UGA_RangedAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                       const FGameplayAbilityActivationInfo ActivationInfo,
                                       const FGameplayEventData* TriggerEventData)
{
	ABaseCombatCharacter* Avatar = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo());
	UWeaponComponent* Weapons = Avatar ? Avatar->GetWeaponComponent() : nullptr;
	const FRangedWeaponSpec* Spec = Weapons ? Weapons->GetRangedSpec() : nullptr;
	if (!Spec)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bShotFired = false;
	bTelegraphing = false;
	const float Magnitude = TriggerEventData ? TriggerEventData->EventMagnitude : 0.f;
	const bool bReloadRequested = TriggerEventData && TriggerEventData->EventTag == CombatTags::Event_Ability_Reload;

	// An empty magazine turns a shot request into a reload, so the attack button never goes dead.
	if (bReloadRequested || !Weapons->HasAmmo())
	{
		if (!HasValidAnimInstance(ActorInfo) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
		// A reload ends any combo in progress, so the next shot starts from the opener, not a stale continuation row.
		if (UCombatComboComponent* Stats = Avatar->GetCombatComboComponent()) Stats->ResetCombo();

		if (Weapons->IsMagazineFull())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false); // nothing to reload
			return;
		}
		StartReload(Magnitude > 0.f ? Magnitude : 1.f);
		return;
	}

	// The melee base resolves the row, commits and consumes stamina, then calls BeginStrike below.
	PendingTelegraph = FMath::Max(0.f, Magnitude);
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

// ---------- Shot ----------

void UGA_RangedAttack::BeginStrike(const FCombatStrikeRow& Row, const FCombatStrikePayload& Payload, float PlayRate,
                                   const FGameplayEventData* TriggerEventData)
{
	ShotRow = Row;
	ShotPayload = Payload;
	ShotPayload.bRanged = true;
	ShotPlayRate = PlayRate;

	ABaseCombatCharacter* Avatar = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo());

	// An aimed shot is one precise bullet. The base just cached this row's continuations, so cancel them.
	if (Avatar && Avatar->IsAiming())
	{
		if (UCombatComboComponent* Stats = Avatar->GetCombatComboComponent()) Stats->CacheContinuations(NAME_None, NAME_None);
	}

	if (PendingTelegraph <= KINDA_SMALL_NUMBER)
	{
		PlayShotMontage();
		return;
	}

	// The wind-up. The target's UI animates for exactly this long, so the player can see when the shot lands.
	// The ability is tagged as an attack, so an unblocked hit during the wind-up cancels the shot.
	const float Duration = PendingTelegraph;
	PendingTelegraph = 0.f;
	if (Avatar)
	{
		if (UCombatFeedbackComponent* Feedback = Avatar->GetFeedbackComponent()) Feedback->BeginTelegraph(Duration);
	}
	GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(CombatTags::State_Combat_Telegraphing);
	bTelegraphing = true;

	UAbilityTask_WaitDelay* Delay = UAbilityTask_WaitDelay::WaitDelay(this, Duration);
	Delay->OnFinish.AddDynamic(this, &UGA_RangedAttack::HandleTelegraphDone);
	Delay->ReadyForActivation();
}

void UGA_RangedAttack::HandleTelegraphDone()
{
	ClearTelegraph();
	PlayShotMontage();
}

void UGA_RangedAttack::PlayShotMontage()
{
	// The bullet leaves on the montage's Event.Combat.Fire notify, so the animator picks the frame.
	UAbilityTask_WaitGameplayEvent* FireTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, CombatTags::Event_Combat_Fire, nullptr, /*OnlyTriggerOnce=*/true);
	FireTask->EventReceived.AddDynamic(this, &UGA_RangedAttack::HandleFireEvent);
	FireTask->ReadyForActivation();

	PlayStrikeMontage(ShotRow.Montage, ShotPlayRate);
}

void UGA_RangedAttack::HandleFireEvent(FGameplayEventData Payload)
{
	FireShot();
}

void UGA_RangedAttack::HandleMontageCompleted()
{
	if (!bShotFired)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: the shot montage has no Event.Combat.Fire notify. Firing at the end."), *GetName());
		FireShot(); // never lose the shot because a notify is missing
	}
	Super::HandleMontageCompleted(); // resets the combo and ends the ability
}

FVector UGA_RangedAttack::ResolveShotDirection(const ABaseCombatCharacter& Avatar, const FRangedWeaponSpec& Spec) const
{
	// The lane is the world X axis, and the shooter faces one way along it.
	const float Facing = (Avatar.GetActorForwardVector().X >= 0.f) ? 1.f : -1.f;
	if (!Avatar.IsAiming())
	{
		return FVector(Facing, 0.f, 0.f);
	}

	// Aiming: the bullet follows where the weapon points. The muzzle socket's X axis has to run down the barrel.
	// A socket that doesn't face forward falls back to the aim angle the player set.
	float PitchDegrees = Avatar.GetAimPitch();
	const UMeshComponent* WeaponMesh = Avatar.GetEquippedWeaponMesh();
	if (WeaponMesh && WeaponMesh->DoesSocketExist(Spec.MuzzleSocket))
	{
		const FVector Barrel = WeaponMesh->GetSocketRotation(Spec.MuzzleSocket).Vector();
		const float Forward = Barrel.X * Facing;
		if (Forward > 0.1f)
		{
			PitchDegrees = FMath::RadiansToDegrees(FMath::Atan2(Barrel.Z, Forward));
		}
	}
	PitchDegrees = FMath::Clamp(PitchDegrees, -Spec.MaxAimAngle, Spec.MaxAimAngle);

	const float Radians = FMath::DegreesToRadians(PitchDegrees);
	return FVector(Facing * FMath::Cos(Radians), 0.f, FMath::Sin(Radians));
}

void UGA_RangedAttack::FireShot()
{
	if (bShotFired) return;
	bShotFired = true;
	ClearTelegraph();

	ABaseCombatCharacter* Avatar = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo());
	UWeaponComponent* Weapons = Avatar ? Avatar->GetWeaponComponent() : nullptr;
	const FRangedWeaponSpec* Spec = Weapons ? Weapons->GetRangedSpec() : nullptr;
	UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
	if (!Spec || !World || !Weapons->ConsumeAmmo()) return;

	const UMeshComponent* WeaponMesh = Avatar->GetEquippedWeaponMesh();
	const FVector Muzzle = (WeaponMesh && WeaponMesh->DoesSocketExist(Spec->MuzzleSocket))
		                       ? WeaponMesh->GetSocketLocation(Spec->MuzzleSocket)
		                       : Avatar->GetActorLocation() + FVector(0.f, 0.f, 60.f);

	const FVector Direction = ResolveShotDirection(*Avatar, *Spec);
	FVector End = Muzzle + Direction * Spec->Range;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(RangedShot), false, Avatar);

	// Level geometry stops the bullet. Only things that block visibility count, so trigger volumes don't.
	FCollisionObjectQueryParams WallObjects;
	WallObjects.AddObjectTypesToQuery(ECC_WorldStatic);
	WallObjects.AddObjectTypesToQuery(ECC_WorldDynamic);
	TArray<FHitResult> WallHits;
	World->LineTraceMultiByObjectType(WallHits, Muzzle, End, WallObjects, Params);
	for (const FHitResult& Wall : WallHits)
	{
		if (Wall.Component.IsValid() && Wall.Component->GetCollisionResponseToChannel(ECC_Visibility) == ECR_Block)
		{
			End = Wall.ImpactPoint;
			break;
		}
	}

	// Same hit channel the melee sweeps use, so characters already respond to it.
	TArray<FHitResult> Hits;
	World->SweepMultiByChannel(Hits, Muzzle, End, FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(12.f), Params);
	Hits.Sort([](const FHitResult& A, const FHitResult& B) { return A.Distance < B.Distance; });

	// The first character in the way takes the bullet. The same hit pipeline as a melee strike decides block, damage and reaction.
	for (const FHitResult& Hit : Hits)
	{
		ABaseCombatCharacter* Target = Cast<ABaseCombatCharacter>(Hit.GetActor());
		if (!Target || Target == Avatar || Target->GetHealthPercent() <= 0.f) continue;
		if (FGenericTeamId::GetAttitude(Avatar->GetGenericTeamId(), Target->GetGenericTeamId()) != ETeamAttitude::Hostile) continue;

		UCombatFeedbackComponent* TargetFeedback = Target->GetFeedbackComponent();
		if (!TargetFeedback) continue;

		const FHitResolution Resolution = TargetFeedback->ResolveAndApplyHit(Avatar, Hit, ShotPayload);
		if (Resolution.Outcome == EHitOutcome::Invincible) continue; // a rolling target is dodged: the bullet flies on

		End = Hit.ImpactPoint;
		if (Resolution.Outcome == EHitOutcome::Unblocked)
		{
			if (APlayerCombatCharacter* Player = Cast<APlayerCombatCharacter>(Avatar))
			{
				Player->GetCombatComboComponent()->RegisterHitLanded();
			}
		}
		break;
	}

	if (CVarDrawShots.GetValueOnGameThread())
	{
		DrawDebugLine(World, Muzzle, End, FColor::Red, false, 1.f, 0, 1.5f);
	}

	if (UNiagaraSystem* Tracer = Spec->TracerFX.LoadSynchronous())
	{
		if (UNiagaraComponent* Fx = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, Tracer, Muzzle))
		{
			Fx->SetVariablePosition(Spec->TracerEndParameter, End);
		}
	}
}

// ---------- Reload ----------

void UGA_RangedAttack::StartReload(float PlayRate)
{
	const ABaseCombatCharacter* Avatar = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo());
	UWeaponComponent* Weapons = Avatar ? Avatar->GetWeaponComponent() : nullptr;
	const FRangedWeaponSpec* Spec = Weapons ? Weapons->GetRangedSpec() : nullptr;
	if (!Spec)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
		return;
	}

	if (!Spec->ReloadMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: the weapon has no reload montage. Reloading instantly."), *GetName());
		Weapons->RefillAmmo();
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
		return;
	}

	// The magazine only fills when the montage finishes, so a hit that interrupts the reload leaves the weapon empty.
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, Spec->ReloadMontage, PlayRate);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_RangedAttack::HandleReloadCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_RangedAttack::HandleReloadInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_RangedAttack::HandleReloadInterrupted);
	MontageTask->ReadyForActivation();
}

void UGA_RangedAttack::HandleReloadCompleted()
{
	if (const ABaseCombatCharacter* Avatar = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UWeaponComponent* Weapons = Avatar->GetWeaponComponent()) Weapons->RefillAmmo();
	}
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGA_RangedAttack::HandleReloadInterrupted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}

// ---------- Cleanup ----------

void UGA_RangedAttack::ClearTelegraph()
{
	if (!bTelegraphing) return;
	bTelegraphing = false;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(CombatTags::State_Combat_Telegraphing);
	}
	if (const ABaseCombatCharacter* Avatar = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UCombatFeedbackComponent* Feedback = Avatar->GetFeedbackComponent()) Feedback->EndTelegraph();
	}
}

void UGA_RangedAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                  const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                                  bool bWasCancelled)
{
	// A cancelled wind-up (the shooter was hit) must also take the indicator off the screen.
	ClearTelegraph();
	PendingTelegraph = 0.f;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
