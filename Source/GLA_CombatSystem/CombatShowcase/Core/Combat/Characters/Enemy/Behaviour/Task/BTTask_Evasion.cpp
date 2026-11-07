// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Evasion.h"

#include "AIController.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"


namespace
{
	ABaseCombatCharacter* GetCharacter(const UBehaviorTreeComponent& OwnerComp)
	{
		const AAIController* AIController = OwnerComp.GetAIOwner();
		return AIController ? Cast<ABaseCombatCharacter>(AIController->GetPawn()) : nullptr;
	}


}

UBTTask_Evasion::UBTTask_Evasion()
{
	NodeName = TEXT("Evasion");
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Evasion::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABaseCombatCharacter* Self = GetCharacter(OwnerComp);
	UAbilitySystemComponent* ASC = Self ? Self->GetAbilitySystemComponent() : nullptr;
	
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}

EBTNodeResult::Type UBTTask_Evasion::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_Evasion::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
}

FString UBTTask_Evasion::GetStaticDescription() const
{
	return Super::GetStaticDescription();
}
