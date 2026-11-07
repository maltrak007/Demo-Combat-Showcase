// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatFeedbackProfile.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "NiagaraSystem.h"
#include "Engine/DataTable.h"

const FCombatFeedbackRow* UCombatFeedbackProfile::Find(const FGameplayTag& WeaponTag, EHitOutcome HitOutcome) const
{
	const FCombatFeedbackRow* Best = nullptr;
	int32 BestDepth = -1;
	for (const FCombatFeedbackRow* Row : CachedRows)
	{
		if (Row->FeedbackEvent != HitOutcome) continue;

		int32 Depth = 0; // an empty tag is the generic fallback
		if (Row->WeaponTag.IsValid())
		{
			if (!WeaponTag.MatchesTag(Row->WeaponTag)) continue;      // same tag or one of its children
			Depth = Row->WeaponTag.GetGameplayTagParents().Num();     // deeper means more specific
		}
		if (Depth > BestDepth) { Best = Row; BestDepth = Depth; }
	}
	return Best;
}

void UCombatFeedbackProfile::PreloadAll()
{
	if (bPreloaded) return;
	bPreloaded = true;

	auto Keep = [this](UObject* Asset) { if (Asset) Loaded.Add(Asset); };
	if (FeedbackTable)
	{
		FeedbackTable->ForeachRow<FCombatFeedbackRow>(TEXT("CombatFeedbackProfile"),
			[&](const FName&, const FCombatFeedbackRow& Row)
			{
				CachedRows.Add(&Row);
				Keep(Row.Impact.LoadSynchronous());
				Keep(Row.Sound.LoadSynchronous());
			});
	}
	Keep(OverlayMaterial.LoadSynchronous());
}