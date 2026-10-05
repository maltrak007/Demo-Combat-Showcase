// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_GrantGameplayTag.h"

#include "AbilitySystemComponent.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"

void UAnimNotifyState_GrantGameplayTag::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
											   float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp->GetOwner())
		if (ABaseCombatCharacter* CombatCharacter = Cast<ABaseCombatCharacter>(Owner))
			CombatCharacter->GetAbilitySystemComponent()->AddLooseGameplayTag(GameplayTagToGrant);
}

void UAnimNotifyState_GrantGameplayTag::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp->GetOwner())
		if (ABaseCombatCharacter* CombatCharacter = Cast<ABaseCombatCharacter>(Owner))
			CombatCharacter->GetAbilitySystemComponent()->RemoveLooseGameplayTag(GameplayTagToGrant);
}