// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "EnemyBaseCharacter.generated.h"

UCLASS()
class GLA_COMBATSYSTEM_API AEnemyBaseCharacter : public ABaseCombatCharacter
{
	GENERATED_BODY()
public:
	AEnemyBaseCharacter();
	
protected:
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void TriggerDeath() override;
private:
	void ApplyArchetype();
};
