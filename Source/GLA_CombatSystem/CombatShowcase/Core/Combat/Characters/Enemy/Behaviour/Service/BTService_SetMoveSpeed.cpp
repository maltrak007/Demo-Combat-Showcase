// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_SetMoveSpeed.h"

#include "AIController.h"
#include "CombatShowcase/Core/Combat/Characters/Enemy/EnemyBaseCharacter.h"
#include "CombatShowcase/Core/Combat/Components/EnemyBehaviourComponent.h"
#include "CombatShowcase/Core/Combat/Components/NavigationComponent.h"

namespace
{
	UNavigationComponent* FindNavigation(UBehaviorTreeComponent& OwnerComp)
	{
		const AAIController* AIController = OwnerComp.GetAIOwner();
		const APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
		return Pawn ? Pawn->FindComponentByClass<UNavigationComponent>() : nullptr;
	}
}

UBTService_SetMoveSpeed::UBTService_SetMoveSpeed()
{
	NodeName = TEXT("Set Move Speed");
	bNotifyTick = false;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTService_SetMoveSpeed::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	if (UNavigationComponent* Navigation = FindNavigation(OwnerComp))
	{
		// The node asks for a pace for this state. The enemy's behaviour row scales it, so a novice walks slower than a master.
		float Scale = 1.f;
		const AAIController* AIController = OwnerComp.GetAIOwner();
		if (const AEnemyBaseCharacter* Enemy = AIController ? Cast<AEnemyBaseCharacter>(AIController->GetPawn()) : nullptr)
		{
			if (const UEnemyBehaviourComponent* Behaviour = Enemy->GetBehaviourComponent()) Scale = Behaviour->GetStats().MoveSpeedScale;
		}
		Navigation->SetMoveSpeed(MoveSpeed * Scale);
	}
}

void UBTService_SetMoveSpeed::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (UNavigationComponent* Navigation = FindNavigation(OwnerComp)) Navigation->ResetMoveSpeed();
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

FString UBTService_SetMoveSpeed::GetStaticDescription() const
{
	return FString::Printf(TEXT("walk speed %.0f while active"), MoveSpeed);
}