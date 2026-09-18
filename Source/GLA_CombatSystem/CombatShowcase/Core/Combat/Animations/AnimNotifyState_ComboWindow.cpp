// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_ComboWindow.h"
#include "AbilitySystemComponent.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "../BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Characters/Player/PlayerCombatCharacter.h"

void UAnimNotifyState_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                               float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (ABaseCombatCharacter* CombatCharacter = Cast<ABaseCombatCharacter>(Owner))
		{
			CombatCharacter->GetAbilitySystemComponent()->AddLooseGameplayTag(CombatTags::Event_Combat_ComboWindowOpen);
		}
	}
}

void UAnimNotifyState_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (ABaseCombatCharacter* CombatCharacter = Cast<ABaseCombatCharacter>(Owner))
		{
			CombatCharacter->GetAbilitySystemComponent()->RemoveLooseGameplayTag(CombatTags::Event_Combat_ComboWindowOpen);
		}
		if (APlayerCombatCharacter* PlayerChar = Cast<APlayerCombatCharacter>(Owner))
		{
			PlayerChar->TryConsumeComboRequest(); // the one and only check-and-act moment now
		}
	}
}
