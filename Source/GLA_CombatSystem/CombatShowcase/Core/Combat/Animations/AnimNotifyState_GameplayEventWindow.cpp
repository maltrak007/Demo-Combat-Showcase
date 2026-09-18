// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_GameplayEventWindow.h"

#include "AbilitySystemBlueprintLibrary.h"

void UAnimNotifyState_GameplayEventWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp->GetOwner())
	{
		FGameplayEventData EventData;
		EventData.EventTag = OpenEventTag;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, OpenEventTag, EventData);
	}
}
void UAnimNotifyState_GameplayEventWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp->GetOwner())
	{
		FGameplayEventData EventData;
		EventData.EventTag = CloseEventTag;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, CloseEventTag, EventData);
	}
}