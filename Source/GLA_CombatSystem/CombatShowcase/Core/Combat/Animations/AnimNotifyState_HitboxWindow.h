// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimNotifyState_GameplayEventWindow.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "AnimNotifyState_HitboxWindow.generated.h"

/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UAnimNotifyState_HitboxWindow : public UAnimNotifyState_GameplayEventWindow
{
	GENERATED_BODY()
public:
	UAnimNotifyState_HitboxWindow() {OpenEventTag = CombatTags::Event_Combat_HitboxOpen; CloseEventTag = CombatTags::Event_Combat_HitboxClose;  }
//	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
//	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
