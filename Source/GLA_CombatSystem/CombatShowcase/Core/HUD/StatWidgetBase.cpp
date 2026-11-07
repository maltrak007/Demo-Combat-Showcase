// Fill out your copyright notice in the Description page of Project Settings.


#include "StatWidgetBase.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Components/CombatFeedbackComponent.h"

void UStatWidgetBase::InitializeForCharacter(ABaseCombatCharacter* InCharacter)
{
	OwningCharacter = InCharacter;

	if (UCombatFeedbackComponent* Feedback = InCharacter ? InCharacter->GetFeedbackComponent() : nullptr)
	{
		Feedback->OnTelegraphStarted.AddUniqueDynamic(this, &UStatWidgetBase::HandleTelegraphStarted);
		Feedback->OnTelegraphEnded.AddUniqueDynamic(this, &UStatWidgetBase::HandleTelegraphEnded);
	}

	OnStatsReady();
}

void UStatWidgetBase::NativeDestruct()
{
	if (const ABaseCombatCharacter* Character = OwningCharacter.Get())
	{
		if (UCombatFeedbackComponent* Feedback = Character->GetFeedbackComponent())
		{
			Feedback->OnTelegraphStarted.RemoveAll(this);
			Feedback->OnTelegraphEnded.RemoveAll(this);
		}
	}
	Super::NativeDestruct();
}

void UStatWidgetBase::HandleTelegraphStarted(float Duration)
{
	OnShotTelegraphStarted(Duration);
}

void UStatWidgetBase::HandleTelegraphEnded()
{
	OnShotTelegraphEnded();
}
