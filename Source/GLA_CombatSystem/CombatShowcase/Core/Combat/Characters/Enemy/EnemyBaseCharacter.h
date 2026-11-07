// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "EnemyBaseCharacter.generated.h"

class UNavigationComponent;
class UEnemyBehaviourComponent;

UCLASS()
class GLA_COMBATSYSTEM_API AEnemyBaseCharacter : public ABaseCombatCharacter
{
	GENERATED_BODY()
public:
	AEnemyBaseCharacter();

	UEnemyBehaviourComponent* GetBehaviourComponent() const { return BehaviourComponent; }
	
protected:
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void TriggerDeath() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UNavigationComponent> NavigationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UEnemyBehaviourComponent> BehaviourComponent;
	
private:
	void ApplyArchetype();
};
