// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_StatusTimer.generated.h"

struct FAnimNotifyEventReference;
/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UAnimNotify_StatusTimer : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.1")) float Duration = 3.f;
	UPROPERTY(EditAnywhere) FName LoopSection = "Loop";
	UPROPERTY(EditAnywhere) FName EndSection = "End";
	
	UPROPERTY(EditAnywhere) bool bFinishLoopFirst = true;
};
