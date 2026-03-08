// FishingComponent.cpp

#include "FishingComponent.h"
#include "FishingZone.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "TimerManager.h"

UFishingComponent::UFishingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // only tick when actively fishing
}

void UFishingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UFishingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentState == EFishingState::Casting)
	{
		StateTimer -= DeltaTime;
		if (StateTimer <= 0.f)
		{
			BeginWaiting();
		}
	}
	else if (CurrentState == EFishingState::FishHooked)
	{
		StateTimer -= DeltaTime;
		if (StateTimer <= 0.f)
		{
			FishEscaped();
		}
	}
	else if (CurrentState == EFishingState::Reeling)
	{
		StateTimer -= DeltaTime;
		if (StateTimer <= 0.f)
		{
			CompleteCatch();
		}
	}
}

// ─── Public API ─────────────────────────────────────────────────────────────

void UFishingComponent::OnFishActionPressed()
{
	switch (CurrentState)
	{
	case EFishingState::Idle:
		BeginCast();
		break;

	case EFishingState::FishHooked:
		BeginReel();
		break;

	default:
		// Pressing during cast / waiting / reeling does nothing
		break;
	}
}

void UFishingComponent::CancelFishing()
{
	if (CurrentState != EFishingState::Idle)
	{
		GetWorld()->GetTimerManager().ClearTimer(BiteTimerHandle);
		ResetToIdle();
	}
}

bool UFishingComponent::IsFishing() const
{
	return CurrentState != EFishingState::Idle;
}

// ─── State machine helpers ──────────────────────────────────────────────────

void UFishingComponent::SetState(EFishingState NewState)
{
	EFishingState OldState = CurrentState;
	CurrentState = NewState;
	OnFishingStateChanged.Broadcast(OldState, NewState);

	// Enable tick only while actively fishing
	SetComponentTickEnabled(NewState != EFishingState::Idle);
}

AFishingZone* UFishingComponent::FindOverlappingZone() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	TArray<AActor*> Overlapping;
	Owner->GetOverlappingActors(Overlapping, AFishingZone::StaticClass());

	return Overlapping.IsEmpty() ? nullptr : Cast<AFishingZone>(Overlapping[0]);
}

void UFishingComponent::BeginCast()
{
	ActiveZone = FindOverlappingZone();
	if (!ActiveZone)
	{
		return; // not near water
	}

	StateTimer = CastDuration;
	SetState(EFishingState::Casting);
}

void UFishingComponent::BeginWaiting()
{
	SetState(EFishingState::Waiting);

	if (!ActiveZone)
	{
		ResetToIdle();
		return;
	}

	float Delay = ActiveZone->GetRandomBiteDelay();
	GetWorld()->GetTimerManager().SetTimer(
		BiteTimerHandle,
		this,
		&UFishingComponent::OnBiteTimerFired,
		Delay,
		false
	);
}

void UFishingComponent::OnBiteTimerFired()
{
	if (CurrentState != EFishingState::Waiting)
	{
		return;
	}

	BeginHookWindow();
}

void UFishingComponent::BeginHookWindow()
{
	if (!ActiveZone)
	{
		ResetToIdle();
		return;
	}

	HookedFish = ActiveZone->PickRandomFish();
	if (!HookedFish)
	{
		ResetToIdle();
		return;
	}

	StateTimer = HookWindowDuration;
	SetState(EFishingState::FishHooked);
}

void UFishingComponent::BeginReel()
{
	StateTimer = ReelDuration;
	SetState(EFishingState::Reeling);
}

void UFishingComponent::CompleteCatch()
{
	if (HookedFish)
	{
		float Weight = FMath::FRandRange(HookedFish->MinWeight, HookedFish->MaxWeight);
		OnFishCaught.Broadcast(HookedFish, Weight, HookedFish->Rarity);
	}

	ResetToIdle();
}

void UFishingComponent::FishEscaped()
{
	OnFishEscaped.Broadcast();
	ResetToIdle();
}

void UFishingComponent::ResetToIdle()
{
	HookedFish = nullptr;
	ActiveZone = nullptr;
	StateTimer = 0.f;
	SetState(EFishingState::Idle);
}
