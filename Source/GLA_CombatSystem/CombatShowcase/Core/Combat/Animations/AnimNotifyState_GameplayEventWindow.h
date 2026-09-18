// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Runtime/GameplayTags/Classes/GameplayTagContainer.h"
#include "AnimNotifyState_GameplayEventWindow.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class GLA_COMBATSYSTEM_API UAnimNotifyState_GameplayEventWindow : public UAnimNotifyState
{
	GENERATED_BODY()
protected:
	FGameplayTag OpenEventTag;
	FGameplayTag CloseEventTag;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
