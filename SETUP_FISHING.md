# Fishing Mechanic — Setup Guide

This document explains how to integrate the **Fishing Mechanic** into your StillWater2 Unreal Engine project.

---

## Overview

The fishing system is built from three C++ pieces that live in `Source/StillWater2/Fishing/`:

| File | Purpose |
|---|---|
| `FishData.h` | `FFishData` struct (name, rarity, weight, score, icon) and `UFishDataAsset` for defining fish species in the editor. |
| `FishingComponent.h/.cpp` | `UFishingComponent` — an Actor Component that drives the full fishing state machine. |
| `FishingZone.h/.cpp` | `AFishingZone` — a trigger-volume actor you place in the world to mark fishable areas. |

### Fishing State Machine

```
Idle ──▶ Casting ──▶ Waiting ──▶ Biting ──▶ Reeling ──▶ Caught ──▶ Idle
  ▲                                │ (timeout)                        │
  └────────────────────────────────┘                                  │
  ▲                                                                   │
  └───────────────────────────────────────────────────────────────────┘
```

- **Idle** — Not fishing.
- **Casting** — Line is being thrown (plays for `CastDuration` seconds).
- **Waiting** — Waiting for a bite (random duration between `MinWaitTime` and `MaxWaitTime`).
- **Biting** — A fish is on the line! Player must call `ReelIn()` within `BiteWindow` seconds or the fish escapes.
- **Reeling** — Pulling the fish in (`ReelDuration` seconds).
- **Caught** — Fish caught! `OnFishCaught` delegate fires. Returns to Idle after `CaughtDisplayTime` seconds.

The player can call `CancelFishing()` at any point to abort and return to Idle.

---

## Step-by-Step Setup

### 1. Compile the Project

Open the project in Unreal Engine 5.7 and compile (or press **Ctrl+Shift+B** in your IDE). The new classes will be picked up automatically because they are inside the existing `StillWater2` module.

### 2. Add the Fishing Component to Your Player

1. Open your player Blueprint (e.g. `BP_Player`).
2. Click **Add Component** and search for **FishingComponent**.
3. Select it and adjust the tuning properties in the Details panel:
   - `CastDuration` — How long casting takes (default: 1 s).
   - `MinWaitTime` / `MaxWaitTime` — Range for the random wait (defaults: 2–8 s).
   - `BiteWindow` — Seconds the player has to react when a fish bites (default: 2 s).
   - `ReelDuration` — Reel-in animation time (default: 1.5 s).
   - `CaughtDisplayTime` — How long the "caught" state is shown (default: 2 s).

### 3. Create Fish Data

#### Option A — Use a Data Asset (recommended for sharing across zones)

1. In the Content Browser, right-click → **Miscellaneous → Data Asset**.
2. Select **FishDataAsset** as the class.
3. Name it (e.g. `DA_LakeFish`).
4. Open it and add entries to the **Fish** array. For each entry set:
   - **Fish Name** — e.g. `Trout`, `Catfish`, `Golden Koi`.
   - **Rarity** — Common / Uncommon / Rare / Legendary.
   - **Min Weight** / **Max Weight** — Weight range in kg.
   - **Base Score** — Points awarded.
   - **Icon** (optional) — A `UTexture2D` reference for UI.

#### Option B — Inline fish per zone

Skip the data asset and fill the **Inline Fish** array directly on each `AFishingZone` actor (see step 4).

### 4. Place Fishing Zones in the World

1. In the **Place Actors** panel, search for **FishingZone** (or drag it from the Content Browser).
2. Position and scale the trigger box to cover the body of water.
3. In the Details panel:
   - Assign your **FishDataAsset** (from Step 3A), **or**
   - Add entries to the **Inline Fish** array.
   - You can use both — the lists are merged at runtime.
4. If you leave both empty, a default "Small Bass" fish is used automatically so the system still works for testing.

### 5. Bind Player Input

The component exposes three key functions to call from your input logic:

| Function | When to Call |
|---|---|
| `StartFishing()` | Player presses the "Fish" key while inside a fishing zone. |
| `ReelIn()` | Player presses the "Reel" key while a fish is biting. |
| `CancelFishing()` | Player presses a cancel key, or you want to abort programmatically. |

#### Example: Enhanced Input (Blueprint)

1. Create an **Input Action** (`IA_Fish`) of type `bool` (Digital).
2. In your **Input Mapping Context**, bind `IA_Fish` to a key (e.g. **F** or **E**).
3. In your player Blueprint's Event Graph:
   - On `IA_Fish` **Started**:
     - If `FishingComponent → IsFishing()` is **false** → call `StartFishing()`.
     - Else if `FishingComponent → GetFishingState()` == **Biting** → call `ReelIn()`.
   - On `IA_Fish` **Canceled** / a separate cancel key → call `CancelFishing()`.

#### Example: Enhanced Input (C++)

```cpp
// In your Character's SetupPlayerInputComponent:
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(FishAction, ETriggerEvent::Started, this, &AMyCharacter::OnFishPressed);
    }
}

void AMyCharacter::OnFishPressed()
{
    UFishingComponent* FC = FindComponentByClass<UFishingComponent>();
    if (!FC) return;

    if (!FC->IsFishing())
    {
        FC->StartFishing();
    }
    else if (FC->GetFishingState() == EFishingState::Biting)
    {
        FC->ReelIn();
    }
}
```

### 6. React to Events (Optional)

Bind to the component's delegates to trigger animations, sounds, or UI:

| Delegate | Payload | Use For |
|---|---|---|
| `OnFishingStateChanged` | `OldState`, `NewState` | Play casting / waiting / reeling animations, show UI prompts. |
| `OnFishCaught` | `Fish` (FFishData), `Weight` (float) | Display caught-fish popup, add to inventory, award score. |

**Blueprint example:**
1. Select the FishingComponent in your player Blueprint.
2. In the Details panel, scroll to **Events** and click the **+** next to `OnFishCaught`.
3. Use the event node to display the fish name and weight, play a sound, etc.

---

## File Reference

```
Source/StillWater2/Fishing/
├── FishData.h              # FFishData struct + UFishDataAsset
├── FishingComponent.h      # UFishingComponent header
├── FishingComponent.cpp    # UFishingComponent implementation
├── FishingZone.h           # AFishingZone header
└── FishingZone.cpp         # AFishingZone implementation
```

---

## Tips

- **Testing quickly:** Place a FishingZone with no data asset — it will spawn a default "Small Bass" so you can test the full loop immediately.
- **Tuning the rarity curve:** In `FishingComponent.cpp` → `SelectRandomFish()`, the weights are Common=60, Uncommon=25, Rare=10, Legendary=5. Adjust these values to change drop rates.
- **Multiple zones:** Each zone can have a different FishDataAsset, so a lake, river, and ocean can offer different fish.
- **Debugging:** All state transitions are logged to `LogTemp`. Open the Output Log in the editor to follow the flow.
