// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCombatCharacter.h"
#include "CombatShowcase/Core/GAS/CombatAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"


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
}

// Called when the game starts or when spawned
void ABaseCombatCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->SetPlaneConstraintOrigin(GetActorLocation());
}

void ABaseCombatCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
    AbilitySystemComponent->InitAbilityActorInfo(this, this); // must happen before anything else touches the ASC
    InitializeAttributes();
    GrantStartingAbilities();
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

// Called every frame
void ABaseCombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABaseCombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

