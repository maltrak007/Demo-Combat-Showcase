// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_ParryWindow.h"

#include "AbilitySystemComponent.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"

void UAnimNotifyState_ParryWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                               float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp->GetOwner())
		if (ABaseCombatCharacter* CombatCharacter = Cast<ABaseCombatCharacter>(Owner))
			CombatCharacter->GetAbilitySystemComponent()->AddLooseGameplayTag(CombatTags::State_Combat_ParryWindowOpen);
}

void UAnimNotifyState_ParryWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp->GetOwner())
		if (ABaseCombatCharacter* CombatCharacter = Cast<ABaseCombatCharacter>(Owner))
			CombatCharacter->GetAbilitySystemComponent()->RemoveLooseGameplayTag(CombatTags::State_Combat_ParryWindowOpen);
}
