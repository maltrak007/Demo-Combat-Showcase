// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Components/ActorComponent.h"
#include "NavigationComponent.generated.h"


class UCharacterMovementComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GLA_COMBATSYSTEM_API UNavigationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNavigationComponent();
	
	void SetMoveSpeed(float NewSpeed);
	void ResetMoveSpeed();
	
	const TArray<TObjectPtr<AActor>>& GetPatrolPoints() const { return PatrolPoints; }
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UPROPERTY(EditInstanceOnly, Category = "AI|Patrol")
	TArray<TObjectPtr<AActor>> PatrolPoints;

private:
	TWeakObjectPtr<UCharacterMovementComponent> MoveComp;
	float DefaultWalkSpeed = 0.f;
};
