// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ReleaseAttackToken.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CombatShowcase/Core/Combat/Characters/Enemy/EnemyBaseCharacter.h"
#include "CombatShowcase/Core/Combat/Characters/Enemy/Behaviour/BehaviourData.h"
#include "CombatShowcase/Core/Combat/Components/EnemyBehaviourComponent.h"
#include "CombatShowcase/Core/Combat/Subsystem/CombatDirectorSubsystem.h"

UBTTask_ReleaseAttackToken::UBTTask_ReleaseAttackToken()
{
	NodeName = TEXT("Release Attack Token");
}

EBTNodeResult::Type UBTTask_ReleaseAttackToken::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	UCombatDirectorSubsystem* Director = Pawn ? Pawn->GetWorld()->GetSubsystem<UCombatDirectorSubsystem>() : nullptr;
	if (Director)
	{
		// One token is one sequence, so the wait before the next one starts here, after the token is back in the pool.
		float Cooldown = -1.f;
		if (bApplyCooldown && bUseBehaviourCooldown)
		{
			const EAttackSlot Slot = Director->GetSlot(Pawn);
			const AEnemyBaseCharacter* Enemy = Cast<AEnemyBaseCharacter>(Pawn);
			const UEnemyBehaviourComponent* Behaviour = Enemy ? Enemy->GetBehaviourComponent() : nullptr;
			if (Behaviour && Slot != EAttackSlot::None)
			{
				const FWeaponBehaviourRow& Stats = Behaviour->GetStats();
				Cooldown = (Slot == EAttackSlot::Ranged) ? Stats.BurstCooldown : Stats.SequenceCooldown;
			}
		}

		if (Cooldown >= 0.f) Director->ReleaseTokenWithCooldown(Pawn, Cooldown);
		else Director->ReleaseToken(Pawn, bApplyCooldown);
	}

	// Clear the flag now, so the decorator doesn't wait for the service's next tick to see the token gone.
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		if (BB->GetKeyID(EnemyBBKeys::HasAttackToken) != FBlackboard::InvalidKey)
		{
			BB->SetValueAsBool(EnemyBBKeys::HasAttackToken, false);
		}
	}
	return EBTNodeResult::Succeeded;
}

FString UBTTask_ReleaseAttackToken::GetStaticDescription() const
{
	if (!bApplyCooldown) return TEXT("give the token back, no cooldown");
	return bUseBehaviourCooldown ? TEXT("give the token back, cooldown from the behaviour row")
	                             : TEXT("give the token back, default cooldown");
}
