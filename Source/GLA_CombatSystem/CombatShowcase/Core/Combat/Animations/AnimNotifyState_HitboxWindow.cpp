// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_HitboxWindow.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"

void UAnimNotifyState_HitboxWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp->GetOwner())
	{
		FGameplayEventData EventData;
		EventData.EventTag = CombatTags::Event_Combat_HitboxOpen;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, CombatTags::Event_Combat_HitboxOpen, EventData);
	}
}

void UAnimNotifyState_HitboxWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp->GetOwner())
	{
		FGameplayEventData EventData;
		EventData.EventTag = CombatTags::Event_Combat_HitboxClose;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, CombatTags::Event_Combat_HitboxClose, EventData);
	}
}