// FishingComponent.h — Attach to your player pawn to enable fishing

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FishDataAsset.h"
#include "FishingComponent.generated.h"

class AFishingZone;
class UInputAction;
struct FInputActionValue;

UENUM(BlueprintType)
enum class EFishingState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Casting		UMETA(DisplayName = "Casting"),
	Waiting		UMETA(DisplayName = "Waiting for Bite"),
	FishHooked	UMETA(DisplayName = "Fish Hooked!"),
	Reeling		UMETA(DisplayName = "Reeling In"),
};

/** Broadcast when player catches a fish */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFishCaught, UFishDataAsset*, FishData, float, Weight, EFishRarity, Rarity);

/** Broadcast whenever fishing state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFishingStateChanged, EFishingState, OldState, EFishingState, NewState);

/** Broadcast when a fish escapes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFishEscaped);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STILLWATER2_API UFishingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFishingComponent();

	// ─── Configuration ──────────────────────────────────────────────

	/** How long the player has to press reel after a fish hooks (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing|Timing", meta = (ClampMin = "0.2"))
	float HookWindowDuration = 1.5f;

	/** How long the reel mini-game lasts before catch is confirmed (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing|Timing", meta = (ClampMin = "0.5"))
	float ReelDuration = 2.0f;

	/** How long the cast animation takes before the line is in the water */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing|Timing", meta = (ClampMin = "0.1"))
	float CastDuration = 0.5f;

	// ─── State ──────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Fishing")
	EFishingState CurrentState = EFishingState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Fishing")
	TObjectPtr<AFishingZone> ActiveZone;

	UPROPERTY(BlueprintReadOnly, Category = "Fishing")
	TObjectPtr<UFishDataAsset> HookedFish;

	// ─── Events ─────────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "Fishing|Events")
	FOnFishCaught OnFishCaught;

	UPROPERTY(BlueprintAssignable, Category = "Fishing|Events")
	FOnFishingStateChanged OnFishingStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Fishing|Events")
	FOnFishEscaped OnFishEscaped;

	// ─── Input entry points (call from your player pawn / controller) ─

	/** Call this when the player presses the "fish" button.
	 *  If idle + overlapping a FishingZone → starts casting.
	 *  If a fish is hooked → starts reeling. */
	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void OnFishActionPressed();

	/** Call this to cancel fishing at any time and return to Idle. */
	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void CancelFishing();

	/** Returns true if the player is currently in any fishing state (not Idle). */
	UFUNCTION(BlueprintPure, Category = "Fishing")
	bool IsFishing() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void SetState(EFishingState NewState);

	/** Find overlapping FishingZone, if any */
	AFishingZone* FindOverlappingZone() const;

	void BeginCast();
	void BeginWaiting();
	void OnBiteTimerFired();
	void BeginHookWindow();
	void BeginReel();
	void CompleteCatch();
	void FishEscaped();
	void ResetToIdle();

	// Timers
	float StateTimer = 0.f;

	FTimerHandle BiteTimerHandle;
};
