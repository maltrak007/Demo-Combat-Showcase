#pragma once
#include "CoreMinimal.h"
#include "CombatData.generated.h"

UENUM(BlueprintType)
enum class ECombatHitboxSource : uint8
{
	BareBone,      // resolved against the character's own skeletal mesh
	WeaponSocket   // resolved against the character's currently equipped weapon mesh
};

USTRUCT(BlueprintType)
struct FCombatHitboxDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) ECombatHitboxSource Source = ECombatHitboxSource::BareBone;
	UPROPERTY(EditAnywhere) FName Bone; // skeleton bone name (BareBone) or weapon socket name (WeaponSocket)
	UPROPERTY(EditAnywhere) float Radius = 15.f;
	UPROPERTY(EditAnywhere) uint8 Rank = 0;
	UPROPERTY(EditAnywhere) uint8 Group = 0;
};

USTRUCT(BlueprintType)
struct FCombatStrikeRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = "Strike") UAnimMontage* Montage = nullptr;
	UPROPERTY(EditAnywhere, Category = "Strike") TArray<FCombatHitboxDef> Hitboxes;
	UPROPERTY(EditAnywhere, Category = "Strike") float Damage = 10.f;
	UPROPERTY(EditAnywhere, Category = "Strike|Feel (Phase 2)") float HitstopDuration = 0.f;
	UPROPERTY(EditAnywhere, Category = "Strike|Feel (Phase 2)") float KnockbackMagnitude = 0.f;
};