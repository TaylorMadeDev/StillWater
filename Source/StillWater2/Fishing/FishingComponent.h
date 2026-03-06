// Copyright StillWater2. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FishData.h"
#include "FishingComponent.generated.h"

/**
 * States of the fishing state machine.
 */
UENUM(BlueprintType)
enum class EFishingState : uint8
{
	/** Player is not fishing. */
	Idle		UMETA(DisplayName = "Idle"),
	/** Line is being cast. */
	Casting		UMETA(DisplayName = "Casting"),
	/** Waiting for a fish to bite. */
	Waiting		UMETA(DisplayName = "Waiting"),
	/** A fish is biting – player must start reeling. */
	Biting		UMETA(DisplayName = "Biting"),
	/** Player is reeling the fish in. */
	Reeling		UMETA(DisplayName = "Reeling"),
	/** Fish has been caught (brief celebration state). */
	Caught		UMETA(DisplayName = "Caught")
};

// ---------------------------------------------------------------------------
// Delegates
// ---------------------------------------------------------------------------

/** Broadcast whenever the fishing state changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFishingStateChanged, EFishingState, OldState, EFishingState, NewState);

/** Broadcast when a fish is successfully caught. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFishCaught, const FFishData&, Fish, float, Weight);

/**
 * UFishingComponent
 *
 * Add this component to any character or pawn to give it fishing abilities.
 * It manages the full fishing loop: cast → wait → bite → reel → catch.
 *
 * Interaction:
 *  - Call StartFishing() when the player presses the fishing key while inside
 *    an AFishingZone. The component tracks overlapping zones automatically via
 *    SetCurrentZone / ClearCurrentZone (called by AFishingZone).
 *  - Call ReelIn() each time the player presses the reel input while in the
 *    Biting state to progress to Reeling / Caught.
 *  - Call CancelFishing() to abort at any time.
 */
UCLASS(ClassGroup = (Fishing), meta = (BlueprintSpawnableComponent))
class STILLWATER2_API UFishingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFishingComponent();

	// -- API ----------------------------------------------------------------

	/** Begin the fishing sequence (requires a valid current zone). */
	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void StartFishing();

	/** Attempt to reel in the fish (valid during the Biting state). */
	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void ReelIn();

	/** Cancel fishing and return to Idle. */
	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void CancelFishing();

	/** Called by AFishingZone when the player enters/exits. */
	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void SetCurrentZone(class AFishingZone* Zone);

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void ClearCurrentZone(class AFishingZone* Zone);

	// -- State queries -------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Fishing")
	EFishingState GetFishingState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Fishing")
	bool IsFishing() const { return CurrentState != EFishingState::Idle; }

	UFUNCTION(BlueprintPure, Category = "Fishing")
	bool IsInsideFishingZone() const { return CurrentZone != nullptr; }

	// -- Delegates -----------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Fishing")
	FOnFishingStateChanged OnFishingStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Fishing")
	FOnFishCaught OnFishCaught;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// -- Tuning (exposed to designers) --------------------------------------

	/** Seconds it takes to cast the line. */
	UPROPERTY(EditAnywhere, Category = "Fishing|Tuning", meta = (ClampMin = "0.1"))
	float CastDuration = 1.0f;

	/** Minimum seconds to wait before a fish bites. */
	UPROPERTY(EditAnywhere, Category = "Fishing|Tuning", meta = (ClampMin = "0.0"))
	float MinWaitTime = 2.0f;

	/** Maximum seconds to wait before a fish bites. */
	UPROPERTY(EditAnywhere, Category = "Fishing|Tuning", meta = (ClampMin = "0.0"))
	float MaxWaitTime = 8.0f;

	/** Seconds the player has to press Reel before the fish escapes. */
	UPROPERTY(EditAnywhere, Category = "Fishing|Tuning", meta = (ClampMin = "0.1"))
	float BiteWindow = 2.0f;

	/** Seconds the reel-in animation plays. */
	UPROPERTY(EditAnywhere, Category = "Fishing|Tuning", meta = (ClampMin = "0.1"))
	float ReelDuration = 1.5f;

	/** Seconds the "caught" celebration state lasts before returning to Idle. */
	UPROPERTY(EditAnywhere, Category = "Fishing|Tuning", meta = (ClampMin = "0.0"))
	float CaughtDisplayTime = 2.0f;

	// -- Internal state ------------------------------------------------------

	UPROPERTY()
	EFishingState CurrentState = EFishingState::Idle;

	UPROPERTY()
	TObjectPtr<class AFishingZone> CurrentZone = nullptr;

	/** Timer that drives state transitions. */
	float StateTimer = 0.0f;

	/** The wait duration chosen for the current cast. */
	float ChosenWaitTime = 0.0f;

	/** Fish selected for the current bite. */
	FFishData PendingFish;

	/** Rolled weight for the pending fish. */
	float PendingWeight = 0.0f;

	// -- Helpers -------------------------------------------------------------

	void SetState(EFishingState NewState);
	void SelectRandomFish();
};
