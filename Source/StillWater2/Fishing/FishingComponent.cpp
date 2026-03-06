// Copyright StillWater2. All Rights Reserved.

#include "FishingComponent.h"
#include "FishingZone.h"

UFishingComponent::UFishingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // enabled only while fishing
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void UFishingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UFishingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	StateTimer += DeltaTime;

	switch (CurrentState)
	{
	case EFishingState::Casting:
		if (StateTimer >= CastDuration)
		{
			SetState(EFishingState::Waiting);
		}
		break;

	case EFishingState::Waiting:
		if (StateTimer >= ChosenWaitTime)
		{
			SelectRandomFish();
			SetState(EFishingState::Biting);
		}
		break;

	case EFishingState::Biting:
		if (StateTimer >= BiteWindow)
		{
			// Player was too slow – fish escapes, return to idle.
			UE_LOG(LogTemp, Warning, TEXT("FishingComponent: Fish escaped!"));
			SetState(EFishingState::Idle);
		}
		break;

	case EFishingState::Reeling:
		if (StateTimer >= ReelDuration)
		{
			SetState(EFishingState::Caught);
		}
		break;

	case EFishingState::Caught:
		if (StateTimer >= CaughtDisplayTime)
		{
			SetState(EFishingState::Idle);
		}
		break;

	default:
		break;
	}
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void UFishingComponent::StartFishing()
{
	if (CurrentState != EFishingState::Idle)
	{
		UE_LOG(LogTemp, Warning, TEXT("FishingComponent: Cannot start – already fishing."));
		return;
	}

	if (!CurrentZone)
	{
		UE_LOG(LogTemp, Warning, TEXT("FishingComponent: Cannot start – not inside a fishing zone."));
		return;
	}

	ChosenWaitTime = FMath::RandRange(MinWaitTime, MaxWaitTime);
	SetState(EFishingState::Casting);
}

void UFishingComponent::ReelIn()
{
	if (CurrentState == EFishingState::Biting)
	{
		SetState(EFishingState::Reeling);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FishingComponent: ReelIn ignored – not in Biting state."));
	}
}

void UFishingComponent::CancelFishing()
{
	if (CurrentState != EFishingState::Idle)
	{
		UE_LOG(LogTemp, Log, TEXT("FishingComponent: Fishing cancelled."));
		SetState(EFishingState::Idle);
	}
}

void UFishingComponent::SetCurrentZone(AFishingZone* Zone)
{
	CurrentZone = Zone;
}

void UFishingComponent::ClearCurrentZone(AFishingZone* Zone)
{
	if (CurrentZone == Zone)
	{
		CurrentZone = nullptr;

		// If we were actively fishing, cancel.
		if (CurrentState != EFishingState::Idle)
		{
			CancelFishing();
		}
	}
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

void UFishingComponent::SetState(EFishingState NewState)
{
	const EFishingState OldState = CurrentState;
	CurrentState = NewState;
	StateTimer = 0.0f;

	// Enable/disable tick depending on whether we are fishing.
	SetComponentTickEnabled(NewState != EFishingState::Idle);

	UE_LOG(LogTemp, Log, TEXT("FishingComponent: %s -> %s"),
		*UEnum::GetValueAsString(OldState),
		*UEnum::GetValueAsString(NewState));

	OnFishingStateChanged.Broadcast(OldState, NewState);

	// Fire the caught delegate when entering Caught state.
	if (NewState == EFishingState::Caught)
	{
		UE_LOG(LogTemp, Log, TEXT("FishingComponent: Caught '%s' weighing %.2f kg!"),
			*PendingFish.FishName.ToString(), PendingWeight);
		OnFishCaught.Broadcast(PendingFish, PendingWeight);
	}
}

void UFishingComponent::SelectRandomFish()
{
	if (!CurrentZone)
	{
		PendingFish = FFishData();
		PendingFish.FishName = FName(TEXT("Unknown Fish"));
		PendingWeight = 1.0f;
		return;
	}

	const TArray<FFishData>& FishList = CurrentZone->GetAvailableFish();
	if (FishList.Num() == 0)
	{
		PendingFish = FFishData();
		PendingFish.FishName = FName(TEXT("Unknown Fish"));
		PendingWeight = 1.0f;
		return;
	}

	// Build a weighted random selection based on rarity.
	// Common = 60, Uncommon = 25, Rare = 10, Legendary = 5
	TArray<float> Weights;
	Weights.SetNum(FishList.Num());
	for (int32 i = 0; i < FishList.Num(); ++i)
	{
		switch (FishList[i].Rarity)
		{
		case EFishRarity::Common:    Weights[i] = 60.0f; break;
		case EFishRarity::Uncommon:  Weights[i] = 25.0f; break;
		case EFishRarity::Rare:      Weights[i] = 10.0f; break;
		case EFishRarity::Legendary: Weights[i] = 5.0f;  break;
		}
	}

	float TotalWeight = 0.0f;
	for (float W : Weights) { TotalWeight += W; }

	float Roll = FMath::FRandRange(0.0f, TotalWeight);
	int32 ChosenIndex = 0;
	for (int32 i = 0; i < Weights.Num(); ++i)
	{
		Roll -= Weights[i];
		if (Roll <= 0.0f)
		{
			ChosenIndex = i;
			break;
		}
	}

	PendingFish = FishList[ChosenIndex];
	PendingWeight = FMath::RandRange(PendingFish.MinWeight, PendingFish.MaxWeight);
}
