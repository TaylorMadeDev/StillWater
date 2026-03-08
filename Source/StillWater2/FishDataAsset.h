// FishDataAsset.h — Data asset defining a type of fish

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FishDataAsset.generated.h"

UENUM(BlueprintType)
enum class EFishRarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	Legendary	UMETA(DisplayName = "Legendary")
};

UCLASS(BlueprintType)
class STILLWATER2_API UFishDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Display name of this fish */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fish")
	FText FishName;

	/** Rarity tier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fish")
	EFishRarity Rarity = EFishRarity::Common;

	/** Catch weight for the weighted random table (higher = more likely) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fish", meta = (ClampMin = "0.01"))
	float CatchWeight = 1.0f;

	/** Weight range in kg */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fish", meta = (ClampMin = "0.01"))
	float MinWeight = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fish", meta = (ClampMin = "0.01"))
	float MaxWeight = 1.0f;

	/** Optional sprite to show when fish is caught (assign in Blueprint) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fish")
	TSoftObjectPtr<UTexture2D> FishSprite;
};
