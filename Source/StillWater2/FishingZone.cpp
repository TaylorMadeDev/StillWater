// FishingZone.cpp

#include "FishingZone.h"

AFishingZone::AFishingZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneTrigger"));
	ZoneTrigger->SetBoxExtent(FVector(200.f, 200.f, 50.f));
	ZoneTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	ZoneTrigger->SetGenerateOverlapEvents(true);
	SetRootComponent(ZoneTrigger);
}

UFishDataAsset* AFishingZone::PickRandomFish() const
{
	if (AvailableFish.IsEmpty())
	{
		return nullptr;
	}

	float TotalWeight = 0.f;
	for (const auto& Fish : AvailableFish)
	{
		if (Fish)
		{
			TotalWeight += Fish->CatchWeight;
		}
	}

	if (TotalWeight <= 0.f)
	{
		return nullptr;
	}

	float Roll = FMath::FRandRange(0.f, TotalWeight);
	float Accumulated = 0.f;

	for (const auto& Fish : AvailableFish)
	{
		if (Fish)
		{
			Accumulated += Fish->CatchWeight;
			if (Roll <= Accumulated)
			{
				return Fish;
			}
		}
	}

	return AvailableFish.Last();
}

float AFishingZone::GetRandomBiteDelay() const
{
	return FMath::FRandRange(MinBiteTime, MaxBiteTime);
}
