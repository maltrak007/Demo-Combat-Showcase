// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"

#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "CombatComboComponent.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Data/PDT_Weapon.h"

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	ApplyMoveset(Baseline); // the baseline moveset is active from the start
	LinkAnimLayers(Baseline); // and so are its poses, before anything is drawn
	if (StartingCarried && SetCarried(StartingCarried) && bStartDrawn) Draw();
}

FGameplayTag UWeaponComponent::GetCarriedWeaponTag() const
{
	return Carried ? Carried->GetWeaponTag() : FGameplayTag();
}

const UPDT_Weapon* UWeaponComponent::GetActiveWeapon() const
{
	return (bDrawn && Carried) ? Carried.Get() : Baseline.Get();
}

UAnimMontage* UWeaponComponent::ResolveGuardMontage(bool bUp) const
{
	const UPDT_Weapon* Active = GetActiveWeapon();
	UAnimMontage* Montage = Active ? (bUp ? Active->GetGuardUpMontage() : Active->GetGuardDownMontage()) : nullptr;
	if (!Montage && Baseline && Baseline.Get() != Active)
	{
		Montage = bUp ? Baseline->GetGuardUpMontage() : Baseline->GetGuardDownMontage();
	}
	return Montage;
}

bool UWeaponComponent::IsRangedDrawn() const
{
	return bDrawn && Carried && Carried->IsRanged();
}

const FRangedWeaponSpec* UWeaponComponent::GetRangedSpec() const
{
	return (Carried && Carried->IsRanged()) ? &Carried->GetRangedSpec() : nullptr;
}

bool UWeaponComponent::IsMagazineFull() const
{
	const FRangedWeaponSpec* Spec = GetRangedSpec();
	return !Spec || Ammo >= Spec->MagazineSize;
}

bool UWeaponComponent::ConsumeAmmo()
{
	if (Ammo <= 0) return false;
	--Ammo;
	return true;
}

void UWeaponComponent::RefillAmmo()
{
	if (const FRangedWeaponSpec* Spec = GetRangedSpec()) Ammo = Spec->MagazineSize;
}

void UWeaponComponent::SetBaseline(UPDT_Weapon* Weapon)
{
	Baseline = Weapon;
	if (!bDrawn)
	{
		ApplyMoveset(Baseline);
		LinkAnimLayers(Baseline);
	}
}

void UWeaponComponent::ApplyMoveset(const UPDT_Weapon* Weapon) const
{
	const ABaseCombatCharacter* Owner = Cast<ABaseCombatCharacter>(GetOwner());
	UCombatComboComponent* Combo = Owner ? Owner->GetCombatComboComponent() : nullptr;
	if (!Combo) return;
	if (!Weapon || !Weapon->GetWeaponDataTable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: no moveset to apply. Is the Baseline weapon assigned?"), *GetNameSafe(GetOwner()));
		return;
	}
	Combo->SetStrikeDataTable(Weapon->GetWeaponDataTable()); // also resets combo state, so no stale row survives a swap

	// Counters follow the weapon too. Without its own table a weapon uses the baseline's. With neither, a null here
	// makes the combo component fall back to its own default, so a previous weapon's counters never linger.
	UDataTable* Counters = Weapon->GetCounterStrikeDataTable();
	if (!Counters && Baseline) Counters = Baseline->GetCounterStrikeDataTable();
	Combo->SetCounterStrikeDataTable(Counters);
}

bool UWeaponComponent::SetCarried(UPDT_Weapon* Weapon)
{
	ABaseCombatCharacter* Owner = Cast<ABaseCombatCharacter>(GetOwner());
	if (!Owner || !Weapon) return false;

	// Everything is validated before any state changes, so a bad asset can't leave the character half-changed.
	USkeletalMesh* SkeletalAsset = Weapon->GetItemSkeletalMesh();
	UStaticMesh* StaticAsset = Weapon->GetItemMesh();
	if (!Weapon->GetWeaponDataTable() || (!SkeletalAsset && !StaticAsset))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetCarried %s: needs a moveset table and a mesh."), *GetNameSafe(Weapon));
		return false;
	}
	USkeletalMeshComponent* CharacterMesh = Owner->GetMesh();
	if (!CharacterMesh->DoesSocketExist(Weapon->GetAttachedSocket()) || !CharacterMesh->DoesSocketExist(Weapon->GetHolsterSocket()))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetCarried %s: hand or holster socket not found on %s."), *GetNameSafe(Weapon), *Owner->GetName());
		return false;
	}

	if (bDrawn) Sheathe(); 
	if (CarriedMesh) CarriedMesh->DestroyComponent();

	UMeshComponent* Mesh = nullptr;
	if (SkeletalAsset)
	{
		USkeletalMeshComponent* Component = NewObject<USkeletalMeshComponent>(Owner);
		Component->SetSkeletalMeshAsset(SkeletalAsset);
		Mesh = Component;
	}
	else
	{
		UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Owner);
		Component->SetStaticMesh(StaticAsset);
		Mesh = Component;
	}
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // hits come from sweeps
	Mesh->RegisterComponent();

	CarriedMesh = Mesh;
	Carried = Weapon;
	Ammo = Weapon->IsRanged() ? Weapon->GetRangedSpec().MagazineSize : 0; // a new weapon arrives loaded
	AttachCarried(Weapon->GetHolsterSocket()); // visible on the holster even while sheathed
	return true;
}

void UWeaponComponent::LinkAnimLayers(const UPDT_Weapon* Weapon)
{
	const ABaseCombatCharacter* Owner = Cast<ABaseCombatCharacter>(GetOwner());
	USkeletalMeshComponent* Mesh = Owner ? Owner->GetMesh() : nullptr;
	if (!Mesh || !Mesh->GetAnimInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: no anim instance on the mesh, so no anim layers were linked."), *GetNameSafe(GetOwner()));
		return;
	}

	// A weapon with no layers of its own borrows the baseline's, so the character is never left on stale poses.
	TSubclassOf<UAnimInstance> Layers = Weapon ? Weapon->GetDefaultAnimLayers() : TSubclassOf<UAnimInstance>();
	if (!Layers && Weapon && Weapon != Baseline)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no Default Anim Layers. Using the baseline's."), *GetNameSafe(Weapon));
		Layers = Baseline ? Baseline->GetDefaultAnimLayers() : TSubclassOf<UAnimInstance>();
	}
	if (!Layers)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: no anim layers to link. Set Default Anim Layers on the weapon asset."), *GetNameSafe(GetOwner()));
		return;
	}

	// Already in place. The instance check also catches a mesh whose anim class was swapped at runtime.
	if (Layers == LinkedLayers && Mesh->GetLinkedAnimLayerInstanceByClass(Layers)) return;

	if (LinkedLayers) Mesh->UnlinkAnimClassLayers(LinkedLayers);
	Mesh->LinkAnimClassLayers(Layers);
	LinkedLayers = Layers;
}

void UWeaponComponent::AttachCarried(FName Socket) const
{
	const ABaseCombatCharacter* Owner = Cast<ABaseCombatCharacter>(GetOwner());
	if (Owner && CarriedMesh)
	{
		CarriedMesh->AttachToComponent(Owner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
	}
}

bool UWeaponComponent::Draw()
{
	ABaseCombatCharacter* Owner = Cast<ABaseCombatCharacter>(GetOwner());
	if (!Owner || bDrawn || !Carried) return false;

	AttachCarried(Carried->GetAttachedSocket());
	Owner->SetEquippedWeaponMesh(CarriedMesh);
	ApplyMoveset(Carried);
	LinkAnimLayers(Carried); // linked before the delegate fires, so a listener already sees the new poses
	bDrawn = true;
	OnDrawnChanged.Broadcast(true);
	return true;
}

bool UWeaponComponent::Sheathe()
{
	ABaseCombatCharacter* Owner = Cast<ABaseCombatCharacter>(GetOwner());
	if (!Owner || !bDrawn || !Carried) return false;

	AttachCarried(Carried->GetHolsterSocket());
	Owner->SetEquippedWeaponMesh(nullptr);
	ApplyMoveset(Baseline);
	bDrawn = false;
	LinkAnimLayers(Baseline);
	OnDrawnChanged.Broadcast(false);
	return true;
}