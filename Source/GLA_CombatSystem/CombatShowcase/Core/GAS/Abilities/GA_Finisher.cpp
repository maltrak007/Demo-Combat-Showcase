// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Finisher.h"

#include "AbilitySystemComponent.h"
#include "ContextualAnimSceneActorComponent.h"
#include "ContextualAnimSceneAsset.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Components/CombatFeedbackComponent.h"
#include "CombatShowcase/Core/Combat/Components/CombatStatsComponent.h"
#include "CombatShowcase/Core/GAS/CombatAttributeSet.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "Engine/OverlapResult.h"


UGA_Finisher::UGA_Finisher()
{
	AbilityTags.AddTag(CombatTags::Ability_Type_Finisher);
	ActivationOwnedTags.AddTag(CombatTags::State_Combat_MovementLocked);

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = CombatTags::Event_Ability_Finisher;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

// ---------- Targeting and prompt ----------

ABaseCombatCharacter* UGA_Finisher::FindTarget(const AActor* Owner) const
{
	UWorld* World = Owner ? Owner->GetWorld() : nullptr;
	if (!World) return nullptr;

	const FVector Origin = Owner->GetActorLocation();
	const float Facing = (Owner->GetActorForwardVector().X >= 0.f) ? 1.f : -1.f;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(FinisherTarget), false, Owner);
	World->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Range + LaneTolerance), Params);

	ABaseCombatCharacter* Best = nullptr;
	float BestDistance = TNumericLimits<float>::Max();
	for (const FOverlapResult& Overlap : Overlaps)
	{
		ABaseCombatCharacter* Candidate = Cast<ABaseCombatCharacter>(Overlap.GetActor());
		if (!Candidate || Candidate == Owner || !Candidate->IsExecutable()) continue;

		const FVector Delta = Candidate->GetActorLocation() - Origin;
		if (Delta.X * Facing <= 0.f) continue; // front executions only
		const float Distance = FMath::Abs(Delta.X);
		if (Distance > Range || FMath::Abs(Delta.Y) > LaneTolerance) continue;

		if (Distance < BestDistance) { BestDistance = Distance; Best = Candidate; }
	}
	return Best;
}

void UGA_Finisher::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	ABaseCombatCharacter* Owner = Cast<ABaseCombatCharacter>(ActorInfo->AvatarActor.Get());
	if (!Owner || !Owner->GetWorld()) return;

	ScanOwner = Owner;
	Owner->GetWorld()->GetTimerManager().SetTimer(ScanHandle, this, &UGA_Finisher::ScanForPrompt, ScanInterval, true);
}

void UGA_Finisher::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if (ABaseCombatCharacter* Owner = ScanOwner.Get())
	{
		if (UWorld* World = Owner->GetWorld()) World->GetTimerManager().ClearTimer(ScanHandle);
	}
	SetCanExecuteTag(false);
	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UGA_Finisher::ScanForPrompt()
{
	ABaseCombatCharacter* Owner = ScanOwner.Get();
	SetCanExecuteTag(Owner && FindTarget(Owner) != nullptr);
}

void UGA_Finisher::SetCanExecuteTag(bool bShouldHold)
{
	ABaseCombatCharacter* Owner = ScanOwner.Get();
	if (!Owner || bShouldHold == bCanExecuteTagHeld) return;

	UAbilitySystemComponent* ASC = Owner->GetAbilitySystemComponent();
	if (bShouldHold) ASC->AddLooseGameplayTag(CombatTags::State_Combat_Executable);
	else ASC->RemoveLooseGameplayTag(CombatTags::State_Combat_Executable);
	bCanExecuteTagHeld = bShouldHold;
}

// ---------- Execution ----------

UContextualAnimSceneAsset* UGA_Finisher::PickScene()
{
	TArray<int32> Candidates;
	for (int32 i = 0; i < ExecutionScenes.Num(); ++i)
		if (ExecutionScenes[i]) Candidates.Add(i);
	if (Candidates.IsEmpty()) return nullptr;

	if (Candidates.Num() > 1) Candidates.Remove(LastSceneIndex); // never the same execution twice in a row
	LastSceneIndex = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
	return ExecutionScenes[LastSceneIndex];
}

void UGA_Finisher::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ABaseCombatCharacter* Player = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo());

	// Re-run here instead of trusting the prompt: the target can die or move between prompt and press.
	ABaseCombatCharacter* Target = Player ? FindTarget(Player) : nullptr;
	UContextualAnimSceneAsset* Scene = PickScene();
	UContextualAnimSceneActorComponent* PlayerComp = Player ? Player->FindComponentByClass<UContextualAnimSceneActorComponent>() : nullptr;

	if (!Target || !Scene || !PlayerComp || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();

	// The finisher takes over the player: drop whatever was running and any pending combo state.
	FGameplayTagContainer CancelTags;
	CancelTags.AddTag(CombatTags::Ability_Type_Block);
	CancelTags.AddTag(CombatTags::Ability_Type_Attack);
	CancelTags.AddTag(CombatTags::Ability_Type_Evasion);
	CancelTags.AddTag(CombatTags::Ability_Type_HitReaction);
	ASC->CancelAbilities(&CancelTags, nullptr, this);
	if (UCombatStatsComponent* Stats = Player->GetCombatStatsComponent())
	{
		Stats->ResetCombo();
		Stats->CacheContinuations(NAME_None, NAME_None);
		Stats->ConsumeComboRequest();
	}

	// Applied after the cancels: GA_Evade's EndAbility resets this tag's count to zero.
	ASC->AddLooseGameplayTag(CombatTags::State_Combat_Invincible);
	bPlayerInvincibleHeld = true;

	ProtectVictim(Target);

	// The scene asset does the rest: alignment, IK, montage sync, and collision between the pair.
	TMap<FName, FContextualAnimSceneBindingContext> Roles;
	Roles.Add(AttackerRole, FContextualAnimSceneBindingContext(Player)); // VERIFY the constructor
	Roles.Add(VictimRole, FContextualAnimSceneBindingContext(Target));

	FContextualAnimSceneBindings Bindings;
	if (!FContextualAnimSceneBindings::TryCreateBindings(*Scene, /*SectionIdx*/ 0, /*AnimSetIdx*/ 0, Roles, Bindings)
		|| !PlayerComp->StartContextualAnimScene(Bindings))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: could not start scene %s. Check the role names against the asset."), *GetName(), *Scene->GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	bSceneStarted = true;

	PlayerComp->OnLeftSceneDelegate.AddUniqueDynamic(this, &UGA_Finisher::HandleSceneLeft); // VERIFY the member name

	UAbilityTask_WaitDelay* Watchdog = UAbilityTask_WaitDelay::WaitDelay(this, MaxSceneDuration);
	Watchdog->OnFinish.AddDynamic(this, &UGA_Finisher::HandleSceneTimeout);
	Watchdog->ReadyForActivation();
}

void UGA_Finisher::ProtectVictim(ABaseCombatCharacter* InVictim)
{
	Victim = InVictim;
	UAbilitySystemComponent* VictimASC = InVictim->GetAbilitySystemComponent();
	VictimASC->CancelAllAbilities();
	VictimASC->AddLooseGameplayTag(CombatTags::State_Combat_BeingExecuted); // Phase 7: the Behavior Tree reads this to stop and retreat
	VictimASC->AddLooseGameplayTag(CombatTags::State_Combat_Invincible);
	bVictimProtected = true;
}

void UGA_Finisher::ReleaseVictim()
{
	if (!bVictimProtected) return;
	bVictimProtected = false;
	if (ABaseCombatCharacter* V = Victim.Get())
	{
		UAbilitySystemComponent* VictimASC = V->GetAbilitySystemComponent();
		VictimASC->RemoveLooseGameplayTag(CombatTags::State_Combat_Invincible);
		VictimASC->RemoveLooseGameplayTag(CombatTags::State_Combat_BeingExecuted);
	}
}

void UGA_Finisher::KillVictim(ABaseCombatCharacter* InVictim)
{
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = InVictim->GetAbilitySystemComponent();
	UCombatAttributeSet* Attributes = InVictim->GetAttributeSet();
	if (!SourceASC || !TargetASC || !Attributes || !DamageEffectClass || Attributes->GetHealth() <= 0.f) return;

	// if (UCombatFeedbackComponent* Feedback = InVictim->GetFeedbackComponent())
	// 	Feedback->ClearLastKnockback(); // otherwise the ragdoll flies off along the last hit's direction

	AActor* Avatar = GetAvatarActorFromActorInfo();
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddInstigator(Avatar, Avatar);
	FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(CombatTags::Data_Damage, -Attributes->GetHealth());
		SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC); // HandleHealthChanged then runs TriggerDeath: ragdoll and slow-mo
	}
}

void UGA_Finisher::HandleSceneLeft(UContextualAnimSceneActorComponent* SceneActorComponent)
{
	if (!IsActive()) return;
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGA_Finisher::HandleSceneTimeout()
{
	if (!IsActive()) return;
	UE_LOG(LogTemp, Warning, TEXT("%s: ended by the watchdog, so the scene's end callback never arrived."), *GetName());
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGA_Finisher::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ABaseCombatCharacter* Dying = Victim.Get();
	const bool bKill = bSceneStarted && !bWasCancelled && Dying;

	if (ABaseCombatCharacter* Player = Cast<ABaseCombatCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UContextualAnimSceneActorComponent* Comp = Player->FindComponentByClass<UContextualAnimSceneActorComponent>())
			Comp->OnLeftSceneDelegate.RemoveDynamic(this, &UGA_Finisher::HandleSceneLeft);

		if (bPlayerInvincibleHeld)
		{
			Player->GetAbilitySystemComponent()->RemoveLooseGameplayTag(CombatTags::State_Combat_Invincible);
			bPlayerInvincibleHeld = false;
		}
	}

	ReleaseVictim(); // before the kill, so nothing the victim still holds interferes
	if (bKill) KillVictim(Dying);

	bSceneStarted = false;
	Victim.Reset();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}