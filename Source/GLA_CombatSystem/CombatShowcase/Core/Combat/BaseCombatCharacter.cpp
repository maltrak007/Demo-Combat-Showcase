// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCombatCharacter.h"

#include "ContextualAnimSceneActorComponent.h"
#include "MotionWarpingComponent.h"
#include "TimerManager.h"
#include "CombatShowcase/Core/GAS/CombatAttributeSet.h"
#include "Components/CombatFeedbackComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "CombatShowcase/Core/HUD/StatWidgetBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/CombatStatsComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"

// Sets default values
ABaseCombatCharacter::ABaseCombatCharacter()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bConstrainToPlane = true;
	MoveComp->SetPlaneConstraintNormal(FVector::RightVector); // locks world Y
	MoveComp->bSnapToPlaneAtStart = true;

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	MoveComp->bOrientRotationToMovement = false;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	AttributeSet = CreateDefaultSubobject<UCombatAttributeSet>(TEXT("AttributeSet"));

	FeedbackComponent = CreateDefaultSubobject<UCombatFeedbackComponent>(TEXT("FeedbackComponent"));
	CombatStatsComponent = CreateDefaultSubobject<UCombatStatsComponent>(TEXT("CombatStatsComponent"));
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	ContextualAnimSceneActorComponent = CreateDefaultSubobject<UContextualAnimSceneActorComponent>(
		TEXT("ContextualAnimSceneActorComponent"));

	//PLACEHOLDER REMOVE AT PHASE 9
	StatWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatWidgetComponent"));
	StatWidgetComponent->SetupAttachment(GetMesh());
	StatWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 200.f)); // tune against your actual skeleton height
	StatWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	StatWidgetComponent->SetDrawSize(FVector2D(200.f, 50.f));
	StatWidgetComponent->SetGeometryMode(EWidgetGeometryMode::Plane); // Cylinder is for curved surfaces, not this
	StatWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	// default is (0,0) — top-left anchored, a documented, commonly-hit surprise
	StatWidgetComponent->SetRelativeScale3D(FVector(0.5f)); // scale down to fit the character
	StatWidgetComponent->SetBlendMode(EWidgetBlendMode::Masked);
}

void ABaseCombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called when the game starts or when spawned
void ABaseCombatCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->SetPlaneConstraintOrigin(GetActorLocation());

	if (UStatWidgetBase* StatWidget = Cast<UStatWidgetBase>(StatWidgetComponent->GetUserWidgetObject()))
	{
		StatWidget->InitializeForCharacter(this);
	}
}

void ABaseCombatCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AbilitySystemComponent->InitAbilityActorInfo(this, this); // must happen before anything else touches the ASC
	InitializeAttributes();
	GrantStartingAbilities();
	if (AttributeSet)
	{
		AttributeSet->OnHealthChanged.AddDynamic(this, &ABaseCombatCharacter::HandleHealthChanged);
	}
}

void ABaseCombatCharacter::FaceDirection(float SignedDirection)
{
	if (FMath::IsNearlyZero(SignedDirection)) return;

	const float TargetYaw = (SignedDirection > 0.f) ? 0.f : 180.f;
	if (!FMath::IsNearlyEqual(GetActorRotation().Yaw, TargetYaw))
	{
		SetActorRotation(FRotator(0.f, TargetYaw, 0.f));
	}
}

void ABaseCombatCharacter::InitializeAttributes()
{
	if (!DefaultAttributesEffect) return;
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);
	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributesEffect, 1.f, Context);
	if (Spec.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}

	if (StaminaRegenEffect)
	{
		FGameplayEffectContextHandle RegenContext = AbilitySystemComponent->MakeEffectContext();
		RegenContext.AddSourceObject(this);
		FGameplayEffectSpecHandle RegenSpec = AbilitySystemComponent->MakeOutgoingSpec(
			StaminaRegenEffect, 1.f, RegenContext);
		if (RegenSpec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*RegenSpec.Data.Get());
		}
	}
}

void ABaseCombatCharacter::GrantStartingAbilities()
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartingAbilities)
	{
		if (AbilityClass)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
		}
	}
}

void ABaseCombatCharacter::ResumeStaminaRegen()
{
	AbilitySystemComponent->RemoveLooseGameplayTag(CombatTags::State_Stamina_RegenBlocked);
}

void ABaseCombatCharacter::UpdateExecutableState()
{
	const float Health = GetHealthPercent();
	const bool bShouldBeExecutable = ExecutionThresholdPercent > 0.f && Health > 0.f && Health <=
		ExecutionThresholdPercent;
	if (bShouldBeExecutable == bExecutableTagHeld) return;

	if (bShouldBeExecutable) AbilitySystemComponent->AddLooseGameplayTag(CombatTags::State_Combat_Executable);
	else AbilitySystemComponent->RemoveLooseGameplayTag(CombatTags::State_Combat_Executable);
	bExecutableTagHeld = bShouldBeExecutable;
}

void ABaseCombatCharacter::HandleHealthChanged(float NewHealth, float DamageAmount, bool bIsDead)
{
	UpdateExecutableState();
	if (bIsDead)
	{
		TriggerDeath();
		return;
	}
	if (DamageAmount > 0.f)
	{
		GetCombatStatsComponent()->RegisterDamageTaken();
		GetCombatStatsComponent()->ResetCombo();

		FGameplayTagContainer AttackTags;
		AttackTags.AddTag(CombatTags::Ability_Type_Attack);
		AbilitySystemComponent->CancelAbilities(&AttackTags);
	}
}

void ABaseCombatCharacter::TriggerDeath()
{
	GetCharacterMovement()->DisableMovement();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
	MeshComp->SetSimulatePhysics(true);
	MeshComp->WakeAllRigidBodies();

	const FVector DeathImpulse = GetFeedbackComponent()->GetLastKnockbackVelocity() * DeathImpulseScale;
	MeshComp->AddImpulse(DeathImpulse, NAME_None, false);

	if (!IsPlayerControlled())
	{
		GetFeedbackComponent()->TriggerDeathSlowMo();
	}

	AbilitySystemComponent->CancelAllAbilities();
}

float ABaseCombatCharacter::GetHealthPercent() const
{
	return (AttributeSet && AttributeSet->GetMaxHealth() > 0.f)
		       ? AttributeSet->GetHealth() / AttributeSet->GetMaxHealth()
		       : 0.f;
}

float ABaseCombatCharacter::GetStaminaPercent() const
{
	return (AttributeSet && AttributeSet->GetMaxStamina() > 0.f)
		       ? AttributeSet->GetStamina() / AttributeSet->GetMaxStamina()
		       : 0.f;
}

bool ABaseCombatCharacter::IsMovementLocked() const
{
	return AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(
		CombatTags::State_Combat_MovementLocked);
}

bool ABaseCombatCharacter::IsExecutable() const
{
	return AbilitySystemComponent
		&& AbilitySystemComponent->HasMatchingGameplayTag(CombatTags::State_Combat_Executable)
		&& !AbilitySystemComponent->HasMatchingGameplayTag(CombatTags::State_Combat_BeingExecuted);
}

void ABaseCombatCharacter::ConsumeStamina(float Amount)
{
	if (!StaminaCostEffect || !AbilitySystemComponent) return;

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);
	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(StaminaCostEffect, 1.f, Context);
	if (!Spec.IsValid()) return;

	// REVISE CAUSE IT CAN GO WAY FURTHER BELOW 0 THAN EXPECTED
	Spec.Data->SetSetByCallerMagnitude(CombatTags::Data_StaminaConsumed, -Amount);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	if (!AbilitySystemComponent->HasMatchingGameplayTag(CombatTags::State_Stamina_RegenBlocked))
	{
		AbilitySystemComponent->AddLooseGameplayTag(CombatTags::State_Stamina_RegenBlocked);
	}

	// Decided by the RESULT of this spend, not the amount — hitting empty specifically
	// gets the longer penalty delay, everything else gets the short one.
	const bool bExhausted = AttributeSet->GetStamina() <= 0.f;
	const float Delay = bExhausted ? StaminaRegenDelayExhausted : StaminaRegenDelayNormal;

	// Always resets to the full delay on every new spend — matches "after your last action"
	// for the normal case, and "resets to three seconds" for the exhausted case, with the
	// same one mechanism serving both.
	GetWorld()->GetTimerManager().SetTimer(StaminaRegenDelayHandle, this, &ABaseCombatCharacter::ResumeStaminaRegen,
	                                       Delay, false);
}

void ABaseCombatCharacter::GrantStaminaBurst(float Amount)
{
	if (!StaminaBurstEffect || !AbilitySystemComponent) return;
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);
	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(StaminaBurstEffect, 1.f, Context);
	if (Spec.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}
