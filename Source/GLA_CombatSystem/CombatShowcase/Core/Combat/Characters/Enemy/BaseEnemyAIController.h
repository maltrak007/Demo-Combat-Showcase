// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Behaviour/BehaviourData.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "BaseEnemyAIController.generated.h"


struct FAIStimulus;
class UAISenseConfig_Sight;

UCLASS()
class GLA_COMBATSYSTEM_API ABaseEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	ABaseEnemyAIController();
	// Team identity belongs to the pawn. The controller only reports it.
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void SetGenericTeamId(const FGenericTeamId& NewID) override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

	// Called with the controller's defaults now, and with the archetype's settings once archetypes exist.
	void ApplyPerceptionSettings(const FEnemyPerceptionSettings& Settings);

protected:
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI|Behavior")
	TMap<FGameplayTag, TObjectPtr<UBehaviorTree>> DefaultSubtrees;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI|Behavior")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI|Perception")
	FEnemyPerceptionSettings DefaultPerception;

private:
	
	void OnExecutionTagChanged(FGameplayTag Tag, int32 NewCount);
	
	bool ValidateBlackboardKeys() const;
	
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	
	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
};
