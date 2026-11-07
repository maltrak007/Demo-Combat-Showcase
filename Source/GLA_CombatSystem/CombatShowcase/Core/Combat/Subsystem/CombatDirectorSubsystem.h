// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Public/Subsystems/WorldSubsystem.h"
#include "CombatDirectorSubsystem.generated.h"

class UCombatDirectorSettings;

UENUM(BlueprintType)
enum class EAttackCategory : uint8 { Melee, Ranged };

UENUM(BlueprintType)
enum class EAttackSlot : uint8 { None, MeleeLeft, MeleeRight, Ranged };

UENUM(BlueprintType)
enum class EDirectorPhase : uint8
{
	BuildUp, // pressure ramping up, a few attackers
	Peak,    // full pressure
	Relax    // the player is hurting: pull attackers back and let them breathe
};

USTRUCT()
struct FAttackToken
{
	GENERATED_BODY()

	TWeakObjectPtr<AActor> Holder;
	TWeakObjectPtr<AActor> Target;
	EAttackSlot Slot = EAttackSlot::None;
	double GrantTime = 0.0;
};

/**
 * Hands out attack tokens so only a few enemies attack the player at once.
 * Slots: one melee token per side of the player, plus one ranged token.
 * Tuning lives in Project Settings > Combat Director.
 */
UCLASS()
class GLA_COMBATSYSTEM_API UCombatDirectorSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Asks for a token. If the requester already holds one, that one is returned. OutSlot is the slot granted. */
	UFUNCTION(BlueprintCallable, Category = "AI|Director")
	bool RequestToken(AActor* Requester, AActor* Target, EAttackCategory Category, EAttackSlot& OutSlot);

	/** Gives the token back. Call it when the attack ends, and when the holder dies. */
	UFUNCTION(BlueprintCallable, Category = "AI|Director")
	void ReleaseToken(AActor* Requester, bool bApplyCooldown = true);

	/** Gives the token back and keeps this enemy from asking again for exactly CooldownSeconds, whatever the global default. */
	UFUNCTION(BlueprintCallable, Category = "AI|Director")
	void ReleaseTokenWithCooldown(AActor* Requester, float CooldownSeconds);

	UFUNCTION(BlueprintPure, Category = "AI|Director")
	EAttackSlot GetSlot(const AActor* Requester) const;

	UFUNCTION(BlueprintPure, Category = "AI|Director")
	bool HasToken(const AActor* Requester) const { return GetSlot(Requester) != EAttackSlot::None; }

	/** -1 for the left slot, +1 for the right slot, 0 for ranged or no token. */
	static float SlotToSideSign(EAttackSlot Slot);

	UFUNCTION(BlueprintPure, Category = "AI|Director")
	FVector GetSideAxis() const;

	/** Called when the target loses health. DamageFraction is the share of its max health that was lost, 0 to 1. */
	UFUNCTION(BlueprintCallable, Category = "AI|Director")
	void NotifyTargetDamaged(AActor* Target, float DamageFraction);

	/** Manual limits. With bAutoDirector on, the next phase change overwrites them. */
	UFUNCTION(BlueprintCallable, Category = "AI|Director")
	void SetMaxTokens(int32 NewMaxMelee, int32 NewMaxRanged);

	UFUNCTION(BlueprintPure, Category = "AI|Director") EDirectorPhase GetPhase() const { return Phase; }
	UFUNCTION(BlueprintPure, Category = "AI|Director") float GetIntensity() const { return Intensity; }
	UFUNCTION(BlueprintPure, Category = "AI|Director") int32 GetMaxMeleeTokens() const { return MaxMeleeTokens; }
	UFUNCTION(BlueprintPure, Category = "AI|Director") int32 GetMaxRangedTokens() const { return MaxRangedTokens; }

private:
	const FAttackToken* FindToken(const AActor* Requester) const;
	bool IsSlotFree(const AActor* Target, EAttackSlot Slot) const;
	int32 CountSlots(const AActor* Target, bool bMelee) const;
	EAttackSlot SideOf(const AActor* Requester, const AActor* Target) const;
	bool IsOnCooldown(const AActor* Requester) const;

	void UpdatePhase(float DeltaTime, const UCombatDirectorSettings& Settings);
	void SetPhase(EDirectorPhase NewPhase, const UCombatDirectorSettings& Settings);
	void DrawDebug() const;

	TArray<FAttackToken> Tokens;
	TMap<TWeakObjectPtr<const AActor>, double> CooldownUntil;

	EDirectorPhase Phase = EDirectorPhase::BuildUp;
	float PhaseTime = 0.f;
	float Intensity = 0.f;
	int32 MaxMeleeTokens = 1;
	int32 MaxRangedTokens = 1;
};
