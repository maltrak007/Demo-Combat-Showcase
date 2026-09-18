// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCombatCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "CombatShowcase/Core/Combat/Components/CombatStatsComponent.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"


// Sets default values
APlayerCombatCharacter::APlayerCombatCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void APlayerCombatCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerCombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APlayerCombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayerCombatCharacter::HandleMove(float AxisValue)
{
	if (!FMath::IsNearlyZero(AxisValue))
	{
		AddMovementInput(FVector::XAxisVector, AxisValue);
		FaceDirection(AxisValue);
	}
}

void APlayerCombatCharacter::LightAttack() { TryAttack(CombatTags::Event_Ability_LightAttack); }
void APlayerCombatCharacter::HeavyAttack() { TryAttack(CombatTags::Event_Ability_HeavyAttack); }
void APlayerCombatCharacter::StartBlock()   { SendCombatEvent(CombatTags::Event_Ability_Block); }
void APlayerCombatCharacter::StopBlock()    { /* wire once the Block ability exists — likely CancelAbilitiesByTag */ }
void APlayerCombatCharacter::Finisher()		{ SendCombatEvent(CombatTags::Event_Ability_Finisher); }
void APlayerCombatCharacter::Roll()         { SendCombatEvent(CombatTags::Event_Ability_Roll); }

void APlayerCombatCharacter::SendCombatEvent(const FGameplayTag& EventTag)
{
	FGameplayEventData EventData;
	EventData.EventTag = EventTag;
	EventData.Instigator = this;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventTag, EventData);
}

//Attack gated by tag
void APlayerCombatCharacter::TryAttack(const FGameplayTag& AttackEventTag)
{
	if (!AbilitySystemComponent->HasMatchingGameplayTag(CombatTags::Ability_Type_Attack))
	{
		SendCombatEvent(AttackEventTag);
		return;
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(CombatTags::Event_Combat_ComboWindowOpen))
	{
		GetCombatStatsComponent()->RequestComboContinuation(AttackEventTag); 
	}
}

void APlayerCombatCharacter::TryConsumeComboRequest()
{
	const FGameplayTag RequestedTag = GetCombatStatsComponent()->ConsumeComboRequest();
	if (!RequestedTag.IsValid())
	{
		return;
	}

	GetCombatStatsComponent()->AdvanceCombo();

	FGameplayTagContainer AttackTags;
	AttackTags.AddTag(CombatTags::Ability_Type_Attack);
	AbilitySystemComponent->CancelAbilities(&AttackTags); // safe now — the window, and the hitbox before it, have already closed

	SendCombatEvent(RequestedTag);
}