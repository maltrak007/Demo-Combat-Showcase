// Fill out your copyright notice in the Description page of Project Settings.


#include "PDT_Weapon.h"

#include "CombatData.h"
#include "Engine/DataTable.h"
#include "Misc/DataValidation.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"

bool UPDT_Weapon::IsRanged() const
{
	return WeaponTag.IsValid() && WeaponTag.MatchesTag(CombatTags::Weapon_Type_Ranged);
}

bool UPDT_Weapon::HasFinishers() const
{
	for (const TSoftObjectPtr<UContextualAnimSceneAsset>& Scene : FinisherScenes)
	{
		if (!Scene.IsNull()) return true;
	}
	return false;
}

bool UPDT_Weapon::GetTrailSockets(FName& OutStart, FName& OutEnd) const
{
	OutStart = NAME_None;
	OutEnd = NAME_None;
	for (const FWeaponTracePoint& Point : TracePoints)
	{
		if (Point.PointSocket.IsNone()) continue;
		if (Point.PointType == EWeaponTraceType::Base && OutStart.IsNone()) OutStart = Point.PointSocket;
		else if (Point.PointType == EWeaponTraceType::End && OutEnd.IsNone()) OutEnd = Point.PointSocket;
	}
	return !OutStart.IsNone() && !OutEnd.IsNone();
}

#if WITH_EDITOR
void UPDT_Weapon::PostLoad()
{
	Super::PostLoad();
	bShowRangedSpec = IsRanged();
}

void UPDT_Weapon::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	bShowRangedSpec = IsRanged();
}

void UPDT_Weapon::PostEditUndo()
{
	Super::PostEditUndo();
	bShowRangedSpec = IsRanged();
}

EDataValidationResult UPDT_Weapon::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	if (ItemMesh && ItemSkeletalMesh)
	{
		Context.AddError(FText::FromString(TEXT("Set a static mesh or a skeletal mesh, not both.")));
		Result = EDataValidationResult::Invalid;
	}
	if (!WeaponDataTable)
	{
		Context.AddError(FText::FromString(TEXT("A weapon needs a moveset table.")));
		Result = EDataValidationResult::Invalid;
	}
	auto HasSocket = [this](FName Socket)
	{
		if (ItemSkeletalMesh) return ItemSkeletalMesh->FindSocket(Socket) != nullptr;
		if (ItemMesh) return ItemMesh->FindSocket(Socket) != nullptr;
		return false;
	};
	for (const FWeaponTracePoint& Point : TracePoints)
	{
		if (Point.PointSocket.IsNone() || !HasSocket(Point.PointSocket))
		{
			Context.AddError(FText::Format(NSLOCTEXT("Weapon", "BadTraceSocket", "Trace point socket '{0}' doesn't exist on the weapon mesh."),
				FText::FromName(Point.PointSocket)));
			Result = EDataValidationResult::Invalid;
		}
	}
	if (IsRanged())
	{
		if (RangedSpec.MagazineSize < 1)
		{
			Context.AddError(FText::FromString(TEXT("A ranged weapon needs a magazine size of at least 1.")));
			Result = EDataValidationResult::Invalid;
		}
		if (RangedSpec.MuzzleSocket.IsNone() || !HasSocket(RangedSpec.MuzzleSocket))
		{
			Context.AddError(FText::Format(NSLOCTEXT("Weapon", "BadMuzzleSocket", "Muzzle socket '{0}' doesn't exist on the weapon mesh."),
				FText::FromName(RangedSpec.MuzzleSocket)));
			Result = EDataValidationResult::Invalid;
		}
		const FCombatStrikeRow* Shot = WeaponDataTable
			? WeaponDataTable->FindRow<FCombatStrikeRow>(RangedSpec.ShotRow, TEXT("PDT_Weapon"), false)
			: nullptr;
		if (!Shot || !Shot->Montage)
		{
			Context.AddError(FText::Format(NSLOCTEXT("Weapon", "BadShotRow", "ShotRow '{0}' must name a row of the moveset table that has a montage."),
				FText::FromName(RangedSpec.ShotRow)));
			Result = EDataValidationResult::Invalid;
		}
	}
	// Finishers: an empty slot is a mistake. No scenes at all is allowed, and just means this weapon cannot execute.
	for (int32 i = 0; i < FinisherScenes.Num(); ++i)
	{
		if (FinisherScenes[i].IsNull())
		{
			Context.AddError(FText::Format(NSLOCTEXT("Weapon", "EmptyFinisher", "Finisher scene {0} is empty."), FText::AsNumber(i)));
			Result = EDataValidationResult::Invalid;
		}
	}
	if (!HasFinishers())
	{
		Context.AddWarning(FText::FromString(TEXT("No finisher scenes. The player cannot execute enemies while this weapon is in use.")));
	}

	// Guard: the pair belongs together.
	if ((GuardUpMontage != nullptr) != (GuardDownMontage != nullptr))
	{
		Context.AddWarning(FText::FromString(TEXT("Only one guard montage is set. The missing direction falls back to the baseline weapon's.")));
	}

	// Counters: every row has to be playable, because the counter picks one at random.
	if (CounterStrikeTable)
	{
		if (CounterStrikeTable->GetRowStruct() != FCombatStrikeRow::StaticStruct())
		{
			Context.AddError(FText::FromString(TEXT("The counter table must use the Combat Strike Row structure.")));
			Result = EDataValidationResult::Invalid;
		}
		else
		{
			CounterStrikeTable->ForeachRow<FCombatStrikeRow>(TEXT("PDT_Weapon"), [&](const FName& RowName, const FCombatStrikeRow& Row)
			{
				if (!Row.Montage)
				{
					Context.AddError(FText::Format(NSLOCTEXT("Weapon", "CounterNoMontage", "Counter row '{0}' has no montage."), FText::FromName(RowName)));
					Result = EDataValidationResult::Invalid;
				}
			});
		}
	}
	else
	{
		Context.AddWarning(FText::FromString(TEXT("No counter table. A parry with this weapon uses the baseline's counters.")));
	}
	return Result;
}
#endif

