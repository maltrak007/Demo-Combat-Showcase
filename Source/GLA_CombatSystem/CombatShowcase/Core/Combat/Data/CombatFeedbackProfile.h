// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatData.h"
#include "GameplayTagContainer.h"
#include "Runtime/Engine/Classes/Engine/DataAsset.h"
#include "CombatFeedbackProfile.generated.h"



USTRUCT(BlueprintType)
struct FOutcomeOverlay
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere) FLinearColor Color = FLinearColor::Blue;
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0")) float Intensity = 1.f;
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.01")) float Duration = 0.2f;
};

UCLASS(BlueprintType)
class GLA_COMBATSYSTEM_API UCombatFeedbackProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	// The most specific row for this weapon and event, or null. Call PreloadAll first.
	const FCombatFeedbackRow* Find(const FGameplayTag& WeaponTag, EHitOutcome HitOutcome) const;
	// Loads every soft reference once so the first hit never hitches. A no-op after the first call.
	void PreloadAll();

	UPROPERTY(EditAnywhere, Category = "Rows", meta = (RequiredAssetDataTags = "RowStructure=/Script/GLA_CombatSystem.CombatFeedbackRow"))
	TObjectPtr<UDataTable> FeedbackTable;

	// The MetaSound input that receives the surface type. Keep it identical to the input's name.
	UPROPERTY(EditAnywhere, Category = "Sound") FName SurfaceParameter = "Surface";

	UPROPERTY(EditAnywhere, Category = "Overlay") TSoftObjectPtr<UMaterialInterface> OverlayMaterial;
	UPROPERTY(EditAnywhere, Category = "Overlay") FName ColorParameter = "OutlineColor";
	UPROPERTY(EditAnywhere, Category = "Overlay") FName IntensityParameter = "Intensity";
	// Only Blocked and Parried need an entry.
	UPROPERTY(EditAnywhere, Category = "Overlay") TMap<EHitOutcome, FOutcomeOverlay> Overlays;

private:
	UPROPERTY(Transient) TArray<TObjectPtr<UObject>> Loaded;
	TArray<const FCombatFeedbackRow*> CachedRows;
	bool bPreloaded = false;
};
