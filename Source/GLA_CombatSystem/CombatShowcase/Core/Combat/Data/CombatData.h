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
	UPROPERTY(EditAnywhere) FName Bone; 
	UPROPERTY(EditAnywhere) float Radius = 15.f;
	UPROPERTY(EditAnywhere) uint8 Rank = 0;
	UPROPERTY(EditAnywhere) uint8 Group = 0;
};

USTRUCT(BlueprintType)
struct FCombatImpactFeel
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float HitstopDuration = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float KnockbackMagnitude = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) TSubclassOf<UCameraShakeBase> ImpactShake;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float ShakeScale = 1.f;
};

USTRUCT(BlueprintType)
struct FCombatStrikeRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = "Strike") UAnimMontage* Montage = nullptr;
	UPROPERTY(EditAnywhere, Category = "Strike") TArray<FCombatHitboxDef> Hitboxes;
	UPROPERTY(EditAnywhere, Category = "Strike") float Damage = 10.f;
	UPROPERTY(EditAnywhere, Category = "Strike|Feel") FCombatImpactFeel ImpactFeel;
	
	// Tree branching: each row points to its own next row per button.
	// Empty = this branch has no continuation for that input; the chain ends here.
	UPROPERTY(EditAnywhere, Category = "Strike|Combo") FName NextRowOnLight;
	UPROPERTY(EditAnywhere, Category = "Strike|Combo") FName NextRowOnHeavy;
};

USTRUCT(BlueprintType)
struct FCombatImpactEvent
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) AActor* Target = nullptr;
	UPROPERTY(BlueprintReadOnly) AActor* Instigator = nullptr;
	UPROPERTY(BlueprintReadOnly) FVector KnockbackVelocity = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly) FVector ImpactPoint = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly) FCombatImpactFeel ImpactFeel;
};