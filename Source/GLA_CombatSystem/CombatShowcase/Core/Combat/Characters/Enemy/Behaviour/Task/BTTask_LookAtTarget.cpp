// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_LookAtTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CombatShowcase/Core/Combat/Characters/Enemy/EnemyBaseCharacter.h"

UBTTask_LookAtTarget::UBTTask_LookAtTarget()
{
	NodeName = TEXT("Look At Target");

	// One selector, two accepted key types.
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_LookAtTarget, TargetKey), AActor::StaticClass());
	TargetKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_LookAtTarget, TargetKey));
}

void UBTTask_LookAtTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (const UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetKey.ResolveSelectedKey(*BBAsset);
	}
}

EBTNodeResult::Type UBTTask_LookAtTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	ABaseCombatCharacter* Self = AIController ? Cast<ABaseCombatCharacter>(AIController->GetPawn()) : nullptr;
	const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Self || !BB || !TargetKey.IsSet()) return EBTNodeResult::Failed;

	if (bRespectMovementLock && Self->IsMovementLocked()) return EBTNodeResult::Failed;

	// Works for both key types: an actor key gives the actor's location, a vector key gives the vector.
	FVector TargetLocation;
	if (!BB->GetLocationFromEntry(TargetKey.SelectedKeyName, TargetLocation)) return EBTNodeResult::Failed;

	// The lane is the X axis, so only X matters. Y and Z are ignored on purpose.
	const float DeltaX = TargetLocation.X - Self->GetActorLocation().X;
	if (FMath::Abs(DeltaX) >= MinDistanceX)
	{
		Self->FaceDirection(DeltaX);
	}
	return EBTNodeResult::Succeeded;
}

FString UBTTask_LookAtTarget::GetStaticDescription() const
{
	return FString::Printf(TEXT("face toward %s"), *TargetKey.SelectedKeyName.ToString());
}
