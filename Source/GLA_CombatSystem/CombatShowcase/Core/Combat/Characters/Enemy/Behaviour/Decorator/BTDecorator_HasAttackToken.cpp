// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_HasAttackToken.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "CombatShowcase/Core/Combat/Subsystem/CombatDirectorSubsystem.h"

UBTDecorator_HasAttackToken::UBTDecorator_HasAttackToken()
{
	NodeName = TEXT("Has Attack Token");
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;
	FlowAbortMode = EBTFlowAbortMode::Both; // drop the attack when the token goes, and start it when the token arrives
}

bool UBTDecorator_HasAttackToken::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const UWorld* World = OwnerComp.GetWorld();
	const UCombatDirectorSubsystem* Director = World ? World->GetSubsystem<UCombatDirectorSubsystem>() : nullptr;
	if (!AIController || !AIController->GetPawn() || !Director) return false;

	const EAttackSlot Slot = Director->GetSlot(AIController->GetPawn());
	switch (Filter)
	{
	case EAttackTokenFilter::MeleeOnly:  return Slot == EAttackSlot::MeleeLeft || Slot == EAttackSlot::MeleeRight;
	case EAttackTokenFilter::RangedOnly: return Slot == EAttackSlot::Ranged;
	default:                             return Slot != EAttackSlot::None;
	}
}

void UBTDecorator_HasAttackToken::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTHasAttackTokenMemory* Memory = CastInstanceNodeMemory<FBTHasAttackTokenMemory>(NodeMemory);
	Memory->bLastValue = CalculateRawConditionValue(OwnerComp, NodeMemory);
}

void UBTDecorator_HasAttackToken::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTHasAttackTokenMemory* Memory = CastInstanceNodeMemory<FBTHasAttackTokenMemory>(NodeMemory);
	const bool bNow = CalculateRawConditionValue(OwnerComp, NodeMemory);
	if (bNow != Memory->bLastValue)
	{
		Memory->bLastValue = bNow;
		OwnerComp.RequestExecution(this); // the abort mode decides what that restarts
	}
}

FString UBTDecorator_HasAttackToken::GetStaticDescription() const
{
	const TCHAR* Which = Filter == EAttackTokenFilter::MeleeOnly ? TEXT("melee token")
	                   : Filter == EAttackTokenFilter::RangedOnly ? TEXT("ranged token")
	                   : TEXT("any token");
	return FString::Printf(TEXT("holds a %s\n%s"), Which, *Super::GetStaticDescription());
}
