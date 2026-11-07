// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvQueryGenerator_SideLine.h"

#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "NavigationSystem.h"

#define LOCTEXT_NAMESPACE "SideLineGenerator"

UEnvQueryGenerator_SideLine::UEnvQueryGenerator_SideLine()
{
	ItemType = UEnvQueryItemType_Point::StaticClass();
	GenerateAround = UEnvQueryContext_Querier::StaticClass();
	HalfLength.DefaultValue = 900.f;
	SpaceBetween.DefaultValue = 75.f;
}

void UEnvQueryGenerator_SideLine::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	UObject* BindOwner = QueryInstance.Owner.Get();
	HalfLength.BindData(BindOwner, QueryInstance.QueryID);
	SpaceBetween.BindData(BindOwner, QueryInstance.QueryID);

	const float Half = FMath::Max(0.f, HalfLength.GetValue());
	const float Step = FMath::Max(10.f, SpaceBetween.GetValue()); // a floor, so a bad value can't make thousands of points
	const FVector Axis = LaneAxis.GetSafeNormal();

	TArray<FVector> Centres;
	QueryInstance.PrepareContext(GenerateAround, Centres);

	TArray<FNavLocation> Points;
	for (const FVector& Centre : Centres)
	{
		for (float Offset = -Half; Offset <= Half; Offset += Step)
		{
			Points.Add(FNavLocation(Centre + Axis * Offset));
		}
	}

	ProjectAndFilterNavPoints(Points, QueryInstance);
	StoreNavPoints(Points, QueryInstance);
}

FText UEnvQueryGenerator_SideLine::GetDescriptionTitle() const
{
	return FText::Format(LOCTEXT("Title", "{0}: points along the lane around {1}"),
		Super::GetDescriptionTitle(), UEnvQueryTypes::DescribeContext(GenerateAround));
}

FText UEnvQueryGenerator_SideLine::GetDescriptionDetails() const
{
	return FText::Format(LOCTEXT("Details", "half length {0}, spacing {1}"),
		FText::FromString(HalfLength.ToString()), FText::FromString(SpaceBetween.ToString()));
}

#undef LOCTEXT_NAMESPACE
