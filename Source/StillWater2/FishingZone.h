// FishingZone.h — Place this actor over water to mark fishable areas

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "FishDataAsset.h"
#include "FishingZone.generated.h"

UCLASS(BlueprintType, Blueprintable)
class STILLWATER2_API AFishingZone : public AActor
{
	GENERATED_BODY()

public:
	AFishingZone();

	/** The trigger volume defining the fishable area */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fishing")
	TObjectPtr<UBoxComponent> ZoneTrigger;

	/** Fish that can be caught in this zone */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fishing")
	TArray<TObjectPtr<UFishDataAsset>> AvailableFish;

	/** Min seconds before a fish bites after casting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fishing", meta = (ClampMin = "0.5"))
	float MinBiteTime = 2.0f;

	/** Max seconds before a fish bites after casting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fishing", meta = (ClampMin = "0.5"))
	float MaxBiteTime = 8.0f;

	/** Pick a random fish from this zone's table using weighted random selection */
	UFUNCTION(BlueprintCallable, Category = "Fishing")
	UFishDataAsset* PickRandomFish() const;

	/** Get a random bite delay */
	UFUNCTION(BlueprintCallable, Category = "Fishing")
	float GetRandomBiteDelay() const;
};
