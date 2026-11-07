// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_RequestAttackToken.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Characters/Enemy/EnemyBaseCharacter.h"
#include "CombatShowcase/Core/Combat/Components/EnemyBehaviourComponent.h"
#include "CombatShowcase/Core/Combat/Components/WeaponComponent.h"
#include "CombatShowcase/Core/Combat/Subsystem/CombatDirectorSubsystem.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "CombatShowcase/Core/Combat/Characters/Enemy/Behaviour/BehaviourData.h"

UBTService_RequestAttackToken::UBTService_RequestAttackToken()
{
	NodeName = TEXT("Request Attack Token");
	Interval = 0.25f;
	RandomDeviation = 0.05f;
	bNotifyTick = true;
	bNotifyCeaseRelevant = true;

	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RequestAttackToken, TargetKey), AActor::StaticClass());
	HasTokenKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RequestAttackToken, HasTokenKey));
	SideSignKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RequestAttackToken, SideSignKey));
	AttackLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RequestAttackToken, AttackLocationKey));
	RangeMinKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RequestAttackToken, RangeMinKey));
	RangeMaxKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RequestAttackToken, RangeMaxKey));

	// The names the project's blackboard uses, so a new node works without touching the dropdowns.
	TargetKey.SelectedKeyName = EnemyBBKeys::TargetActor;
	HasTokenKey.SelectedKeyName = EnemyBBKeys::HasAttackToken;
	SideSignKey.SelectedKeyName = EnemyBBKeys::AttackSideSign;
	AttackLocationKey.SelectedKeyName = EnemyBBKeys::AttackLocation;
	RangeMinKey.SelectedKeyName = EnemyBBKeys::PreferredRangeMin;
	RangeMaxKey.SelectedKeyName = EnemyBBKeys::PreferredRangeMax;
}

void UBTService_RequestAttackToken::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (const UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetKey.ResolveSelectedKey(*BBAsset);
		HasTokenKey.ResolveSelectedKey(*BBAsset);
		SideSignKey.ResolveSelectedKey(*BBAsset);
		AttackLocationKey.ResolveSelectedKey(*BBAsset);
		RangeMinKey.ResolveSelectedKey(*BBAsset);
		RangeMaxKey.ResolveSelectedKey(*BBAsset);
	}
}

EAttackCategory UBTService_RequestAttackToken::ResolveCategory(const APawn* Pawn) const
{
	const ABaseCombatCharacter* Character = Cast<ABaseCombatCharacter>(Pawn);
	const UWeaponComponent* Weapons = Character ? Character->GetWeaponComponent() : nullptr;
	const FGameplayTag WeaponTag = Weapons ? Weapons->GetCarriedWeaponTag() : FGameplayTag();

	// No weapon (a brawler) falls through to melee.
	return (WeaponTag.IsValid() && WeaponTag.MatchesTag(CombatTags::Weapon_Type_Ranged))
		       ? EAttackCategory::Ranged
		       : EAttackCategory::Melee;
}

void UBTService_RequestAttackToken::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	UWorld* World = OwnerComp.GetWorld();
	UCombatDirectorSubsystem* Director = World ? World->GetSubsystem<UCombatDirectorSubsystem>() : nullptr;
	if (!Pawn || !BB || !Director) return;

	AActor* Target = TargetKey.IsSet() ? Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName)) : nullptr;

	EAttackSlot Slot = EAttackSlot::None;
	bool bHasToken = false;

	if (Target)
	{
		const EAttackCategory Category = ResolveCategory(Pawn);

		// The weapon changed under a held token (drew or sheathed a different kind): hand it back and ask again.
		const EAttackSlot Held = Director->GetSlot(Pawn);
		if (Held != EAttackSlot::None && (Held == EAttackSlot::Ranged) != (Category == EAttackCategory::Ranged))
		{
			Director->ReleaseToken(Pawn, false);
		}
		bHasToken = Director->RequestToken(Pawn, Target, Category, Slot);
	}
	else
	{
		Director->ReleaseToken(Pawn, false); // lost the target: nothing to attack, so nothing to hold
	}

	const float Side = UCombatDirectorSubsystem::SlotToSideSign(Slot);

	if (HasTokenKey.IsSet()) BB->SetValueAsBool(HasTokenKey.SelectedKeyName, bHasToken);
	if (SideSignKey.IsSet()) BB->SetValueAsFloat(SideSignKey.SelectedKeyName, Side);

	// The enemy's own numbers for this weapon and skill level, when it has a behaviour component.
	const AEnemyBaseCharacter* Enemy = Cast<AEnemyBaseCharacter>(Pawn);
	const UEnemyBehaviourComponent* Behaviour = Enemy ? Enemy->GetBehaviourComponent() : nullptr;
	const FWeaponBehaviourRow* Stats = Behaviour ? &Behaviour->GetStats() : nullptr;
	if (Stats)
	{
		if (RangeMinKey.IsSet()) BB->SetValueAsFloat(RangeMinKey.SelectedKeyName, Stats->PreferredRangeMin);
		if (RangeMaxKey.IsSet()) BB->SetValueAsFloat(RangeMaxKey.SelectedKeyName, Stats->PreferredRangeMax);
	}
	const float StandOff = (Stats && Stats->MeleeStandOffDistance > 0.f) ? Stats->MeleeStandOffDistance : MeleeStandOffDistance;

	// A melee token has a fixed spot beside the target. Y and Z come from the pawn: the plane constraint pins it.
	if (bHasToken && Target && Side != 0.f && AttackLocationKey.IsSet())
	{
		FVector Spot = Target->GetActorLocation() + Director->GetSideAxis() * Side * StandOff;
		Spot.Y = Pawn->GetActorLocation().Y;
		Spot.Z = Pawn->GetActorLocation().Z;
		BB->SetValueAsVector(AttackLocationKey.SelectedKeyName, Spot);
	}
}

void UBTService_RequestAttackToken::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Leaving the combat branch gives the slot back. No cooldown: it didn't attack, it just left.
	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (APawn* Pawn = AIController ? AIController->GetPawn() : nullptr)
	{
		if (UCombatDirectorSubsystem* Director = OwnerComp.GetWorld()->GetSubsystem<UCombatDirectorSubsystem>())
		{
			Director->ReleaseToken(Pawn, false);
		}
	}
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		if (HasTokenKey.IsSet()) BB->SetValueAsBool(HasTokenKey.SelectedKeyName, false);
		if (SideSignKey.IsSet()) BB->SetValueAsFloat(SideSignKey.SelectedKeyName, 0.f);
	}
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

FString UBTService_RequestAttackToken::GetStaticDescription() const
{
	return FString::Printf(TEXT("target: %s\ntoken -> %s, stand-off %.0f\n%s"),
		*TargetKey.SelectedKeyName.ToString(), *HasTokenKey.SelectedKeyName.ToString(), MeleeStandOffDistance,
		*Super::GetStaticDescription());
}
