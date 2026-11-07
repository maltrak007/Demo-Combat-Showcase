// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Runtime/Engine/Classes/Engine/DataAsset.h"
#include "PDT_Weapon.generated.h"

class UDataTable;
class USkeletalMesh;
class UStaticMesh;
class UAnimMontage;
class UNiagaraSystem;
class UContextualAnimSceneAsset;
/**
 * 
 */
UENUM(BlueprintType)
enum class EWeaponTraceType : uint8
{
	Base,
	Middle,
	End
};

USTRUCT(BlueprintType)
struct FRangedWeaponSpec
{
	GENERATED_BODY()

	// Socket on the weapon mesh the bullet leaves from.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName MuzzleSocket = NAME_None;
	// How far a bullet travels along the lane before it gives up.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100.0")) float Range = 1500.f;
	// How far up or down, in degrees, the weapon can be aimed from the horizontal while aiming.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "85.0")) float MaxAimAngle = 45.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1")) int32 MagazineSize = 8;
	// The row in the weapon's moveset table that describes one shot: montage, damage, direction, impact feel.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ShotRow = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<UAnimMontage> ReloadMontage;
	// Optional beam or streak from the muzzle to where the bullet ended.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UNiagaraSystem> TracerFX;
	// The Niagara user parameter that receives the end point.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TracerEndParameter = "BeamEnd";
};

USTRUCT(BlueprintType)
struct FWeaponTracePoint
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponTraceType PointType = EWeaponTraceType::Base;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PointSocket = NAME_None;
};


UCLASS()
class GLA_COMBATSYSTEM_API UPDT_Weapon : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Item|Type")
	const FGameplayTag& GetWeaponTag() const { return WeaponTag; }

	UFUNCTION(BlueprintPure, Category = "Item|Moveset")
	UDataTable* GetWeaponDataTable() const { return WeaponDataTable; }

	UFUNCTION(BlueprintPure, Category = "Item|Visual")
	UStaticMesh* GetItemMesh() const { return ItemMesh; }

	UFUNCTION(BlueprintPure, Category = "Item|Visual")
	USkeletalMesh* GetItemSkeletalMesh() const { return ItemSkeletalMesh; }

	UFUNCTION(BlueprintPure, Category = "Item|Visual")
	FName GetAttachedSocket() const { return AttachedSocket; }

	UFUNCTION(BlueprintPure, Category = "Item|Visual")
	FName GetHolsterSocket() const { return HolsterSocket; }

	UFUNCTION(BlueprintPure, Category = "Item|Equip")
	UAnimMontage* GetDrawMontage() const { return DrawMontage; }

	UFUNCTION(BlueprintPure, Category = "Item|Equip")
	UAnimMontage* GetSheatheMontage() const { return SheatheMontage; }

	UFUNCTION(BlueprintPure, Category = "Item|Trace")
	const TArray<FWeaponTracePoint>& GetTracePoints() const { return TracePoints; }

	// Ranged means the weapon tag sits under Weapon.Type.Ranged.
	UFUNCTION(BlueprintPure, Category = "Item|Ranged")
	bool IsRanged() const;

	UFUNCTION(BlueprintPure, Category = "Item|Ranged")
	const FRangedWeaponSpec& GetRangedSpec() const { return RangedSpec; }
	
	UFUNCTION(BlueprintPure, Category = "Item|Trace")
	bool GetTrailSockets(FName& OutStart, FName& OutEnd) const;
	
	UFUNCTION(BlueprintPure, Category="Item | Animation")
	TSubclassOf<UAnimInstance> GetDefaultAnimLayers() const { return DefaultAnimLayers; }

	// FINISHERS: the contextual animation scenes this weapon executes with. A weapon with none cannot perform a finisher.
	UFUNCTION(BlueprintPure, Category = "Item|Finisher")
	const TArray<TSoftObjectPtr<UContextualAnimSceneAsset>>& GetFinisherScenes() const { return FinisherScenes; }

	// True when at least one finisher scene is set.
	UFUNCTION(BlueprintPure, Category = "Item|Finisher")
	bool HasFinishers() const;

	// GUARD: the montage that raises the guard high or low with this weapon.
	UFUNCTION(BlueprintPure, Category = "Item|Guard")
	UAnimMontage* GetGuardUpMontage() const { return GuardUpMontage; }

	UFUNCTION(BlueprintPure, Category = "Item|Guard")
	UAnimMontage* GetGuardDownMontage() const { return GuardDownMontage; }

	// COUNTERS: the table the counter attack after a successful parry draws from.
	UFUNCTION(BlueprintPure, Category = "Item|Counter")
	UDataTable* GetCounterStrikeDataTable() const { return CounterStrikeTable; }
	
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;

	// Keep bShowRangedSpec in step with the weapon tag: when the asset opens, when the tag changes, and on undo.
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
#endif

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Type")
	FGameplayTag WeaponTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Moveset",
		meta = (RequiredAssetDataTags = "RowStructure=/Script/GLA_CombatSystem.CombatStrikeRow"))
	UDataTable* WeaponDataTable = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Visual")
	TObjectPtr<UStaticMesh> ItemMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Visual")
	TObjectPtr<USkeletalMesh> ItemSkeletalMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Visual")
	FName AttachedSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Visual")
	FName HolsterSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Item|Equip")
	TObjectPtr<UAnimMontage> DrawMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Item|Equip")
	TObjectPtr<UAnimMontage> SheatheMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item|Trace")
	TArray<FWeaponTracePoint> TracePoints;

	// Only read when the weapon tag is under Weapon.Type.Ranged.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Ranged",
		meta = (EditCondition = "bShowRangedSpec", EditConditionHides))
	FRangedWeaponSpec RangedSpec;

	// Follows the weapon tag and only decides whether the Ranged Spec shows in the details panel. Never saved.
	UPROPERTY(Transient)
	bool bShowRangedSpec = false;

	// Soft references, so a scene's animations load only when the finisher is picked, not whenever the weapon asset loads.
	// Every scene must use the role names set on GA_Finisher (Attacker and Victim by default).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Finisher")
	TArray<TSoftObjectPtr<UContextualAnimSceneAsset>> FinisherScenes;

	// Both are needed for a full guard. A missing one falls back to the baseline weapon's, then to GA_Block's own.
	UPROPERTY(EditDefaultsOnly, Category = "Item|Guard")
	TObjectPtr<UAnimMontage> GuardUpMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Item|Guard")
	TObjectPtr<UAnimMontage> GuardDownMontage;

	// Rows are strikes, like the moveset table. A weapon without one falls back to the baseline's, then to the component's default.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Counter",
		meta = (RequiredAssetDataTags = "RowStructure=/Script/GLA_CombatSystem.CombatStrikeRow"))
	UDataTable* CounterStrikeTable = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item | Animation")
	TSubclassOf<UAnimInstance> DefaultAnimLayers; 
};
