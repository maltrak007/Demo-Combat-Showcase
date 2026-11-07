// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Runtime/Engine/Classes/Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SendCombatEvent.generated.h"

/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UAnimNotify_SendCombatEvent : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
protected:
	UPROPERTY(EditAnywhere, Category = "Event") FGameplayTag EventTag;
};

