// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvQueryContext_OtherEnemies.h"

#include "AIController.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Kismet/GameplayStatics.h"
#include "CombatShowcase/Core/Combat/Characters/Enemy/EnemyBaseCharacter.h"

void UEnvQueryContext_OtherEnemies::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	const AActor* Querier = Cast<AActor>(QueryInstance.Owner.Get());
	if (const AController* Controller = Cast<AController>(Querier)) Querier = Controller->GetPawn();

	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(QueryInstance.World, AEnemyBaseCharacter::StaticClass(), Enemies);

	Enemies.RemoveAll([Querier](const AActor* Enemy)
	{
		const ABaseCombatCharacter* Character = Cast<ABaseCombatCharacter>(Enemy);
		return Enemy == Querier || (Character && Character->GetHealthPercent() <= 0.f);
	});

	UEnvQueryItemType_Actor::SetContextHelper(ContextData, Enemies);
}
