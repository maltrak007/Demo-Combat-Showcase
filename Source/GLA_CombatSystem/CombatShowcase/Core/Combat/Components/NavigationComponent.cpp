// Fill out your copyright notice in the Description page of Project Settings.


#include "NavigationComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values for this component's properties
UNavigationComponent::UNavigationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UNavigationComponent::SetMoveSpeed(float NewSpeed)
{
	if (MoveComp.IsValid()) MoveComp->MaxWalkSpeed = NewSpeed;
}

void UNavigationComponent::ResetMoveSpeed()
{
	if (MoveComp.IsValid() && DefaultWalkSpeed > 0.f) MoveComp->MaxWalkSpeed = DefaultWalkSpeed;
}


// Called when the game starts
void UNavigationComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		MoveComp = Character->GetCharacterMovement();
		if (MoveComp.IsValid()) DefaultWalkSpeed = MoveComp->MaxWalkSpeed;
	}
}




