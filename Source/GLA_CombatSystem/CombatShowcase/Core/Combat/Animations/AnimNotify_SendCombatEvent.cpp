// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_SendCombatEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

void UAnimNotify_SendCombatEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr)
	{
		FGameplayEventData Data;
		Data.EventTag = EventTag;
		Data.Instigator = Owner;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Data);
	}
}
