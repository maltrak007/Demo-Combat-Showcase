// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindLanePoint.h"

#include "DrawDebugHelpers.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BlackboardComponent.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"

static TAutoConsoleVariable<bool> CVarDrawPatrolPoint(
	TEXT("Combat.DrawPatrolPoint"), false,
	TEXT("Draw the point chosen by BTTask_FindLanePoint."), ECVF_Cheat);

UBTTask_FindLanePoint::UBTTask_FindLanePoint()
{
	NodeName = TEXT("Find Lane Point");

	// Restrict the dropdowns to Vector keys.
	OriginKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindLanePoint, OriginKey));
	OriginKey.AllowNoneAsValue(true);
	ResultKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindLanePoint, ResultKey));
}

void UBTTask_FindLanePoint::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	// Custom nodes with their own key selectors must resolve them against the blackboard asset.
	if (const UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		OriginKey.ResolveSelectedKey(*BBAsset);
		ResultKey.ResolveSelectedKey(*BBAsset);
	}
}

EBTNodeResult::Type UBTTask_FindLanePoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	UNavigationSystemV1* NavSys = Pawn ? UNavigationSystemV1::GetCurrent(Pawn->GetWorld()) : nullptr;
	if (!Pawn || !BB || !NavSys || !ResultKey.IsSet()) return EBTNodeResult::Failed;

	const FVector Current = Pawn->GetActorLocation();
	const FVector Origin = OriginKey.IsSet() ? BB->GetValueAsVector(OriginKey.SelectedKeyName) : Current;

	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		// Y comes from the pawn, not the origin: the plane constraint pins it, so any other Y is unreachable.
		const FVector Candidate(Origin.X + FMath::FRandRange(-HalfWidth, HalfWidth), Current.Y, Origin.Z);
		if (FMath::Abs(Candidate.X - Current.X) < MinDistanceFromCurrent) continue;

		FNavLocation Projected;
		if (NavSys->ProjectPointToNavigation(Candidate, Projected, ProjectionExtent))
		{
			BB->SetValueAsVector(ResultKey.SelectedKeyName, Projected.Location);

			if (CVarDrawPatrolPoint.GetValueOnGameThread())
			{
				DrawDebugSphere(Pawn->GetWorld(), Projected.Location, 25.f, 12, FColor::Cyan, false, 3.f);
			}
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed; // the Sequence above falls through instead of getting stuck
}

FString UBTTask_FindLanePoint::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s = random X within %.0f of %s"),
		*ResultKey.SelectedKeyName.ToString(), HalfWidth,
		OriginKey.IsSet() ? *OriginKey.SelectedKeyName.ToString() : TEXT("the pawn"));
}