// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "BaseCombatCharacter.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class UCombatAttributeSet;

UCLASS()
class GLA_COMBATSYSTEM_API ABaseCombatCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCombatCharacter();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	
	//WEAPON RELATED
	UStaticMeshComponent* GetEquippedWeaponMesh() const { return EquippedWeaponMesh; }
	
	void SetEquippedWeaponMesh(UStaticMeshComponent* NewWeaponMesh) { EquippedWeaponMesh = NewWeaponMesh; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UCombatAttributeSet* AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TArray<TSubclassOf<UGameplayAbility>> StartingAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> DefaultAttributesEffect;
	
	// Set by whatever equips a weapon later. Null = bare-handed. Not building the equip
	// system itself here — this is just the seam it plugs into.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UStaticMeshComponent* EquippedWeaponMesh = nullptr;
	
	virtual void PossessedBy(AController* NewController) override;
	
	void FaceDirection(float SignedDirection);
	
private:
	void InitializeAttributes();
	
	void GrantStartingAbilities();
	
};
