// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatGameplayAbility.h"

bool UCombatGameplayAbility::HasValidAnimInstance(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo || !ActorInfo->GetAnimInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: no AnimInstance on avatar %s — likely missing an Anim Blueprint assignment."),
			*GetName(), (ActorInfo && ActorInfo->AvatarActor.IsValid()) ? *ActorInfo->AvatarActor->GetName() : TEXT("(unknown)"));
		return false;
	}
	return true;
}