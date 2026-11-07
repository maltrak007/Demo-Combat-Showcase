// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/EnvironmentQuery/Generators/EnvQueryGenerator_ProjectedPoints.h"
#include "Runtime/AIModule/Classes/DataProviders/AIDataProvider.h"
#include "EnvQueryGenerator_SideLine.generated.h"

class UEnvQueryContext;

/**
 * Points in a straight line along the lane (world X), centred on a context. The generators that ship with the
 * engine fill a plane, and this lane has no use for the points off the line.
 */
UCLASS(meta = (DisplayName = "Points: Side Line"))
class GLA_COMBATSYSTEM_API UEnvQueryGenerator_SideLine : public UEnvQueryGenerator_ProjectedPoints
{
	GENERATED_BODY()
public:
	UEnvQueryGenerator_SideLine();

	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;

	// Where the line is centred.
	UPROPERTY(EditDefaultsOnly, Category = "Generator") TSubclassOf<UEnvQueryContext> GenerateAround;
	// Distance from the centre to each end of the line.
	UPROPERTY(EditDefaultsOnly, Category = "Generator") FAIDataProviderFloatValue HalfLength;
	// Distance between two neighbouring points.
	UPROPERTY(EditDefaultsOnly, Category = "Generator") FAIDataProviderFloatValue SpaceBetween;
	// The lane direction. World X in this project.
	UPROPERTY(EditDefaultsOnly, Category = "Generator") FVector LaneAxis = FVector::ForwardVector;
};
