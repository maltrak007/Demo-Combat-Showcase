// Fill out your copyright notice in the Description page of Project Settings.


#include "StatWidgetBase.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"

void UStatWidgetBase::InitializeForCharacter(ABaseCombatCharacter* InCharacter)
{
	OwningCharacter = InCharacter;
	OnStatsReady();
}

