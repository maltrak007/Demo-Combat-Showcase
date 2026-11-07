// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvQueryContext_AttackTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Kismet/GameplayStatics.h"
#include "CombatShowcase/Core/Combat/Characters/Enemy/Behaviour/BehaviourData.h"

void UEnvQueryContext_AttackTarget::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	// The querier is the controller when the query runs from a behavior tree, and the pawn otherwise.
	const AActor* Querier = Cast<AActor>(QueryInstance.Owner.Get());
	const AAIController* AIController = Cast<AAIController>(Querier);
	if (!AIController)
	{
		if (const APawn* Pawn = Cast<APawn>(Querier)) AIController = Cast<AAIController>(Pawn->GetController());
	}

	AActor* Target = nullptr;
	if (AIController && AIController->GetBlackboardComponent())
	{
		Target = Cast<AActor>(AIController->GetBlackboardComponent()->GetValueAsObject(EnemyBBKeys::TargetActor));
	}
	if (!Target)
	{
		Target = UGameplayStatics::GetPlayerPawn(QueryInstance.World, 0);
	}
	if (Target)
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, Target);
	}
}
