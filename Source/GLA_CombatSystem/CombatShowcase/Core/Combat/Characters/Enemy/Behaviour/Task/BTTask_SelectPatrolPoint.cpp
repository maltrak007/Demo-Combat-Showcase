// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SelectPatrolPoint.h"

#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CombatShowcase/Core/Combat/Components/NavigationComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<bool> CVarDrawPatrolRoute(
	TEXT("Combat.DrawPatrolRoute"), false,
	TEXT("Draw each enemy's patrol points and the one it is heading to."), ECVF_Cheat);

UBTTask_SelectPatrolPoint::UBTTask_SelectPatrolPoint()
{
	NodeName = TEXT("Select Patrol Point");
	ResultKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SelectPatrolPoint, ResultKey));
	IndexKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SelectPatrolPoint, IndexKey));
}

void UBTTask_SelectPatrolPoint::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (const UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		ResultKey.ResolveSelectedKey(*BBAsset);
		IndexKey.ResolveSelectedKey(*BBAsset);
	}
}

EBTNodeResult::Type UBTTask_SelectPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* PawnAgent = AIController ? Cast<APawn>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	UNavigationSystemV1* NavSys = PawnAgent ? UNavigationSystemV1::GetCurrent(PawnAgent->GetWorld()) : nullptr;
	if (!PawnAgent || !BB || !NavSys || !ResultKey.IsSet() || !IndexKey.IsSet()) return EBTNodeResult::Failed;
	
	const UNavigationComponent* NavComp = PawnAgent->FindComponentByClass<UNavigationComponent>();
	if (!NavComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: patrol needs a NavigationComponent."), *PawnAgent->GetName());
		return EBTNodeResult::Failed;
	}
	
	// Skip points that were never assigned or have been destroyed.
	TArray<FVector> Route;
	for (const AActor* Point : NavComp->GetPatrolPoints())
	{
		if (IsValid(Point)) Route.Add(Point->GetActorLocation());
	}
	if (Route.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: patrol needs at least two valid PatrolPoints."), *PawnAgent->GetName());
		return EBTNodeResult::Failed;
	}

	const FVector Current = PawnAgent->GetActorLocation();
	int32 Index = FMath::Max(0, BB->GetValueAsInt(IndexKey.SelectedKeyName)) % Route.Num();

	// Arrived at the current point: move on to the next. An interrupted trip (chase, hit) keeps
	// its index, so the enemy resumes toward the same point instead of skipping it.
	if (FMath::Abs(Route[Index].X - Current.X) <= ArrivalRadius)
	{
		Index = (Index + 1) % Route.Num();
	}
	BB->SetValueAsInt(IndexKey.SelectedKeyName, Index);

	// Y comes from the enemy: the plane constraint pins it, so a marker placed off the lane is unreachable.
	FNavLocation Projected;
	const FVector Wanted(Route[Index].X, Current.Y, Route[Index].Z);
	if (!NavSys->ProjectPointToNavigation(Wanted, Projected, ProjectionExtent))
	{
		return EBTNodeResult::Failed;
	}
	BB->SetValueAsVector(ResultKey.SelectedKeyName, Projected.Location);

	if (CVarDrawPatrolRoute.GetValueOnGameThread())
	{
		UWorld* World = PawnAgent->GetWorld();
		for (int32 i = 0; i < Route.Num(); ++i)
		{
			DrawDebugSphere(World, Route[i], 25.f, 12, i == Index ? FColor::Green : FColor::Silver, false, 3.f);
		}
		DrawDebugLine(World, Current, Projected.Location, FColor::Green, false, 3.f, 0, 2.f);
	}
	return EBTNodeResult::Succeeded;
}

FString UBTTask_SelectPatrolPoint::GetStaticDescription() const
{
	return FString::Printf(TEXT("next patrol point -> %s (route index in %s)"),
		*ResultKey.SelectedKeyName.ToString(), *IndexKey.SelectedKeyName.ToString());
}
