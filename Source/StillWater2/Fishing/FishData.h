// Copyright StillWater2. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FishData.generated.h"

/**
 * Rarity tiers for fish, affecting catch probability.
 */
UENUM(BlueprintType)
enum class EFishRarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	Legendary	UMETA(DisplayName = "Legendary")
};

/**
 * Data describing a single fish species.
 */
USTRUCT(BlueprintType)
struct FFishData
{
	GENERATED_BODY()

	/** Display name of the fish. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fish")
	FName FishName;

	/** Rarity tier. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fish")
	EFishRarity Rarity = EFishRarity::Common;

	/** Minimum weight in kilograms. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fish", meta = (ClampMin = "0.01"))
	float MinWeight = 0.5f;

	/** Maximum weight in kilograms. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fish", meta = (ClampMin = "0.01"))
	float MaxWeight = 5.0f;

	/** Base score / value awarded for catching this fish. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fish", meta = (ClampMin = "0"))
	int32 BaseScore = 10;

	/** Optional icon to display in UI. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fish")
	TSoftObjectPtr<UTexture2D> Icon;
};

/**
 * A DataAsset that holds an array of fish species available in a fishing zone.
 * Create instances via the Content Browser: right-click > Miscellaneous > Data Asset > FishDataAsset.
 */
UCLASS(BlueprintType)
class STILLWATER2_API UFishDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** All fish species that can be caught when this asset is assigned to a Fishing Zone. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fish")
	TArray<FFishData> Fish;
};
