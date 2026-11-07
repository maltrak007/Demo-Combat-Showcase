#include "CombatDirectorSubsystem.h"

#include "CombatDirectorSettings.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"

static TAutoConsoleVariable<bool> CVarDrawDirector(
	TEXT("Combat.DrawDirector"), false,
	TEXT("Draw who holds which attack token, plus the director's phase and intensity."), ECVF_Cheat);

namespace
{
	// A dead combatant can't hold or be the target of a token. Non-combat actors count as alive.
	bool IsAlive(const AActor* Actor)
	{
		if (!IsValid(Actor)) return false;
		const ABaseCombatCharacter* Character = Cast<ABaseCombatCharacter>(Actor);
		return !Character || Character->GetHealthPercent() > 0.f;
	}
}

bool UCombatDirectorSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld() && Super::ShouldCreateSubsystem(Outer);
}

void UCombatDirectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	SetPhase(EDirectorPhase::BuildUp, *GetDefault<UCombatDirectorSettings>());
}

TStatId UCombatDirectorSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCombatDirectorSubsystem, STATGROUP_Tickables);
}

FVector UCombatDirectorSubsystem::GetSideAxis() const
{
	return GetDefault<UCombatDirectorSettings>()->SideAxis.GetSafeNormal();
}

float UCombatDirectorSubsystem::SlotToSideSign(EAttackSlot Slot)
{
	switch (Slot)
	{
	case EAttackSlot::MeleeLeft:  return -1.f;
	case EAttackSlot::MeleeRight: return  1.f;
	default:                      return  0.f;
	}
}

const FAttackToken* UCombatDirectorSubsystem::FindToken(const AActor* Requester) const
{
	return Tokens.FindByPredicate([Requester](const FAttackToken& T) { return T.Holder == Requester; });
}

EAttackSlot UCombatDirectorSubsystem::GetSlot(const AActor* Requester) const
{
	const FAttackToken* T = FindToken(Requester);
	return T ? T->Slot : EAttackSlot::None;
}

bool UCombatDirectorSubsystem::IsSlotFree(const AActor* Target, EAttackSlot Slot) const
{
	return !Tokens.ContainsByPredicate([&](const FAttackToken& T)
	{
		return T.Target == Target && T.Slot == Slot && T.Holder.IsValid();
	});
}

int32 UCombatDirectorSubsystem::CountSlots(const AActor* Target, bool bMelee) const
{
	int32 Count = 0;
	for (const FAttackToken& T : Tokens)
	{
		const bool bIsMelee = (T.Slot == EAttackSlot::MeleeLeft || T.Slot == EAttackSlot::MeleeRight);
		if (T.Target == Target && bIsMelee == bMelee) { ++Count; }
	}
	return Count;
}

EAttackSlot UCombatDirectorSubsystem::SideOf(const AActor* Requester, const AActor* Target) const
{
	const float D = FVector::DotProduct(Requester->GetActorLocation() - Target->GetActorLocation(), GetSideAxis());
	return D >= 0.f ? EAttackSlot::MeleeRight : EAttackSlot::MeleeLeft;
}

bool UCombatDirectorSubsystem::IsOnCooldown(const AActor* Requester) const
{
	const double* Until = CooldownUntil.Find(Requester);
	return Until && GetWorld()->GetTimeSeconds() < *Until;
}

bool UCombatDirectorSubsystem::RequestToken(AActor* Requester, AActor* Target, EAttackCategory Category, EAttackSlot& OutSlot)
{
	OutSlot = EAttackSlot::None;
	if (!IsAlive(Requester) || !IsAlive(Target)) { return false; }

	if (const FAttackToken* Existing = FindToken(Requester))
	{
		OutSlot = Existing->Slot;
		return true;
	}
	if (IsOnCooldown(Requester)) { return false; }

	EAttackSlot Granted = EAttackSlot::None;

	if (Category == EAttackCategory::Ranged)
	{
		if (CountSlots(Target, false) < MaxRangedTokens && IsSlotFree(Target, EAttackSlot::Ranged))
		{
			Granted = EAttackSlot::Ranged;
		}
	}
	else if (CountSlots(Target, true) < MaxMeleeTokens)
	{
		const EAttackSlot Own = SideOf(Requester, Target);
		const EAttackSlot Other = (Own == EAttackSlot::MeleeLeft) ? EAttackSlot::MeleeRight : EAttackSlot::MeleeLeft;

		if (IsSlotFree(Target, Own)) { Granted = Own; }
		else if (GetDefault<UCombatDirectorSettings>()->bAllowCrossSide && IsSlotFree(Target, Other)) { Granted = Other; }
	}

	if (Granted == EAttackSlot::None) { return false; }

	FAttackToken& New = Tokens.AddDefaulted_GetRef();
	New.Holder = Requester;
	New.Target = Target;
	New.Slot = Granted;
	New.GrantTime = GetWorld()->GetTimeSeconds();

	OutSlot = Granted;
	return true;
}

void UCombatDirectorSubsystem::ReleaseToken(AActor* Requester, bool bApplyCooldown)
{
	if (!Requester) { return; }
	const int32 Removed = Tokens.RemoveAll([Requester](const FAttackToken& T) { return T.Holder == Requester; });
	if (Removed > 0 && bApplyCooldown)
	{
		CooldownUntil.Add(Requester, GetWorld()->GetTimeSeconds() + GetDefault<UCombatDirectorSettings>()->CooldownAfterRelease);
	}
}

void UCombatDirectorSubsystem::ReleaseTokenWithCooldown(AActor* Requester, float CooldownSeconds)
{
	if (!Requester) { return; }
	const int32 Removed = Tokens.RemoveAll([Requester](const FAttackToken& T) { return T.Holder == Requester; });
	if (Removed > 0 && CooldownSeconds > 0.f)
	{
		CooldownUntil.Add(Requester, GetWorld()->GetTimeSeconds() + CooldownSeconds);
	}
}

void UCombatDirectorSubsystem::NotifyTargetDamaged(AActor* Target, float DamageFraction)
{
	if (!Target || DamageFraction <= 0.f) { return; }
	const UCombatDirectorSettings* Settings = GetDefault<UCombatDirectorSettings>();
	Intensity = FMath::Clamp(Intensity + DamageFraction * Settings->DamageToIntensity, 0.f, 1.f);
}

void UCombatDirectorSubsystem::SetMaxTokens(int32 NewMaxMelee, int32 NewMaxRanged)
{
	MaxMeleeTokens = FMath::Clamp(NewMaxMelee, 0, 2);
	MaxRangedTokens = FMath::Clamp(NewMaxRanged, 0, 1);
}

void UCombatDirectorSubsystem::SetPhase(EDirectorPhase NewPhase, const UCombatDirectorSettings& Settings)
{
	Phase = NewPhase;
	PhaseTime = 0.f;

	const FDirectorPhaseLimits& Limits = (NewPhase == EDirectorPhase::Peak) ? Settings.PeakLimits
	                                   : (NewPhase == EDirectorPhase::Relax) ? Settings.RelaxLimits
	                                   : Settings.BuildUpLimits;
	SetMaxTokens(Limits.MaxMelee, Limits.MaxRanged);
}

void UCombatDirectorSubsystem::UpdatePhase(float DeltaTime, const UCombatDirectorSettings& Settings)
{
	Intensity = FMath::Max(0.f, Intensity - Settings.IntensityDecayPerSecond * DeltaTime);
	PhaseTime += DeltaTime;

	switch (Phase)
	{
	case EDirectorPhase::BuildUp:
		if (Intensity >= Settings.RelaxEnterIntensity)       SetPhase(EDirectorPhase::Relax, Settings);
		else if (PhaseTime >= Settings.BuildUpDuration)      SetPhase(EDirectorPhase::Peak, Settings);
		break;
	case EDirectorPhase::Peak:
		if (Intensity >= Settings.RelaxEnterIntensity || PhaseTime >= Settings.PeakDuration)
			SetPhase(EDirectorPhase::Relax, Settings);
		break;
	case EDirectorPhase::Relax:
		if (PhaseTime >= Settings.RelaxMinDuration && Intensity <= Settings.RelaxExitIntensity)
			SetPhase(EDirectorPhase::BuildUp, Settings);
		break;
	}
}

void UCombatDirectorSubsystem::Tick(float DeltaTime)
{
	const UCombatDirectorSettings* Settings = GetDefault<UCombatDirectorSettings>();
	const double Now = GetWorld()->GetTimeSeconds();

	Tokens.RemoveAll([&](const FAttackToken& T)
	{
		if (!IsAlive(T.Holder.Get()) || !IsAlive(T.Target.Get())) { return true; } // dead or destroyed
		if (Now - T.GrantTime > Settings->TokenTimeout)                           // stuck
		{
			CooldownUntil.Add(T.Holder.Get(), Now + Settings->CooldownAfterRelease);
			return true;
		}
		return false;
	});

	for (auto It = CooldownUntil.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid() || Now >= It.Value()) { It.RemoveCurrent(); }
	}

	if (Settings->bAutoDirector) { UpdatePhase(DeltaTime, *Settings); }
	if (CVarDrawDirector.GetValueOnGameThread()) { DrawDebug(); }
}

void UCombatDirectorSubsystem::DrawDebug() const
{
	UWorld* World = GetWorld();
	for (const FAttackToken& T : Tokens)
	{
		if (const AActor* Holder = T.Holder.Get())
		{
			DrawDebugString(World, Holder->GetActorLocation() + FVector(0.f, 0.f, 160.f),
				UEnum::GetValueAsString(T.Slot), nullptr, FColor::Orange, 0.f, true);
		}
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(7001, 0.f, FColor::Yellow,
			FString::Printf(TEXT("Director: %s  intensity %.2f  tokens %d melee / %d ranged  held %d"),
				*UEnum::GetValueAsString(Phase), Intensity, MaxMeleeTokens, MaxRangedTokens, Tokens.Num()));
	}
}
