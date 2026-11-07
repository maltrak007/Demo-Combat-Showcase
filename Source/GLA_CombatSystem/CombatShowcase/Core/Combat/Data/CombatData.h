#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CombatData.generated.h"

class UNiagaraSystem;
class UGameplayEffect;

UENUM(BlueprintType)
enum class ECombatHitboxSource : uint8
{
	BareBone,      // resolved against the character's own skeletal mesh
	WeaponSocket   // resolved against the character's currently equipped weapon mesh
};

UENUM(BlueprintType)
enum class EStrikeDirection: uint8
{
	Up,
	Down
};

UENUM(BlueprintType)
enum class EHitOutcome : uint8
{
	Unblocked, 
	Blocked, 
	Parried,
	Invincible,
	Whiff
};

UENUM(BlueprintType)
enum class EHitReactionDirection : uint8
{
	Generic,   
	HeadLeft,
	HeadMiddle,
	HeadRight,
	BodyLeft,
	BodyMiddle,
	BodyRight,
	Uppercut,
	Downward,
	HighSweep,
	LowSweep,
	LowLeft,
	LowRight
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
	UPROPERTY(EditAnywhere, Category = "Strike") float MontagePlayRate = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Strike") TArray<FCombatHitboxDef> Hitboxes;
	UPROPERTY(EditAnywhere, Category = "Strike") float Damage = 10.f;
	UPROPERTY(EditAnywhere, Category = "Strike") float StaminaCost = 0.f;
	UPROPERTY(EditAnywhere, Category = "Strike") float StaminaDamage = 10.f;
	UPROPERTY(EditAnywhere, Category = "Strike") EStrikeDirection Direction = EStrikeDirection::Up;
	UPROPERTY(EditAnywhere, Category = "Strike|Reaction") EHitReactionDirection ReactionDirection = EHitReactionDirection::Generic;
	UPROPERTY(EditAnywhere, Category = "Strike|Feel") FCombatImpactFeel ImpactFeel;
	UPROPERTY(EditAnywhere, Category = "Strike|Type") FGameplayTag StrikeTag;
	
	// Tree branching: each row points to its own next row per button.
	// Empty = this branch has no continuation for that input; the chain ends here.
	UPROPERTY(EditAnywhere, Category = "Strike|Combo") FName NextRowOnLight;
	UPROPERTY(EditAnywhere, Category = "Strike|Combo") FName NextRowOnHeavy;
};

USTRUCT(BlueprintType)
struct FHitResolution
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) EHitOutcome Outcome = EHitOutcome::Unblocked;
	UPROPERTY(BlueprintReadOnly) float FinalDamage = 0.f;
	UPROPERTY(BlueprintReadOnly) float FinalStaminaDamage = 0.f;
	UPROPERTY(BlueprintReadOnly) bool bAppliesKnockback = false;
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
	UPROPERTY(BlueprintReadOnly) FHitResolution OutCome;
	UPROPERTY(BlueprintReadOnly) FGameplayTag TypeStrikeTag;
	UPROPERTY(BlueprintReadOnly) uint8 SurfaceType = 0;
};

USTRUCT(BlueprintType)
struct FCombatStrikePayload
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) float Damage = 0.f;
	UPROPERTY(BlueprintReadOnly) float StaminaDamage = 0.f;
	UPROPERTY(BlueprintReadOnly) EStrikeDirection Direction = EStrikeDirection::Up;
	UPROPERTY(BlueprintReadOnly) EHitReactionDirection ReactionDirection = EHitReactionDirection::Generic;
	UPROPERTY(BlueprintReadOnly) FCombatImpactFeel ImpactFeel;
	UPROPERTY(BlueprintReadOnly) TSubclassOf<UGameplayEffect> HPEffectClass;
	UPROPERTY(BlueprintReadOnly) TSubclassOf<UGameplayEffect> STAEffectClass;
	UPROPERTY(BlueprintReadOnly) FGameplayTag StrikeTag;
	// A bullet. A guard held at the parry moment blocks it instead of parrying.
	UPROPERTY(BlueprintReadOnly) bool bRanged = false;
};


enum class ECombatTeam : uint8 { Player = 1, Enemy = 2 };


USTRUCT(BlueprintType)
struct FCombatFeedbackRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere) FGameplayTag WeaponTag;
	UPROPERTY(EditAnywhere) EHitOutcome FeedbackEvent = EHitOutcome::Unblocked;
	UPROPERTY(EditAnywhere) TSoftObjectPtr<UNiagaraSystem> Impact;
	UPROPERTY(EditAnywhere) TSoftObjectPtr<USoundBase> Sound; 
};