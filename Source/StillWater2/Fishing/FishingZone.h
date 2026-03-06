// Copyright StillWater2. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishData.h"
#include "FishingZone.generated.h"

class UBoxComponent;
class UFishingComponent;

/**
 * AFishingZone
 *
 * Place this actor in the world to define an area where the player can fish.
 * When a pawn with a UFishingComponent overlaps the zone's trigger volume,
 * the component is notified so the player can begin casting.
 *
 * Assign a UFishDataAsset (or manually fill the InlineFish array) to control
 * which fish species can be caught in this zone.
 */
UCLASS(Blueprintable)
class STILLWATER2_API AFishingZone : public AActor
{
	GENERATED_BODY()

public:
	AFishingZone();

	/** Returns the combined list of fish available in this zone. */
	UFUNCTION(BlueprintPure, Category = "Fishing")
	const TArray<FFishData>& GetAvailableFish() const;

protected:
	virtual void BeginPlay() override;

	// Overlap events
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	/** Trigger volume that detects the player. */
	UPROPERTY(VisibleAnywhere, Category = "Fishing")
	TObjectPtr<UBoxComponent> TriggerVolume;

	/** Optional data asset listing fish species for this zone. */
	UPROPERTY(EditAnywhere, Category = "Fishing")
	TObjectPtr<UFishDataAsset> FishDataAsset;

	/**
	 * Inline fish array — use this if you prefer per-zone overrides instead
	 * of a shared data asset. Both sources are merged at runtime.
	 */
	UPROPERTY(EditAnywhere, Category = "Fishing")
	TArray<FFishData> InlineFish;

	/** Merged fish list built at BeginPlay. */
	UPROPERTY()
	TArray<FFishData> CachedFishList;

	void BuildFishList();
};
