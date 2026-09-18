// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatStatsComponent.h"

void UCombatStatsComponent::AdvanceCombo()
{
	ComboIndex++;
}
void UCombatStatsComponent::ResetCombo() { ComboIndex = 0; }
void UCombatStatsComponent::RegisterHitLanded() { ComboHitCount++; }
void UCombatStatsComponent::RegisterDamageTaken() { ComboHitCount = 0; }