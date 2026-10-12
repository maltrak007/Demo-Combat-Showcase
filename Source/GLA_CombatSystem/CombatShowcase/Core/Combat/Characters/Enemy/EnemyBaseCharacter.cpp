// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBaseCharacter.h"
#include "BaseEnemyAIController.h"
#include "Behaviour/CombatData.h"


AEnemyBaseCharacter::AEnemyBaseCharacter()
{
	AIControllerClass = ABaseEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	SetGenericTeamId(FGenericTeamId(static_cast<uint8>(ECombatTeam::Enemy)));
}


void AEnemyBaseCharacter::PossessedBy(AController* NewController)
{
	ApplyArchetype();
	Super::PossessedBy(NewController);
}

void AEnemyBaseCharacter::TriggerDeath()
{
	Super::TriggerDeath();
	GetController()->UnPossess();
}

void AEnemyBaseCharacter::ApplyArchetype()
{
}
