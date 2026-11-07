// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Runtime/Engine/Classes/Components/ActorComponent.h"
#include "WeaponComponent.generated.h"


class UDataTable;
class UMeshComponent;
class UPDT_Weapon;
class UAnimInstance;
class UAnimMontage;
struct FRangedWeaponSpec;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnDrawnChanged,
	bool,
	bDrawn
);
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GLA_COMBATSYSTEM_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent() { PrimaryComponentTick.bCanEverTick = false; }
	
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsDrawn() const { return bDrawn; }
	
	UFUNCTION(BlueprintPure, Category = "Weapon")
	const UPDT_Weapon* GetCarried() const { return Carried; }

	// The weapon asset's own tag, or an empty tag when nothing is carried (unarmed). Drawn or sheathed makes no difference.
	FGameplayTag GetCarriedWeaponTag() const;

	// The weapon whose moveset is live right now: the carried one while drawn, the baseline otherwise.
	// Finishers, guards and counters all ask this, so they can never disagree about what is equipped.
	UFUNCTION(BlueprintPure, Category = "Weapon")
	const UPDT_Weapon* GetActiveWeapon() const;

	const UPDT_Weapon* GetBaseline() const { return Baseline; }

	// The guard montage of the active weapon, then the baseline's. Null when neither names one.
	UAnimMontage* ResolveGuardMontage(bool bUp) const;

	// True only while a ranged weapon is in hand. Sheathed, the baseline (melee) moveset is active.
	bool IsRangedDrawn() const;
	// The carried weapon's ranged data, or null when it isn't a ranged weapon.
	const FRangedWeaponSpec* GetRangedSpec() const;

	// Rounds left in the magazine of the carried weapon.
	int32 GetAmmo() const { return Ammo; }
	bool HasAmmo() const { return Ammo > 0; }
	bool IsMagazineFull() const;
	// Takes one round. False when the magazine is empty.
	bool ConsumeAmmo();
	void RefillAmmo();

	// What the character owns. Spawns its mesh at the holster socket. An inventory would call this.
	bool SetCarried(UPDT_Weapon* Weapon);
	// For archetypes. If nothing is drawn, the baseline moveset is applied at once.
	void SetBaseline(UPDT_Weapon* Weapon);
	
	// The two commit points. GA_EquipWeapon calls them at its notify, and enemies call them at spawn.
	bool Draw();
	bool Sheathe();
	
	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnDrawnChanged OnDrawnChanged;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon") TObjectPtr<UPDT_Weapon> Baseline;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon") TObjectPtr<UPDT_Weapon> StartingCarried;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon") bool bStartDrawn = false;
	
private:
	void ApplyMoveset(const UPDT_Weapon* Weapon) const;
	void AttachCarried(FName Socket) const;
	// Points the character's linked anim layers at the weapon's Default Anim Layers. Null means the baseline's.
	void LinkAnimLayers(const UPDT_Weapon* Weapon);

	UPROPERTY(Transient) TObjectPtr<UPDT_Weapon> Carried;
	UPROPERTY(Transient) TObjectPtr<UMeshComponent> CarriedMesh;
	bool bDrawn = false;
	int32 Ammo = 0;

	// The layer class currently linked on the mesh, so a swap can unlink it first.
	UPROPERTY(Transient) TSubclassOf<UAnimInstance> LinkedLayers;
};
