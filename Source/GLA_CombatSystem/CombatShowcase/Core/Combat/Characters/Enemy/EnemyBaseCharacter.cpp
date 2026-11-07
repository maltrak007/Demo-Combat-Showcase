// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBaseCharacter.h"
#include "BaseEnemyAIController.h"
#include "BrainComponent.h"
#include "CombatShowcase/Core/Combat/Components/NavigationComponent.h"
#include "CombatShowcase/Core/Combat/Components/EnemyBehaviourComponent.h"
#include "CombatShowcase/Core/Combat/Data/CombatData.h"


AEnemyBaseCharacter::AEnemyBaseCharacter()
{
	AIControllerClass = ABaseEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	SetGenericTeamId(FGenericTeamId(static_cast<uint8>(ECombatTeam::Enemy)));
	
	NavigationComponent = CreateDefaultSubobject<UNavigationComponent>(TEXT("NavigationComponent"));
	BehaviourComponent = CreateDefaultSubobject<UEnemyBehaviourComponent>(TEXT("BehaviourComponent"));
}


void AEnemyBaseCharacter::PossessedBy(AController* NewController)
{
	ApplyArchetype();
	Super::PossessedBy(NewController);
}

void AEnemyBaseCharacter::TriggerDeath()
{
	Super::TriggerDeath();
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
		
		if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
		{
			BrainComp->StopLogic("Died");
		}
		
		AIController->UnPossess();
	}
}

void AEnemyBaseCharacter::ApplyArchetype()
{
}
