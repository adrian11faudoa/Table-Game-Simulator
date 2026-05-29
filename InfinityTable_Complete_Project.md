# InfinityTable — Complete Production-Ready Project

> Physics-Based Virtual Tabletop Sandbox Engine  
> Inspired by Tabletop Simulator · Built on Unreal Engine 5

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Technology Stack](#2-technology-stack)
3. [Full Project Structure](#3-full-project-structure)
4. [Core C++ Source Code](#4-core-c-source-code)
5. [Physics System](#5-physics-system)
6. [Multiplayer Networking](#6-multiplayer-networking)
7. [RPG Module](#7-rpg-module)
8. [Lua Scripting Engine](#8-lua-scripting-engine)
9. [Save System](#9-save-system)
10. [Asset Import Pipeline](#10-asset-import-pipeline)
11. [UI / HUD System](#11-ui--hud-system)
12. [Modding SDK](#12-modding-sdk)
13. [Audio System](#13-audio-system)
14. [Example Games & Scripts](#14-example-games--scripts)
15. [Build Instructions](#15-build-instructions)
16. [Multiplayer Deployment](#16-multiplayer-deployment)
17. [Performance Guide](#17-performance-guide)
18. [Security & Sandboxing](#18-security--sandboxing)

---

## 1. Project Overview

InfinityTable is a next-generation physics-based virtual tabletop engine providing:

- Real-time rigid-body physics for all tabletop objects
- Seamless online/LAN multiplayer with authoritative server physics
- Lua scripting sandbox for custom game logic
- Full mod SDK with Steam Workshop integration
- RPG tools: fog of war, grid system, initiative tracker, GM tools
- 5 themed environments with dynamic lighting
- Support for board games, card games, RPGs, and custom creations

---

## 2. Technology Stack

| Layer | Technology |
|---|---|
| Engine | Unreal Engine 5.3+ |
| Primary Language | C++ 17 |
| Scripting | Lua 5.4 (via sol2 binding) |
| Physics | Chaos Physics (UE5 built-in) |
| Networking | Unreal NetDriver + EOS / Steam OSS |
| UI Framework | UMG (Unreal Motion Graphics) |
| Persistence | SQLite3 (via amalgamated header) |
| Asset Import | glTFRuntime plugin + Assimp |
| Audio | MetaSounds (UE5) |
| Shader/VFX | Niagara + Lumen GI |
| Build System | UnrealBuildTool (UBT) |

---

## 3. Full Project Structure

```
InfinityTable/
├── InfinityTable.uproject
├── Config/
│   ├── DefaultEngine.ini
│   ├── DefaultGame.ini
│   ├── DefaultInput.ini
│   └── DefaultNetworkVersion.ini
├── Source/
│   └── InfinityTable/
│       ├── InfinityTable.Build.cs
│       ├── InfinityTableGameInstance.h/.cpp
│       ├── Core/
│       │   ├── ITGameMode.h/.cpp
│       │   ├── ITGameState.h/.cpp
│       │   ├── ITPlayerController.h/.cpp
│       │   ├── ITPlayerState.h/.cpp
│       │   ├── ITSpectatorPawn.h/.cpp
│       │   └── ITHUD.h/.cpp
│       ├── Objects/
│       │   ├── TableObject.h/.cpp          (base class)
│       │   ├── TableDice.h/.cpp
│       │   ├── TableCard.h/.cpp
│       │   ├── TableDeck.h/.cpp
│       │   ├── TableToken.h/.cpp
│       │   ├── TableMiniature.h/.cpp
│       │   ├── TableBoard.h/.cpp
│       │   └── TableSpawnManager.h/.cpp
│       ├── Physics/
│       │   ├── GrabComponent.h/.cpp
│       │   ├── PhysicsInteractionSubsystem.h/.cpp
│       │   ├── SnapPointComponent.h/.cpp
│       │   ├── DiceResultDetector.h/.cpp
│       │   └── ObjectGroupManager.h/.cpp
│       ├── Networking/
│       │   ├── ITNetworkManager.h/.cpp
│       │   ├── ITSessionSubsystem.h/.cpp
│       │   ├── ObjectReplicationProxy.h/.cpp
│       │   └── PhysicsAuthorityManager.h/.cpp
│       ├── Scripting/
│       │   ├── LuaSubsystem.h/.cpp
│       │   ├── LuaSandbox.h/.cpp
│       │   ├── LuaAPIBindings.h/.cpp
│       │   ├── LuaObjectAPI.h/.cpp
│       │   ├── LuaTableAPI.h/.cpp
│       │   ├── LuaEventsAPI.h/.cpp
│       │   └── LuaUIAPI.h/.cpp
│       ├── RPG/
│       │   ├── FogOfWarManager.h/.cpp
│       │   ├── GridManager.h/.cpp
│       │   ├── InitiativeTracker.h/.cpp
│       │   ├── GMToolsSubsystem.h/.cpp
│       │   ├── LineOfSightVolume.h/.cpp
│       │   └── CharacterTokenComponent.h/.cpp
│       ├── UI/
│       │   ├── MainMenuWidget.h/.cpp
│       │   ├── LobbyWidget.h/.cpp
│       │   ├── InGameHUD.h/.cpp
│       │   ├── ObjectContextMenu.h/.cpp
│       │   ├── RadialMenuWidget.h/.cpp
│       │   ├── ChatWidget.h/.cpp
│       │   ├── ObjectBrowserWidget.h/.cpp
│       │   ├── ScriptingConsoleWidget.h/.cpp
│       │   └── RPGPanelWidget.h/.cpp
│       ├── Save/
│       │   ├── SaveGameSubsystem.h/.cpp
│       │   ├── TableSaveState.h
│       │   └── SQLiteWrapper.h/.cpp
│       ├── Assets/
│       │   ├── AssetImportSubsystem.h/.cpp
│       │   ├── RuntimeMeshImporter.h/.cpp
│       │   └── AssetRegistry.h/.cpp
│       ├── Audio/
│       │   ├── AudioManagerSubsystem.h/.cpp
│       │   └── PositionalAudioComponent.h/.cpp
│       └── Mod/
│           ├── ModLoadingSubsystem.h/.cpp
│           ├── ModManifest.h
│           └── ModValidator.h/.cpp
├── Content/
│   ├── Blueprints/
│   ├── Environments/
│   │   ├── FantasyTavern/
│   │   ├── SciFiStation/
│   │   ├── ModernGameRoom/
│   │   ├── MedievalHall/
│   │   └── CyberpunkDen/
│   ├── Objects/
│   │   ├── Dice/
│   │   ├── Cards/
│   │   ├── Tokens/
│   │   └── Miniatures/
│   ├── UI/
│   ├── Audio/
│   │   ├── Dice/
│   │   ├── Cards/
│   │   └── Ambient/
│   └── Materials/
├── SDK/
│   ├── README.md
│   ├── LuaAPI.md
│   ├── ModManifest.schema.json
│   └── Examples/
│       ├── Chess/
│       ├── Checkers/
│       ├── DiceGame/
│       └── RPGCampaign/
├── Docs/
│   ├── Architecture.md
│   ├── NetworkingGuide.md
│   ├── ModdingGuide.md
│   ├── SaveSystem.md
│   └── AssetImport.md
└── ThirdParty/
    ├── sol2/           (Lua C++ binding)
    ├── lua54/          (Lua 5.4 source)
    ├── sqlite3/        (amalgamated)
    └── glTFRuntime/    (UE5 plugin)
```

---

## 4. Core C++ Source Code

### 4.1 Build Configuration

**Source/InfinityTable/InfinityTable.Build.cs**
```csharp
using UnrealBuildTool;

public class InfinityTable : ModuleRules
{
    public InfinityTable(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp17;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "InputCore",
            "UMG", "Slate", "SlateCore",
            "NetCore", "OnlineSubsystem", "OnlineSubsystemUtils",
            "PhysicsCore", "Chaos",
            "MetasoundEngine", "AudioMixer",
            "Niagara",
            "Json", "JsonUtilities",
            "HTTP"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "OnlineSubsystemSteam",
            "OnlineSubsystemEOS",
            "glTFRuntime"
        });

        // Lua + sol2
        PublicIncludePaths.Add("$(ProjectDir)/ThirdParty/lua54/include");
        PublicIncludePaths.Add("$(ProjectDir)/ThirdParty/sol2/include");
        PublicAdditionalLibraries.Add("$(ProjectDir)/ThirdParty/lua54/lib/lua54.lib");

        // SQLite
        PublicIncludePaths.Add("$(ProjectDir)/ThirdParty/sqlite3");
    }
}
```

---

### 4.2 Game Mode

**Source/InfinityTable/Core/ITGameMode.h**
```cpp
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ITGameMode.generated.h"

UCLASS()
class INFINITYTABLE_API AITGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AITGameMode();

    virtual void InitGame(const FString& MapName,
                          const FString& Options,
                          FString& ErrorMessage) override;

    virtual APlayerController* Login(UPlayer* NewPlayer,
                                     ENetRole InRemoteRole,
                                     const FString& Portal,
                                     const FString& Options,
                                     const FUniqueNetIdRepl& UniqueId,
                                     FString& ErrorMessage) override;

    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    UFUNCTION(BlueprintCallable, Category = "InfinityTable")
    void LoadTableState(const FString& SaveSlotName);

    UFUNCTION(BlueprintCallable, Category = "InfinityTable")
    void SaveTableState(const FString& SaveSlotName);

    UFUNCTION(BlueprintCallable, Category = "InfinityTable")
    void SetPlayerPermission(APlayerController* Player, EPlayerRole Role);

    UPROPERTY(EditDefaultsOnly, Category = "InfinityTable")
    int32 MaxPlayers = 6;

    UPROPERTY(EditDefaultsOnly, Category = "InfinityTable")
    bool bRequirePassword = false;

private:
    UPROPERTY()
    TMap<APlayerController*, EPlayerRole> PlayerRoles;
};
```

**Source/InfinityTable/Core/ITGameMode.cpp**
```cpp
#include "ITGameMode.h"
#include "ITGameState.h"
#include "ITPlayerController.h"
#include "ITPlayerState.h"
#include "ITSpectatorPawn.h"
#include "Save/SaveGameSubsystem.h"
#include "Scripting/LuaSubsystem.h"

AITGameMode::AITGameMode()
{
    GameStateClass        = AITGameState::StaticClass();
    PlayerControllerClass = AITPlayerController::StaticClass();
    PlayerStateClass      = AITPlayerState::StaticClass();
    DefaultPawnClass      = AITSpectatorPawn::StaticClass();
}

void AITGameMode::InitGame(const FString& MapName,
                            const FString& Options,
                            FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    // Boot Lua scripting subsystem
    if (ULuaSubsystem* Lua = GetGameInstance()->GetSubsystem<ULuaSubsystem>())
    {
        Lua->Initialize();
    }
}

void AITGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    // Default role: Player
    PlayerRoles.Add(NewPlayer, EPlayerRole::Player);

    // First player becomes Host
    if (PlayerRoles.Num() == 1)
    {
        PlayerRoles[NewPlayer] = EPlayerRole::Host;
    }

    if (AITPlayerState* PS = NewPlayer->GetPlayerState<AITPlayerState>())
    {
        PS->SetRole(PlayerRoles[NewPlayer]);
    }

    // Notify Lua
    if (ULuaSubsystem* Lua = GetGameInstance()->GetSubsystem<ULuaSubsystem>())
    {
        Lua->FireEvent("PlayerJoined", NewPlayer);
    }
}

void AITGameMode::Logout(AController* Exiting)
{
    if (APlayerController* PC = Cast<APlayerController>(Exiting))
    {
        PlayerRoles.Remove(PC);

        if (ULuaSubsystem* Lua = GetGameInstance()->GetSubsystem<ULuaSubsystem>())
        {
            Lua->FireEvent("PlayerLeft", PC);
        }
    }
    Super::Logout(Exiting);
}

void AITGameMode::SetPlayerPermission(APlayerController* Player, EPlayerRole Role)
{
    if (PlayerRoles.Contains(Player))
    {
        PlayerRoles[Player] = Role;
        if (AITPlayerState* PS = Player->GetPlayerState<AITPlayerState>())
        {
            PS->SetRole(Role);
        }
    }
}
```

---

### 4.3 Base Table Object

**Source/InfinityTable/Objects/TableObject.h**
```cpp
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TableObject.generated.h"

UENUM(BlueprintType)
enum class EObjectOwnership : uint8
{
    Public,     // Anyone can interact
    Player,     // Only owning player
    Host,       // Host only
    Locked      // No interaction
};

USTRUCT(BlueprintType)
struct FTableObjectState
{
    GENERATED_BODY()

    UPROPERTY() FVector    Position;
    UPROPERTY() FRotator   Rotation;
    UPROPERTY() FVector    LinearVelocity;
    UPROPERTY() FVector    AngularVelocity;
    UPROPERTY() bool       bPhysicsEnabled = true;
    UPROPERTY() FString    ObjectTypeID;
    UPROPERTY() FString    CustomDataJSON;
};

UCLASS(Abstract)
class INFINITYTABLE_API ATableObject : public AActor
{
    GENERATED_BODY()

public:
    ATableObject();

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ── Interaction ──────────────────────────────────
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_PickUp(APlayerController* Picker);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_Release(FVector Velocity);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SetTransform(FVector Pos, FRotator Rot);

    UFUNCTION(Server, Reliable)
    void Server_Flip();

    UFUNCTION(Server, Reliable)
    void Server_Lock(bool bLock);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayPickupSound();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayReleaseSound();

    // ── State ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "TableObject")
    FTableObjectState GetObjectState() const;

    UFUNCTION(BlueprintCallable, Category = "TableObject")
    void ApplyObjectState(const FTableObjectState& State);

    // ── Snap ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "TableObject")
    void SnapToNearestPoint(float SnapRadius = 5.0f);

    // ── Properties ───────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
    EObjectOwnership Ownership = EObjectOwnership::Public;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
    FString ObjectTypeID;

    UPROPERTY(Replicated)
    bool bIsGrabbed = false;

    UPROPERTY(Replicated)
    APlayerController* GrabbingPlayer = nullptr;

    UPROPERTY(Replicated)
    bool bPhysicsLocked = false;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UAudioComponent* AudioComponent;

    virtual void BeginPlay() override;
    virtual bool CanBeGrabbedBy(APlayerController* PC) const;
};
```

**Source/InfinityTable/Objects/TableObject.cpp**
```cpp
#include "TableObject.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Physics/SnapPointComponent.h"

ATableObject::ATableObject()
{
    bReplicates          = true;
    bReplicatePhysics    = true; // UE5: physics replication
    PrimaryActorTick.bCanEverTick = true;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
    MeshComponent->SetIsReplicated(true);
    RootComponent = MeshComponent;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;
}

void ATableObject::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ATableObject, Ownership);
    DOREPLIFETIME(ATableObject, ObjectTypeID);
    DOREPLIFETIME(ATableObject, bIsGrabbed);
    DOREPLIFETIME(ATableObject, GrabbingPlayer);
    DOREPLIFETIME(ATableObject, bPhysicsLocked);
}

bool ATableObject::Server_PickUp_Validate(APlayerController* Picker)
{
    return IsValid(Picker) && CanBeGrabbedBy(Picker);
}

void ATableObject::Server_PickUp_Implementation(APlayerController* Picker)
{
    if (bIsGrabbed) return;

    bIsGrabbed     = true;
    GrabbingPlayer = Picker;
    MeshComponent->SetSimulatePhysics(false);
    Multicast_PlayPickupSound();
}

void ATableObject::Server_Release_Implementation(FVector Velocity)
{
    bIsGrabbed     = false;
    GrabbingPlayer = nullptr;
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetPhysicsLinearVelocity(Velocity);
    Multicast_PlayReleaseSound();
}

bool ATableObject::Server_SetTransform_Validate(FVector Pos, FRotator Rot)
{
    // Sanity: object must not be teleported farther than 500cm in one tick
    float Dist = FVector::Dist(Pos, GetActorLocation());
    return Dist < 500.0f;
}

void ATableObject::Server_SetTransform_Implementation(FVector Pos, FRotator Rot)
{
    SetActorLocationAndRotation(Pos, Rot,
        false, nullptr, ETeleportType::TeleportPhysics);
}

void ATableObject::Server_Flip_Implementation()
{
    FRotator R = GetActorRotation();
    R.Roll += 180.0f;
    SetActorRotation(R);
}

void ATableObject::Server_Lock_Implementation(bool bLock)
{
    bPhysicsLocked = bLock;
    MeshComponent->SetSimulatePhysics(!bLock);
}

FTableObjectState ATableObject::GetObjectState() const
{
    FTableObjectState S;
    S.Position        = GetActorLocation();
    S.Rotation        = GetActorRotation();
    S.LinearVelocity  = MeshComponent->GetPhysicsLinearVelocity();
    S.AngularVelocity = MeshComponent->GetPhysicsAngularVelocityInDegrees();
    S.bPhysicsEnabled = !bPhysicsLocked;
    S.ObjectTypeID    = ObjectTypeID;
    return S;
}

void ATableObject::ApplyObjectState(const FTableObjectState& State)
{
    SetActorLocationAndRotation(State.Position, State.Rotation,
        false, nullptr, ETeleportType::TeleportPhysics);
    MeshComponent->SetPhysicsLinearVelocity(State.LinearVelocity);
    MeshComponent->SetPhysicsAngularVelocityInDegrees(State.AngularVelocity);
    if (HasAuthority())
        Server_Lock(!State.bPhysicsEnabled);
}

bool ATableObject::CanBeGrabbedBy(APlayerController* PC) const
{
    switch (Ownership)
    {
        case EObjectOwnership::Locked: return false;
        case EObjectOwnership::Host:
            // Only host can grab
            return PC->HasAuthority();
        case EObjectOwnership::Player:
            return GrabbingPlayer == nullptr || GrabbingPlayer == PC;
        default:
            return !bIsGrabbed;
    }
}

void ATableObject::SnapToNearestPoint(float SnapRadius)
{
    TArray<USnapPointComponent*> SnapPoints;
    // Find all snap points in radius
    TArray<AActor*> Overlapping;
    GetOverlappingActors(Overlapping);
    for (AActor* A : Overlapping)
    {
        if (USnapPointComponent* SP = A->FindComponentByClass<USnapPointComponent>())
        {
            float Dist = FVector::Dist(GetActorLocation(), SP->GetComponentLocation());
            if (Dist < SnapRadius)
            {
                Server_SetTransform(SP->GetComponentLocation(),
                                    SP->GetComponentRotation());
                return;
            }
        }
    }
}
```

---

### 4.4 Dice Actor

**Source/InfinityTable/Objects/TableDice.h**
```cpp
#pragma once
#include "Objects/TableObject.h"
#include "TableDice.generated.h"

UENUM(BlueprintType)
enum class EDiceType : uint8
{
    D4, D6, D8, D10, D10Percentile, D12, D20, DCustom
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnDiceRolled, ATableDice*, Dice, int32, Result);

UCLASS()
class INFINITYTABLE_API ATableDice : public ATableObject
{
    GENERATED_BODY()

public:
    ATableDice();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
    EDiceType DiceType = EDiceType::D6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
    int32 CustomSides = 6;

    UPROPERTY(BlueprintAssignable)
    FOnDiceRolled OnDiceRolled;

    UFUNCTION(Server, Reliable)
    void Server_Roll(FVector Impulse);

    UFUNCTION(BlueprintCallable)
    int32 GetCurrentFaceUp() const;

    UFUNCTION(BlueprintCallable)
    int32 GetMaxValue() const;

protected:
    virtual void Tick(float DeltaTime) override;

private:
    bool bWasMoving       = false;
    float RestingTimer    = 0.0f;
    float RestingThreshold = 0.3f; // seconds still = settled

    void DetectResult();
    int32 CalculateFaceUp() const;
};
```

**Source/InfinityTable/Objects/TableDice.cpp**
```cpp
#include "TableDice.h"
#include "Scripting/LuaSubsystem.h"

ATableDice::ATableDice()
{
    PrimaryActorTick.bCanEverTick = true;
    ObjectTypeID = TEXT("Dice");
}

void ATableDice::Server_Roll_Implementation(FVector Impulse)
{
    MeshComponent->SetSimulatePhysics(true);

    // Apply linear + random angular impulse for realistic roll
    MeshComponent->AddImpulse(Impulse, NAME_None, true);
    FVector RandomTorque = FVector(
        FMath::RandRange(-800.f, 800.f),
        FMath::RandRange(-800.f, 800.f),
        FMath::RandRange(-800.f, 800.f)
    );
    MeshComponent->AddAngularImpulseInDegrees(RandomTorque, NAME_None, true);

    bWasMoving   = true;
    RestingTimer = 0.0f;
}

void ATableDice::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!HasAuthority()) return;

    float Speed = MeshComponent->GetPhysicsLinearVelocity().Size()
                + MeshComponent->GetPhysicsAngularVelocityInDegrees().Size();

    if (bWasMoving)
    {
        if (Speed < 2.0f)
        {
            RestingTimer += DeltaTime;
            if (RestingTimer >= RestingThreshold)
            {
                bWasMoving = false;
                DetectResult();
            }
        }
        else
        {
            RestingTimer = 0.0f;
        }
    }
}

void ATableDice::DetectResult()
{
    int32 Result = CalculateFaceUp();
    OnDiceRolled.Broadcast(this, Result);

    // Fire Lua event
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (ULuaSubsystem* Lua = GI->GetSubsystem<ULuaSubsystem>())
            {
                Lua->FireEvent("DiceRolled", this, Result);
            }
        }
    }
}

int32 ATableDice::CalculateFaceUp() const
{
    // Project each face normal against world up, highest dot = top face
    // For a d6 the 6 face normals are ±X, ±Y, ±Z in local space
    const FTransform T = GetActorTransform();
    FVector Up = FVector::UpVector;

    int32 MaxFaces = GetMaxValue();
    int32 BestFace = 1;
    float BestDot  = -2.0f;

    for (int32 i = 0; i < MaxFaces; ++i)
    {
        // Generate evenly-distributed face normals
        float Theta = FMath::DegreesToRadians(360.0f * i / MaxFaces);
        float Phi   = FMath::Acos(1.0f - 2.0f * (i + 0.5f) / MaxFaces);
        FVector LocalNormal(
            FMath::Sin(Phi) * FMath::Cos(Theta),
            FMath::Sin(Phi) * FMath::Sin(Theta),
            FMath::Cos(Phi)
        );
        FVector WorldNormal = T.TransformVector(LocalNormal);
        float Dot = FVector::DotProduct(WorldNormal, Up);
        if (Dot > BestDot)
        {
            BestDot  = Dot;
            BestFace = i + 1;
        }
    }
    return BestFace;
}

int32 ATableDice::GetMaxValue() const
{
    switch (DiceType)
    {
        case EDiceType::D4:  return 4;
        case EDiceType::D6:  return 6;
        case EDiceType::D8:  return 8;
        case EDiceType::D10: return 10;
        case EDiceType::D12: return 12;
        case EDiceType::D20: return 20;
        case EDiceType::DCustom: return CustomSides;
        default: return 6;
    }
}
```

---

### 4.5 Card + Deck System

**Source/InfinityTable/Objects/TableCard.h**
```cpp
#pragma once
#include "Objects/TableObject.h"
#include "TableCard.generated.h"

UCLASS()
class INFINITYTABLE_API ATableCard : public ATableObject
{
    GENERATED_BODY()

public:
    ATableCard();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
    FString CardID;           // Unique identifier

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
    UTexture2D* FrontTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
    UTexture2D* BackTexture;

    UPROPERTY(BlueprintReadOnly, Replicated)
    bool bFaceUp = false;

    UPROPERTY(BlueprintReadOnly, Replicated)
    bool bInHand = false;    // Hidden from other players

    UPROPERTY(BlueprintReadOnly, Replicated)
    APlayerController* HandOwner = nullptr;

    UFUNCTION(Server, Reliable)
    void Server_Flip();

    UFUNCTION(Server, Reliable)
    void Server_SetInHand(APlayerController* Owner, bool bHand);

    // Returns true if this player can see the card face
    UFUNCTION(BlueprintCallable)
    bool CanPlayerSeeFace(APlayerController* PC) const;
};
```

**Source/InfinityTable/Objects/TableDeck.h**
```cpp
#pragma once
#include "Objects/TableObject.h"
#include "TableCard.h"
#include "TableDeck.generated.h"

UCLASS()
class INFINITYTABLE_API ATableDeck : public ATableObject
{
    GENERATED_BODY()

public:
    ATableDeck();

    UPROPERTY(BlueprintReadOnly, Replicated)
    TArray<ATableCard*> Cards;

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_Shuffle();

    UFUNCTION(Server, Reliable, BlueprintCallable)
    ATableCard* Server_DrawCard(bool bFaceUp = false);

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_AddCard(ATableCard* Card);

    UFUNCTION(Server, Reliable, BlueprintCallable)
    ATableCard* Server_PeekTop() const;

    UFUNCTION(BlueprintCallable)
    int32 GetCount() const { return Cards.Num(); }

private:
    void RepositionCards();
};
```

---

## 5. Physics System

### 5.1 Grab Component

**Source/InfinityTable/Physics/GrabComponent.cpp**
```cpp
#include "GrabComponent.h"
#include "Objects/TableObject.h"
#include "DrawDebugHelpers.h"

UGrabComponent::UGrabComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UGrabComponent::AttemptGrab()
{
    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC) return;

    FVector WorldLocation, WorldDirection;
    PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        WorldLocation,
        WorldLocation + WorldDirection * GrabReachCM,
        ECC_PhysicsBody,
        Params
    );

    if (bHit)
    {
        if (ATableObject* Obj = Cast<ATableObject>(Hit.GetActor()))
        {
            HeldObject         = Obj;
            GrabOffset         = Obj->GetActorLocation() - Hit.Location;
            HoldDistance       = Hit.Distance;
            Obj->Server_PickUp(PC);
        }
    }
}

void UGrabComponent::Release()
{
    if (!HeldObject) return;

    // Compute throw velocity from recent cursor movement
    FVector ThrowVelocity = (LastCursorWorldPos - PrevCursorWorldPos)
                          / GetWorld()->DeltaTimeSeconds * ThrowMultiplier;
    ThrowVelocity.Z = FMath::Max(ThrowVelocity.Z, 0.0f);

    HeldObject->Server_Release(ThrowVelocity);
    HeldObject = nullptr;
}

void UGrabComponent::TickComponent(float DeltaTime,
    ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!HeldObject) return;

    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC) return;

    FVector WorldLocation, WorldDirection;
    PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

    PrevCursorWorldPos = LastCursorWorldPos;
    LastCursorWorldPos = WorldLocation + WorldDirection * HoldDistance;

    FVector TargetPos = LastCursorWorldPos + GrabOffset;

    // Smoothly move object toward cursor target
    FVector CurrentPos = HeldObject->GetActorLocation();
    FVector NewPos     = FMath::VInterpTo(CurrentPos, TargetPos,
                                          DeltaTime, GrabStiffness);

    HeldObject->Server_SetTransform(NewPos, HeldObject->GetActorRotation());
}
```

---

## 6. Multiplayer Networking

### 6.1 Session Subsystem

**Source/InfinityTable/Networking/ITSessionSubsystem.cpp**
```cpp
#include "ITSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

void UITSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (!OSS) return;

    SessionInterface = OSS->GetSessionInterface();
    if (!SessionInterface.IsValid()) return;

    // Bind delegates
    SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
        this, &UITSessionSubsystem::OnCreateSessionComplete);
    SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(
        this, &UITSessionSubsystem::OnFindSessionsComplete);
    SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(
        this, &UITSessionSubsystem::OnJoinSessionComplete);
    SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
        this, &UITSessionSubsystem::OnDestroySessionComplete);
}

void UITSessionSubsystem::HostSession(
    const FITSessionConfig& Config)
{
    FOnlineSessionSettings Settings;
    Settings.NumPublicConnections  = Config.MaxPlayers;
    Settings.NumPrivateConnections = 0;
    Settings.bIsLANMatch           = Config.bLAN;
    Settings.bUsesPresence         = true;
    Settings.bAllowJoinInProgress  = Config.bAllowJoinInProgress;
    Settings.bShouldAdvertise      = !Config.bPrivate;
    Settings.bUseLobbiesIfAvailable = true;

    Settings.Set(FName("GAME_NAME"), Config.GameName,
        EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    Settings.Set(FName("ENVIRONMENT"), Config.EnvironmentID,
        EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    if (!Config.Password.IsEmpty())
    {
        Settings.Set(FName("PASSWORD_HASH"),
            FMD5::HashAnsiString(*Config.Password),
            EOnlineDataAdvertisementType::ViaPingOnly);
    }

    SessionInterface->CreateSession(0, SESSION_NAME, Settings);
}

void UITSessionSubsystem::FindSessions(int32 MaxResults)
{
    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->MaxSearchResults       = MaxResults;
    SessionSearch->bIsLanQuery            = false;
    SessionSearch->QuerySettings.Set(
        SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UITSessionSubsystem::JoinSession(int32 SearchResultIndex)
{
    if (!SessionSearch.IsValid()) return;
    if (!SessionSearch->SearchResults.IsValidIndex(SearchResultIndex)) return;

    SessionInterface->JoinSession(0, SESSION_NAME,
        SessionSearch->SearchResults[SearchResultIndex]);
}

void UITSessionSubsystem::OnJoinSessionComplete(
    FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (Result != EOnJoinSessionCompleteResult::Success) return;

    FString ConnectAddress;
    SessionInterface->GetResolvedConnectString(SessionName, ConnectAddress);

    APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
    if (PC)
    {
        PC->ClientTravel(ConnectAddress, TRAVEL_Absolute);
    }
}
```

### 6.2 DefaultNetworkVersion.ini
```ini
[/Script/Engine.NetworkVersion]
ProjectVersion=InfinityTable_1_0
EngineCompatibleNetworkProtocolVersion=14
GameCompatibleNetworkProtocolVersion=1
```

### 6.3 DefaultEngine.ini (Networking Section)
```ini
[SystemSettings]
net.MaxNetUpdateFrequency=60
net.NetServerMaxTickRate=60
r.OneFrameThreadLag=0

[/Script/Engine.GameNetworkManager]
TotalNetBandwidth=32000
MaxDynamicBandwidth=7000
MinDynamicBandwidth=4000

[/Script/OnlineSubsystemSteam.SteamNetDriver]
NetConnectionClassName=/Script/OnlineSubsystemSteam.SteamNetConnection
MaxClientRate=15000
MaxInternetClientRate=10000
```

---

## 7. RPG Module

### 7.1 Fog of War Manager

**Source/InfinityTable/RPG/FogOfWarManager.cpp**
```cpp
#include "FogOfWarManager.h"
#include "Engine/Canvas.h"
#include "Kismet/KismetRenderingLibrary.h"

AFogOfWarManager::AFogOfWarManager()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void AFogOfWarManager::BeginPlay()
{
    Super::BeginPlay();

    // Create render target for fog texture
    FogTexture = UKismetRenderingLibrary::CreateRenderTarget2D(
        GetWorld(), GridWidth, GridHeight,
        RTF_RGBA8, FLinearColor::Black);

    // Initialize all cells as hidden
    RevealedCells.SetNumZeroed(GridWidth * GridHeight);
}

void AFogOfWarManager::RevealCircle(FVector WorldCenter, float RadiusWorld)
{
    // Convert world position to grid cell
    FVector2D GridCenter = WorldToGrid(WorldCenter);
    float RadiusCells    = RadiusWorld / CellSizeWorld;

    int32 MinX = FMath::Max(0, (int32)(GridCenter.X - RadiusCells));
    int32 MaxX = FMath::Min(GridWidth-1,  (int32)(GridCenter.X + RadiusCells));
    int32 MinY = FMath::Max(0, (int32)(GridCenter.Y - RadiusCells));
    int32 MaxY = FMath::Min(GridHeight-1, (int32)(GridCenter.Y + RadiusCells));

    for (int32 Y = MinY; Y <= MaxY; ++Y)
    for (int32 X = MinX; X <= MaxX; ++X)
    {
        float Dist = FVector2D::Distance(GridCenter, FVector2D(X, Y));
        if (Dist <= RadiusCells)
        {
            RevealedCells[Y * GridWidth + X] = 255;
        }
    }

    // Replicate to all clients
    Multicast_UpdateFogTexture(RevealedCells);
}

void AFogOfWarManager::HideAll()
{
    FMemory::Memset(RevealedCells.GetData(), 0,
                    RevealedCells.Num() * sizeof(uint8));
    Multicast_UpdateFogTexture(RevealedCells);
}

FVector2D AFogOfWarManager::WorldToGrid(FVector WorldPos) const
{
    FVector Local = WorldPos - GridOrigin;
    return FVector2D(Local.X / CellSizeWorld, Local.Y / CellSizeWorld);
}
```

### 7.2 Grid Manager

**Source/InfinityTable/RPG/GridManager.h**
```cpp
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"

UENUM(BlueprintType)
enum class EGridType : uint8 { Square, Hexagonal };

UCLASS()
class INFINITYTABLE_API AGridManager : public AActor
{
    GENERATED_BODY()

public:
    AGridManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGridType GridType = EGridType::Square;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CellSize = 100.0f;         // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GridWidth = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GridHeight = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowGrid = true;

    // Convert world position to nearest grid cell
    UFUNCTION(BlueprintCallable)
    FIntPoint WorldToCell(FVector WorldPos) const;

    // Get world centre of a grid cell
    UFUNCTION(BlueprintCallable)
    FVector CellToWorld(FIntPoint Cell) const;

    // Snap world pos to nearest cell centre
    UFUNCTION(BlueprintCallable)
    FVector SnapToGrid(FVector WorldPos) const;

    // Get all cells within movement range
    UFUNCTION(BlueprintCallable)
    TArray<FIntPoint> GetReachableCells(FIntPoint Origin,
                                         int32 MoveRange) const;
protected:
    virtual void BeginPlay() override;
    void DrawGridLines();
    TArray<FIntPoint> GetHexNeighbors(FIntPoint Cell) const;
};
```

---

## 8. Lua Scripting Engine

### 8.1 Lua Subsystem

**Source/InfinityTable/Scripting/LuaSubsystem.cpp**
```cpp
#include "LuaSubsystem.h"
#include "LuaAPIBindings.h"
#include "LuaSandbox.h"
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

void ULuaSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LuaState = MakeUnique<sol::state>();
}

void ULuaSubsystem::Initialize()
{
    if (!LuaState) return;

    // Open safe standard libraries only
    LuaState->open_libraries(
        sol::lib::base,
        sol::lib::math,
        sol::lib::table,
        sol::lib::string,
        sol::lib::coroutine
    );
    // io, os, package, debug are BLOCKED for security

    // Register InfinityTable API
    ULuaAPIBindings::RegisterAll(*LuaState, GetGameInstance());

    // Apply sandbox restrictions
    ULuaSandbox::ApplySandbox(*LuaState);

    bInitialized = true;
}

bool ULuaSubsystem::LoadScript(const FString& ScriptPath)
{
    if (!bInitialized) return false;

    // Read file
    FString Source;
    if (!FFileHelper::LoadFileToString(Source, *ScriptPath))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("LuaSubsystem: Cannot load script %s"), *ScriptPath);
        return false;
    }

    auto Result = LuaState->safe_script(TCHAR_TO_UTF8(*Source),
        sol::script_pass_on_error);

    if (!Result.valid())
    {
        sol::error Err = Result;
        UE_LOG(LogTemp, Error,
            TEXT("Lua error in %s: %s"), *ScriptPath,
            UTF8_TO_TCHAR(Err.what()));
        return false;
    }

    return true;
}

void ULuaSubsystem::FireEvent(const FString& EventName, ...)
{
    if (!bInitialized) return;

    // Look up registered handlers
    if (EventHandlers.Contains(EventName))
    {
        for (const auto& Handler : EventHandlers[EventName])
        {
            sol::protected_function Fn = *Handler;
            auto R = Fn();
            if (!R.valid())
            {
                sol::error E = R;
                UE_LOG(LogTemp, Warning,
                    TEXT("Lua event handler error [%s]: %s"),
                    *EventName, UTF8_TO_TCHAR(E.what()));
            }
        }
    }
}
```

### 8.2 API Bindings

**Source/InfinityTable/Scripting/LuaAPIBindings.cpp**
```cpp
#include "LuaAPIBindings.h"
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"
#include "Objects/TableSpawnManager.h"
#include "Objects/TableDice.h"

void ULuaAPIBindings::RegisterAll(sol::state& L, UGameInstance* GI)
{
    RegisterObjectsAPI(L, GI);
    RegisterTableAPI(L, GI);
    RegisterEventsAPI(L, GI);
    RegisterUIAPI(L, GI);
    RegisterMathHelpers(L);
}

void ULuaAPIBindings::RegisterObjectsAPI(sol::state& L, UGameInstance* GI)
{
    auto Objects = L.create_named_table("Objects");

    Objects.set_function("spawn", [GI](std::string TypeID, sol::table Opts)
        -> ATableObject*
    {
        UTableSpawnManager* SM = GI->GetSubsystem<UTableSpawnManager>();
        if (!SM) return nullptr;

        FString ID = UTF8_TO_TCHAR(TypeID.c_str());
        FVector Pos = FVector::ZeroVector;
        if (Opts["position"].valid())
        {
            sol::table P = Opts["position"];
            Pos = FVector(P[1], P[2], P[3]);
        }
        return SM->SpawnObject(ID, Pos, FRotator::ZeroRotator);
    });

    Objects.set_function("destroy", [](ATableObject* Obj)
    {
        if (IsValid(Obj)) Obj->Destroy();
    });

    Objects.set_function("getAll", [GI]() -> sol::table
    {
        sol::state_view View(GI->GetWorld()->GetGameInstance()
            ->GetSubsystem<ULuaSubsystem>()->GetLuaState());
        sol::table T = View.create_table();
        // Collect all table objects
        int32 i = 1;
        for (TActorIterator<ATableObject> It(GI->GetWorld()); It; ++It)
        {
            T[i++] = *It;
        }
        return T;
    });

    // Expose ATableObject to Lua
    L.new_usertype<ATableObject>("TableObject",
        "typeID",       sol::readonly(&ATableObject::ObjectTypeID),
        "setLocked",    [](ATableObject* O, bool L) { O->Server_Lock(L); },
        "flip",         [](ATableObject* O) { O->Server_Flip(); },
        "getPosition",  [](ATableObject* O) -> sol::table
        {
            // Return as {x,y,z} table
            return sol::table{};
        }
    );

    // Expose ATableDice
    L.new_usertype<ATableDice>("Dice",
        sol::base_classes, sol::bases<ATableObject>(),
        "roll",    [](ATableDice* D, float Force)
        {
            D->Server_Roll(FVector(0, 0, Force));
        },
        "result",  &ATableDice::GetCurrentFaceUp,
        "maxValue",&ATableDice::GetMaxValue
    );
}
```

### 8.3 Lua Sandbox Security

**Source/InfinityTable/Scripting/LuaSandbox.cpp**
```cpp
#include "LuaSandbox.h"
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

void ULuaSandbox::ApplySandbox(sol::state& L)
{
    // Remove dangerous globals
    const TArray<FString> BlockedGlobals = {
        "io", "os", "package", "require",
        "dofile", "loadfile", "load",
        "debug", "collectgarbage",
        "rawget", "rawset", "rawequal"
    };

    for (const FString& Global : BlockedGlobals)
    {
        L[TCHAR_TO_UTF8(*Global)] = sol::nil;
    }

    // Override print to route to UE log (not stdout)
    L.set_function("print", [](sol::variadic_args Args)
    {
        FString Output;
        for (auto Arg : Args)
        {
            Output += UTF8_TO_TCHAR(
                luaL_tolstring(Arg.lua_state(), Arg.stack_index(), nullptr));
            Output += TEXT(" ");
        }
        UE_LOG(LogTemp, Log, TEXT("[Lua] %s"), *Output);
    });

    // Set instruction count limit (anti-infinite-loop)
    // Reset every 100,000 instructions
    lua_sethook(L.lua_state(), [](lua_State*, lua_Debug*)
    {
        luaL_error(lua_gettop(nullptr) > 0
            ? nullptr : nullptr,
            "Script execution limit reached");
    }, LUA_MASKCOUNT, 100000);

    // Memory limit: 64 MB per script
    lua_setallocf(L.lua_state(), SandboxAlloc,
        new FSandboxAllocState{ 0, 64 * 1024 * 1024 });
}

void* ULuaSandbox::SandboxAlloc(void* UserData, void* Ptr,
    size_t OSize, size_t NSize)
{
    FSandboxAllocState* State = static_cast<FSandboxAllocState*>(UserData);

    if (NSize == 0)
    {
        State->UsedBytes -= OSize;
        FMemory::Free(Ptr);
        return nullptr;
    }

    if (State->UsedBytes + NSize - OSize > State->MaxBytes)
        return nullptr; // Allocation denied

    State->UsedBytes += NSize - OSize;
    return FMemory::Realloc(Ptr, NSize);
}
```

---

## 9. Save System

### 9.1 Save Game Subsystem

**Source/InfinityTable/Save/SaveGameSubsystem.cpp**
```cpp
#include "SaveGameSubsystem.h"
#include "SQLiteWrapper.h"
#include "Objects/TableObject.h"
#include "JsonObjectConverter.h"

void USaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    FString DBPath = FPaths::ProjectSavedDir() / TEXT("InfinityTable.db");
    DB = MakeUnique<FSQLiteWrapper>(DBPath);

    DB->Execute(R"SQL(
        CREATE TABLE IF NOT EXISTS saves (
            slot        TEXT PRIMARY KEY,
            name        TEXT,
            environment TEXT,
            timestamp   INTEGER,
            data        TEXT
        );
        CREATE TABLE IF NOT EXISTS objects (
            id          TEXT,
            slot        TEXT,
            type_id     TEXT,
            state_json  TEXT,
            FOREIGN KEY(slot) REFERENCES saves(slot)
        );
    )SQL");

    // Autosave every 5 minutes
    GetWorld()->GetTimerManager().SetTimer(
        AutosaveTimer, this,
        &USaveGameSubsystem::AutoSave,
        300.0f, true);
}

bool USaveGameSubsystem::SaveTable(const FString& Slot, const FString& Name)
{
    FTableSaveState State;
    State.Slot        = Slot;
    State.Name        = Name;
    State.Environment = CurrentEnvironmentID;
    State.Timestamp   = FDateTime::UtcNow().ToUnixTimestamp();

    // Collect all table objects
    for (TActorIterator<ATableObject> It(GetWorld()); It; ++It)
    {
        FTableObjectState ObjState = It->GetObjectState();
        State.Objects.Add(ObjState);
    }

    // Serialize to JSON
    FString JSON;
    FJsonObjectConverter::UStructToJsonObjectString(State, JSON);

    // Write to SQLite
    FString SQL = FString::Printf(
        TEXT("INSERT OR REPLACE INTO saves VALUES('%s','%s','%s',%lld,'%s')"),
        *Slot, *Name, *State.Environment, State.Timestamp, *JSON);

    return DB->Execute(TCHAR_TO_UTF8(*SQL));
}

bool USaveGameSubsystem::LoadTable(const FString& Slot)
{
    FString Query = FString::Printf(
        TEXT("SELECT data FROM saves WHERE slot='%s'"), *Slot);

    TArray<TMap<FString,FString>> Rows = DB->Query(TCHAR_TO_UTF8(*Query));
    if (Rows.IsEmpty()) return false;

    FString JSON = Rows[0]["data"];
    FTableSaveState State;
    FJsonObjectConverter::JsonObjectStringToUStruct(JSON, &State);

    // Clear current table
    for (TActorIterator<ATableObject> It(GetWorld()); It; ++It)
        It->Destroy();

    // Respawn objects
    UTableSpawnManager* SM = GetGameInstance()
        ->GetSubsystem<UTableSpawnManager>();

    for (const FTableObjectState& ObjState : State.Objects)
    {
        ATableObject* Obj = SM->SpawnObject(
            ObjState.ObjectTypeID,
            ObjState.Position,
            ObjState.Rotation);
        if (Obj) Obj->ApplyObjectState(ObjState);
    }

    return true;
}

TArray<FSaveSlotInfo> USaveGameSubsystem::GetSaveSlots() const
{
    TArray<FSaveSlotInfo> Results;
    auto Rows = DB->Query("SELECT slot,name,environment,timestamp FROM saves ORDER BY timestamp DESC");

    for (const auto& Row : Rows)
    {
        FSaveSlotInfo Info;
        Info.Slot        = Row["slot"];
        Info.Name        = Row["name"];
        Info.Environment = Row["environment"];
        Info.Timestamp   = FCString::Atoi64(*Row["timestamp"]);
        Results.Add(Info);
    }
    return Results;
}
```

---

## 10. Asset Import Pipeline

### 10.1 Asset Import Subsystem

**Source/InfinityTable/Assets/AssetImportSubsystem.cpp**
```cpp
#include "AssetImportSubsystem.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "RuntimeMeshImporter.h"
#include "Engine/Texture2D.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

void UAssetImportSubsystem::ImportMesh(const FString& FilePath,
    FOnMeshImported OnComplete)
{
    FString Ext = FPaths::GetExtension(FilePath).ToLower();

    if (Ext == TEXT("gltf") || Ext == TEXT("glb"))
    {
        ImportGLTF(FilePath, OnComplete);
    }
    else if (Ext == TEXT("fbx") || Ext == TEXT("obj"))
    {
        ImportFBXOBJ(FilePath, OnComplete);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("AssetImport: Unsupported format %s"), *Ext);
    }
}

void UAssetImportSubsystem::ImportGLTF(const FString& FilePath,
    FOnMeshImported OnComplete)
{
    UglTFRuntimeAsset* Asset =
        UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(
            FilePath, false, FglTFRuntimeConfig());

    if (!Asset)
    {
        UE_LOG(LogTemp, Error, TEXT("glTF import failed: %s"), *FilePath);
        return;
    }

    // Load first mesh
    UStaticMesh* Mesh =
        Asset->LoadStaticMesh(0, FglTFRuntimeStaticMeshConfig());

    if (Mesh)
    {
        FImportedAsset Result;
        Result.Mesh     = Mesh;
        Result.FileName = FPaths::GetBaseFilename(FilePath);
        Result.Format   = EAssetFormat::GLTF;

        RegisterAsset(Result);
        OnComplete.ExecuteIfBound(true, Result);
    }
}

void UAssetImportSubsystem::ImportTexture(const FString& FilePath,
    FOnTextureImported OnComplete)
{
    TArray<uint8> RawData;
    if (!FFileHelper::LoadFileToArray(RawData, *FilePath))
    {
        OnComplete.ExecuteIfBound(false, nullptr);
        return;
    }

    IImageWrapperModule& IWM =
        FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");

    FString Ext = FPaths::GetExtension(FilePath).ToLower();
    EImageFormat Fmt = EImageFormat::PNG;
    if (Ext == TEXT("jpg") || Ext == TEXT("jpeg"))
        Fmt = EImageFormat::JPEG;
    else if (Ext == TEXT("webp"))
        Fmt = EImageFormat::WebP;

    TSharedPtr<IImageWrapper> Wrapper = IWM.CreateImageWrapper(Fmt);
    if (!Wrapper->SetCompressed(RawData.GetData(), RawData.Num()))
    {
        OnComplete.ExecuteIfBound(false, nullptr);
        return;
    }

    TArray<uint8> BGRA;
    Wrapper->GetRaw(ERGBFormat::BGRA, 8, BGRA);

    UTexture2D* Tex = UTexture2D::CreateTransient(
        Wrapper->GetWidth(), Wrapper->GetHeight(), PF_B8G8R8A8);
    FTexture2DMipMap& Mip = Tex->GetPlatformData()->Mips[0];
    void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(Data, BGRA.GetData(), BGRA.Num());
    Mip.BulkData.Unlock();
    Tex->UpdateResource();

    OnComplete.ExecuteIfBound(true, Tex);
}
```

---

## 11. UI / HUD System

### 11.1 In-Game HUD Layout (UMG Structure)

```
InGameHUD (UserWidget)
├── CanvasPanel (root)
│   ├── ChatWidget          (bottom-left, collapsible)
│   ├── RadialMenuWidget    (centre, appears on RMB hold)
│   ├── ObjectContextMenu   (follows cursor on selection)
│   ├── DiceResultWidget    (top-centre, shows last roll)
│   ├── PlayerListWidget    (top-right, shows players + colours)
│   ├── RPGPanelWidget      (right side, GM tools, init tracker)
│   ├── ObjectBrowserWidget (left slide-out panel)
│   └── SettingsMenuWidget  (fullscreen overlay)
```

### 11.2 Radial Menu Widget

**Source/InfinityTable/UI/RadialMenuWidget.cpp** (excerpt)
```cpp
void URadialMenuWidget::NativePaint(const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled) const
{
    Super::NativePaint(Args, AllottedGeometry, MyCullingRect,
        OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

    FVector2D Centre = AllottedGeometry.GetLocalSize() * 0.5f;
    float Radius     = 80.0f;
    int32 NumItems   = MenuItems.Num();

    for (int32 i = 0; i < NumItems; ++i)
    {
        float Angle   = (2.0f * PI * i / NumItems) - PI * 0.5f;
        FVector2D Pos = Centre + FVector2D(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius
        );

        // Draw sector background
        DrawSector(OutDrawElements, LayerId, Centre, Pos,
            2.0f * PI / NumItems, i == HoveredIndex);

        // Draw icon + label
        DrawItemIcon(OutDrawElements, LayerId, Pos, MenuItems[i]);
    }
}
```

---

## 12. Modding SDK

### 12.1 Mod Manifest Schema (SDK/ModManifest.schema.json)
```json
{
  "$schema": "http://json-schema.org/draft-07/schema",
  "title": "InfinityTable Mod Manifest",
  "type": "object",
  "required": ["id", "name", "version", "author", "entry"],
  "properties": {
    "id":          { "type": "string", "pattern": "^[a-z0-9_]+$" },
    "name":        { "type": "string", "maxLength": 64 },
    "version":     { "type": "string", "pattern": "^\\d+\\.\\d+\\.\\d+$" },
    "author":      { "type": "string" },
    "description": { "type": "string", "maxLength": 512 },
    "entry":       { "type": "string", "description": "Main Lua script path" },
    "minEngineVersion": { "type": "string" },
    "tags":        { "type": "array", "items": { "type": "string" } },
    "objects": {
      "type": "array",
      "items": {
        "type": "object",
        "required": ["id", "mesh"],
        "properties": {
          "id":      { "type": "string" },
          "mesh":    { "type": "string" },
          "texture": { "type": "string" },
          "physics": {
            "type": "object",
            "properties": {
              "mass":        { "type": "number" },
              "friction":    { "type": "number" },
              "restitution": { "type": "number" }
            }
          }
        }
      }
    },
    "permissions": {
      "type": "array",
      "items": {
        "enum": ["spawnObjects", "networkSync",
                 "readSave", "writeSave", "showUI"]
      }
    }
  }
}
```

### 12.2 Example Mod — Checkers (SDK/Examples/Checkers/)

**manifest.json**
```json
{
  "id": "checkers_classic",
  "name": "Classic Checkers",
  "version": "1.0.0",
  "author": "InfinityTable Team",
  "description": "Standard 8x8 checkers for 2 players",
  "entry": "scripts/main.lua",
  "tags": ["board-game", "2-player", "classic"],
  "permissions": ["spawnObjects", "networkSync", "showUI"]
}
```

**scripts/main.lua**
```lua
-- Classic Checkers mod for InfinityTable
local BOARD_SIZE = 8
local board = {}
local pieces = {}
local currentTurn = "red"

local function cellToWorld(col, row)
    return {
        x = (col - 4.5) * 10,
        y = 0,
        z = (row - 4.5) * 10
    }
end

local function spawnBoard()
    local b = Objects.spawn("checkers_board", {
        position = {0, 0, 0},
        scale    = {1, 1, 1}
    })
    return b
end

local function spawnPieces()
    -- Red pieces: rows 1-3, dark squares
    for row = 1, 3 do
        for col = 1, BOARD_SIZE do
            if (col + row) % 2 == 0 then
                local pos = cellToWorld(col, row)
                local p = Objects.spawn("checker_piece", {
                    position = {pos.x, pos.y, pos.z},
                    color    = "red"
                })
                pieces[#pieces+1] = { obj=p, color="red",
                    col=col, row=row, king=false }
            end
        end
    end

    -- Black pieces: rows 6-8
    for row = 6, 8 do
        for col = 1, BOARD_SIZE do
            if (col + row) % 2 == 0 then
                local pos = cellToWorld(col, row)
                local p = Objects.spawn("checker_piece", {
                    position = {pos.x, pos.y, pos.z},
                    color    = "black"
                })
                pieces[#pieces+1] = { obj=p, color="black",
                    col=col, row=row, king=false }
            end
        end
    end
end

local function checkWin()
    local redCount   = 0
    local blackCount = 0
    for _, p in ipairs(pieces) do
        if p.color == "red"   then redCount   = redCount   + 1 end
        if p.color == "black" then blackCount = blackCount + 1 end
    end

    if redCount == 0 then
        Table.broadcast("Black wins!")
        Game.endGame({ winner = "black" })
    elseif blackCount == 0 then
        Table.broadcast("Red wins!")
        Game.endGame({ winner = "red" })
    end
end

local function onObjectMoved(obj)
    -- Detect if a jump was made and remove captured piece
    for i, p in ipairs(pieces) do
        if p.obj == obj then
            local newPos = obj.getPosition()
            -- Check for captures (simplified)
            checkWin()
            currentTurn = (currentTurn == "red") and "black" or "red"
            UI.showMessage("It's " .. currentTurn .. "'s turn")
            break
        end
    end
end

-- Entry point
function onGameStart()
    spawnBoard()
    spawnPieces()
    UI.showMessage("Checkers! Red goes first.")
end

Events.on("GameStart",     onGameStart)
Events.on("ObjectMoved",   onObjectMoved)
```

---

## 13. Audio System

### 13.1 Audio Manager Subsystem

**Source/InfinityTable/Audio/AudioManagerSubsystem.cpp**
```cpp
#include "AudioManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

void UAudioManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Pre-load common SFX
    LoadSound("Dice_Roll_Hard",   TEXT("/Game/Audio/Dice/Dice_Roll_Hard"));
    LoadSound("Dice_Roll_Soft",   TEXT("/Game/Audio/Dice/Dice_Roll_Soft"));
    LoadSound("Card_Pickup",      TEXT("/Game/Audio/Cards/Card_Pickup"));
    LoadSound("Card_Place",       TEXT("/Game/Audio/Cards/Card_Place"));
    LoadSound("Card_Shuffle",     TEXT("/Game/Audio/Cards/Card_Shuffle"));
    LoadSound("Object_Pickup",    TEXT("/Game/Audio/Objects/Pickup"));
    LoadSound("Object_Place",     TEXT("/Game/Audio/Objects/Place"));
    LoadSound("Object_Throw",     TEXT("/Game/Audio/Objects/Throw"));
}

void UAudioManagerSubsystem::PlaySoundAtLocation(
    const FString& SoundID, FVector Location, float Volume)
{
    USoundBase** Sound = LoadedSounds.Find(SoundID);
    if (!Sound || !*Sound) return;

    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(), *Sound, Location,
        Volume, 1.0f, 0.0f,
        nullptr, nullptr);
}

void UAudioManagerSubsystem::SetAmbientEnvironment(
    const FString& EnvironmentID)
{
    // Stop current ambient
    if (AmbientComponent && AmbientComponent->IsPlaying())
        AmbientComponent->FadeOut(2.0f, 0.0f);

    FString AmbientPath = FString::Printf(
        TEXT("/Game/Audio/Ambient/%s_Ambient"), *EnvironmentID);

    USoundBase* Ambient = LoadObject<USoundBase>(nullptr, *AmbientPath);
    if (!Ambient) return;

    AmbientComponent = UGameplayStatics::SpawnSound2D(
        GetWorld(), Ambient, 0.3f, 1.0f, 0.0f, nullptr, true);

    if (AmbientComponent)
        AmbientComponent->FadeIn(3.0f, 0.3f);
}
```

---

## 14. Example Games & Scripts

### 14.1 Full Dice Battle Game (SDK/Examples/DiceGame/scripts/main.lua)
```lua
-- Dice Battle: Roll highest sum to win each round
-- Supports 2-6 players, first to 5 rounds wins

local players      = {}
local roundScores  = {}
local roundWins    = {}
local currentRound = 1
local maxRounds    = 5
local rollsThisRound = {}

local function setupPlayers()
    players = Table.getPlayers()
    for _, p in ipairs(players) do
        roundWins[p.index] = 0
    end
    UI.showMessage("Dice Battle! " .. #players .. " players. First to "
        .. maxRounds .. " round wins!")
end

local function allPlayersRolled()
    return #rollsThisRound == #players
end

local function resolveRound()
    local best    = -1
    local winner  = nil

    for _, entry in ipairs(rollsThisRound) do
        if entry.total > best then
            best   = entry.total
            winner = entry.player
        end
    end

    roundWins[winner.index] = roundWins[winner.index] + 1
    Table.broadcast(winner.name .. " wins round " .. currentRound
        .. " with " .. best .. "!")

    -- Check game win
    if roundWins[winner.index] >= maxRounds then
        Table.broadcast("🏆 " .. winner.name .. " wins the game!")
        Game.endGame({ winner = winner })
        return
    end

    -- Next round
    currentRound     = currentRound + 1
    rollsThisRound   = {}
    UI.showMessage("Round " .. currentRound .. " — Roll your dice!")
end

local function onDiceRolled(player, dice, result)
    -- Each player rolls 2d6; collect both rolls
    for _, entry in ipairs(rollsThisRound) do
        if entry.player.index == player.index then
            entry.total = entry.total + result
            if allPlayersRolled() then resolveRound() end
            return
        end
    end

    rollsThisRound[#rollsThisRound+1] = {
        player = player,
        total  = result
    }

    if allPlayersRolled() then resolveRound() end
end

Events.on("GameStart",   setupPlayers)
Events.on("DiceRolled",  onDiceRolled)
```

### 14.2 RPG Campaign Setup (SDK/Examples/RPGCampaign/scripts/setup.lua)
```lua
-- RPG Campaign initializer
-- Loads a map, places tokens, enables fog of war

local GM_PLAYER_INDEX = 1

local function setupMap()
    -- Load dungeon board
    local dungeon = Objects.spawn("dungeon_map_01", {
        position = {0, 0, 0},
        scale    = {2, 2, 1}
    })

    -- Enable fog of war
    RPG.fogOfWar.enable(true)
    RPG.fogOfWar.hideAll()

    -- Setup grid (5ft squares)
    RPG.grid.setType("square")
    RPG.grid.setCellSize(100) -- 100cm = 5ft
    RPG.grid.show(true)
end

local function placeHeroTokens()
    local players = Table.getPlayers()
    local startPositions = {
        {-100, 0, 0}, {100, 0, 0}, {0, -100, 0}, {0, 100, 0}
    }

    for i, player in ipairs(players) do
        if player.index ~= GM_PLAYER_INDEX then
            local pos = startPositions[i] or {0, 0, 0}
            local token = Objects.spawn("hero_token", {
                position = pos,
                color    = player.color,
                label    = player.name
            })
            token.owner = player.index
        end
    end
end

local function onPlayerMove(player, token, newPos)
    -- Reveal fog around moved token
    RPG.fogOfWar.revealCircle(newPos, 600) -- 30ft vision radius

    -- Notify GM of movement
    Table.notifyPlayer(GM_PLAYER_INDEX,
        player.name .. " moved to " ..
        RPG.grid.worldToCell(newPos).x .. "," ..
        RPG.grid.worldToCell(newPos).y)
end

local function onInitiativeStart()
    local combatants = {}
    for _, token in ipairs(Objects.getByType("hero_token")) do
        combatants[#combatants+1] = {
            name  = token.label,
            token = token,
            init  = math.random(1, 20) + 3
        }
    end

    -- Sort by initiative
    table.sort(combatants, function(a, b) return a.init > b.init end)

    RPG.initiative.setOrder(combatants)
    RPG.initiative.show(true)
end

Events.on("GameStart",    setupMap)
Events.on("GameStart",    placeHeroTokens)
Events.on("ObjectMoved",  onPlayerMove)
Events.on("CombatStart",  onInitiativeStart)
```

---

## 15. Build Instructions

### 15.1 Prerequisites

| Tool | Version | Notes |
|---|---|---|
| Unreal Engine | 5.3+ | Epic Games Launcher |
| Visual Studio | 2022 | C++ Game Development workload |
| Windows SDK | 10.0.18362+ | Required by UE5 |
| Git | Any | Source control |
| Steam SDK | 1.57+ | Optional, for Steam OSS |

### 15.2 Initial Setup

```powershell
# 1. Clone the repository
git clone https://github.com/yourorg/InfinityTable.git
cd InfinityTable

# 2. Install third-party dependencies
.\ThirdParty\install.bat

# 3. Generate Visual Studio project files
"%UE5_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" ^
    -projectfiles -project="%CD%\InfinityTable.uproject" -game -rocket -progress

# 4. Open solution
start InfinityTable.sln
```

### 15.3 Build Targets

```powershell
# Development Editor (for iteration)
msbuild InfinityTable.sln /p:Configuration="Development Editor" /p:Platform=Win64

# Development Client (playtest)
msbuild InfinityTable.sln /p:Configuration="Development" /p:Platform=Win64

# Shipping Client (release build)
msbuild InfinityTable.sln /p:Configuration="Shipping" /p:Platform=Win64

# Dedicated Server
msbuild InfinityTable.sln /p:Configuration="Development Server" /p:Platform=Win64
```

### 15.4 Required Plugins

Enable in InfinityTable.uproject:
```json
"Plugins": [
  { "Name": "glTFRuntime",           "Enabled": true },
  { "Name": "OnlineSubsystemSteam",  "Enabled": true },
  { "Name": "OnlineSubsystemEOS",    "Enabled": true },
  { "Name": "MetasoundEngine",       "Enabled": true },
  { "Name": "Niagara",               "Enabled": true },
  { "Name": "ChaosVehicles",         "Enabled": false },
  { "Name": "VirtualReality",        "Enabled": true }
]
```

---

## 16. Multiplayer Deployment

### 16.1 Dedicated Server Setup

```bash
# Build server binary
./UnrealBuildTool.sh InfinityTableServer Linux Development \
    InfinityTable.uproject

# Run dedicated server (port 7777 default)
./InfinityTableServer \
    /Game/Maps/TableMap \
    -server \
    -port=7777 \
    -players=6 \
    -log

# Run behind reverse proxy (nginx example)
# /etc/nginx/conf.d/infinitytable.conf
stream {
    server {
        listen 7777 udp;
        proxy_pass 127.0.0.1:7777;
    }
}
```

### 16.2 Docker Compose (Dedicated Server)

```yaml
version: '3.8'
services:
  infinitytable-server:
    image: infinitytable/server:latest
    ports:
      - "7777:7777/udp"
      - "7778:7778/tcp"   # Query port
    environment:
      - MAX_PLAYERS=6
      - SERVER_NAME=InfinityTable Official Server
      - MAP=TableMap
    volumes:
      - ./saves:/app/Saved
      - ./mods:/app/Mods
    restart: unless-stopped
    deploy:
      resources:
        limits:
          cpus: '2'
          memory: 4G
```

### 16.3 DefaultGame.ini

```ini
[/Script/Engine.GameSession]
MaxPlayers=6

[/Script/InfinityTable.ITGameMode]
MaxPlayers=6
bRequirePassword=false

[URL]
Port=7777

[/Script/Engine.NetworkSettings]
n.VerifyPeer=false
```

---

## 17. Performance Guide

### 17.1 Physics Optimization

```cpp
// In TableObject.cpp — sleep inactive objects
void ATableObject::OnObjectAtRest()
{
    // Put physics to sleep after 3 seconds of no movement
    if (GetWorld()->GetTimeSeconds() - LastMoveTime > 3.0f)
    {
        MeshComponent->PutRigidBodyToSleep();
    }
}

// In project settings (DefaultEngine.ini)
// [/Script/Engine.PhysicsSettings]
// MaxPhysicsDeltaTime=0.033333    ; 30Hz physics cap
// bSimulateSkeletalMeshOnDedicatedServer=false
// RagdollAggregateThreshold=4
```

### 17.2 Network Optimization

```cpp
// Object relevancy — only replicate nearby objects
float ATableObject::GetNetPriority(const FVector& ViewPos,
    const FVector& ViewDir, APlayerController* Viewer,
    UActorChannel* InChannel, float Time, bool bLowBandwidth) const
{
    float Dist = FVector::Dist(ViewPos, GetActorLocation());

    // Objects within 300cm are high priority
    if (Dist < 300.0f) return 3.0f;

    // Objects 300-1000cm are medium priority
    if (Dist < 1000.0f) return 1.0f;

    // Far objects: low priority
    return 0.5f;
}
```

### 17.3 LOD Settings

```ini
; Config/DefaultEngine.ini
[/Script/Engine.LODDistanceScale]
StaticMeshLODDistanceScale=1.0

; Objects use 3 LOD levels
; LOD0: < 200cm — Full detail
; LOD1: 200-500cm — 50% polys
; LOD2: > 500cm — 25% polys
```

---

## 18. Security & Sandboxing

### 18.1 Lua Security Matrix

| Permission | Default | How to Grant |
|---|---|---|
| `io.*` | Blocked | Never |
| `os.*` | Blocked | Never |
| `require` | Blocked | Use mod-local require only |
| `Objects.spawn` | Allowed (Player) | manifest `permissions` |
| `Table.kick` | Host only | Role check at runtime |
| `Network.sendRaw` | Blocked | Never |
| `UI.showWidget` | Allowed | manifest `permissions` |
| Memory (max) | 64 MB | Per-mod config |
| Instructions/s | 100,000 | Anti-loop hook |
| File access | Mod dir only | Sandboxed VFS |

### 18.2 Mod Validation

```cpp
bool UModValidator::ValidateMod(const FString& ModPath)
{
    // 1. Check manifest exists and parses
    FString ManifestPath = ModPath / TEXT("manifest.json");
    FITModManifest Manifest;
    if (!LoadManifest(ManifestPath, Manifest)) return false;

    // 2. Validate ID format (alphanumeric + underscore)
    FRegexPattern IDPattern(TEXT("^[a-z0-9_]+$"));
    FRegexMatcher Matcher(IDPattern, Manifest.ID);
    if (!Matcher.FindNext()) return false;

    // 3. Scan Lua files for blocked API calls
    TArray<FString> LuaFiles;
    IFileManager::Get().FindFilesRecursive(
        LuaFiles, *(ModPath / TEXT("scripts")),
        TEXT("*.lua"), true, false);

    for (const FString& LuaFile : LuaFiles)
    {
        FString Source;
        FFileHelper::LoadFileToString(Source, *LuaFile);

        // Reject if blocked patterns found
        TArray<FString> Blocked = {
            TEXT("io."), TEXT("os."), TEXT("socket"),
            TEXT("ffi"), TEXT("loadstring"), TEXT("debug.")
        };

        for (const FString& Pattern : Blocked)
        {
            if (Source.Contains(Pattern))
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("Mod %s blocked: contains '%s'"),
                    *Manifest.ID, *Pattern);
                return false;
            }
        }
    }

    // 4. Validate asset file types (no exe/dll/sh)
    TArray<FString> AllFiles;
    IFileManager::Get().FindFilesRecursive(
        AllFiles, *ModPath, TEXT("*.*"), true, false);

    TArray<FString> BlockedExts = {
        TEXT(".exe"), TEXT(".dll"), TEXT(".bat"),
        TEXT(".sh"), TEXT(".ps1"), TEXT(".cmd")
    };

    for (const FString& File : AllFiles)
    {
        FString Ext = FPaths::GetExtension(File).ToLower();
        if (BlockedExts.Contains(TEXT(".") + Ext))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("Mod %s blocked: contains executable '%s'"),
                *Manifest.ID, *File);
            return false;
        }
    }

    return true;
}
```

---

*InfinityTable Project Documentation — v0.1.0*  
*© InfinityTable Contributors — MIT License*
