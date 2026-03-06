// Copyright StillWater2. All Rights Reserved.

#include "FishingZone.h"
#include "FishingComponent.h"
#include "Components/BoxComponent.h"

AFishingZone::AFishingZone()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));
	TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
	TriggerVolume->SetGenerateOverlapEvents(true);
	RootComponent = TriggerVolume;
}

void AFishingZone::BeginPlay()
{
	Super::BeginPlay();

	BuildFishList();

	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AFishingZone::OnOverlapBegin);
	TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AFishingZone::OnOverlapEnd);
}

const TArray<FFishData>& AFishingZone::GetAvailableFish() const
{
	return CachedFishList;
}

// ---------------------------------------------------------------------------
// Overlap callbacks
// ---------------------------------------------------------------------------

void AFishingZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) { return; }

	if (UFishingComponent* FishingComp = OtherActor->FindComponentByClass<UFishingComponent>())
	{
		FishingComp->SetCurrentZone(this);
		UE_LOG(LogTemp, Log, TEXT("FishingZone: %s entered zone %s"), *OtherActor->GetName(), *GetName());
	}
}

void AFishingZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) { return; }

	if (UFishingComponent* FishingComp = OtherActor->FindComponentByClass<UFishingComponent>())
	{
		FishingComp->ClearCurrentZone(this);
		UE_LOG(LogTemp, Log, TEXT("FishingZone: %s left zone %s"), *OtherActor->GetName(), *GetName());
	}
}

// ---------------------------------------------------------------------------
// Internal
// ---------------------------------------------------------------------------

void AFishingZone::BuildFishList()
{
	CachedFishList.Reset();

	if (FishDataAsset)
	{
		CachedFishList.Append(FishDataAsset->Fish);
	}

	CachedFishList.Append(InlineFish);

	// If nothing was configured, add a default fish so the system still works.
	if (CachedFishList.Num() == 0)
	{
		FFishData Default;
		Default.FishName = FName(TEXT("Small Bass"));
		Default.Rarity = EFishRarity::Common;
		Default.MinWeight = 0.3f;
		Default.MaxWeight = 2.0f;
		Default.BaseScore = 5;
		CachedFishList.Add(Default);

		UE_LOG(LogTemp, Warning, TEXT("FishingZone %s: No fish configured – using default Small Bass."), *GetName());
	}
}
