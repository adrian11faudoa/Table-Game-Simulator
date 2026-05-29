#include "ITGameMode.h"
#include "ITGameState.h"
#include "ITPlayerController.h"
#include "ITPlayerState.h"
#include "ITSpectatorPawn.h"
#include "ITHUD.h"
#include "Save/SaveGameSubsystem.h"
#include "Scripting/LuaSubsystem.h"

TMap<FString, TArray<EPlayerRole>> AITGameMode::PermissionMatrix;

AITGameMode::AITGameMode()
{
    GameStateClass        = AITGameState::StaticClass();
    PlayerControllerClass = AITPlayerController::StaticClass();
    PlayerStateClass      = AITPlayerState::StaticClass();
    DefaultPawnClass      = AITSpectatorPawn::StaticClass();
    HUDClass              = AITHUD::StaticClass();

    InitPermissionMatrix();
}

void AITGameMode::InitPermissionMatrix()
{
    if (PermissionMatrix.Num() > 0) return;

    PermissionMatrix.Add("SpawnObjects",  { EPlayerRole::Host, EPlayerRole::CoHost, EPlayerRole::Player });
    PermissionMatrix.Add("MoveObjects",   { EPlayerRole::Host, EPlayerRole::CoHost, EPlayerRole::Player });
    PermissionMatrix.Add("DeleteObjects", { EPlayerRole::Host, EPlayerRole::CoHost });
    PermissionMatrix.Add("RunScripts",    { EPlayerRole::Host, EPlayerRole::CoHost });
    PermissionMatrix.Add("KickPlayers",   { EPlayerRole::Host });
    PermissionMatrix.Add("LoadSave",      { EPlayerRole::Host, EPlayerRole::CoHost });
    PermissionMatrix.Add("GMTools",       { EPlayerRole::Host });
    PermissionMatrix.Add("ChangeEnv",     { EPlayerRole::Host, EPlayerRole::CoHost });
}

void AITGameMode::InitGame(const FString& MapName,
                            const FString& Options,
                            FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    if (ULuaSubsystem* Lua = GetGameInstance()->GetSubsystem<ULuaSubsystem>())
    {
        Lua->Initialize();
    }
}

APlayerController* AITGameMode::Login(UPlayer* NewPlayer,
                                       ENetRole InRemoteRole,
                                       const FString& Portal,
                                       const FString& Options,
                                       const FUniqueNetIdRepl& UniqueId,
                                       FString& ErrorMessage)
{
    // Password check
    if (bRequirePassword && !RoomPassword.IsEmpty())
    {
        FString ProvidedPW = UGameplayStatics::ParseOption(Options, TEXT("Password"));
        if (ProvidedPW != RoomPassword)
        {
            ErrorMessage = TEXT("Incorrect password.");
            return nullptr;
        }
    }

    return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
}

void AITGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    EPlayerRole Role = (PlayerRoles.Num() == 0)
        ? EPlayerRole::Host
        : EPlayerRole::Player;

    PlayerRoles.Add(NewPlayer, Role);

    if (AITPlayerState* PS = NewPlayer->GetPlayerState<AITPlayerState>())
        PS->SetRole(Role);

    if (ULuaSubsystem* Lua = GetGameInstance()->GetSubsystem<ULuaSubsystem>())
        Lua->FireEvent(TEXT("PlayerJoined"), NewPlayer);

    UE_LOG(LogTemp, Log, TEXT("Player joined: %s as %s"),
        *NewPlayer->GetName(),
        Role == EPlayerRole::Host ? TEXT("Host") : TEXT("Player"));
}

void AITGameMode::Logout(AController* Exiting)
{
    if (APlayerController* PC = Cast<APlayerController>(Exiting))
    {
        if (ULuaSubsystem* Lua = GetGameInstance()->GetSubsystem<ULuaSubsystem>())
            Lua->FireEvent(TEXT("PlayerLeft"), PC);

        PlayerRoles.Remove(PC);
    }
    Super::Logout(Exiting);
}

void AITGameMode::SetPlayerPermission(APlayerController* Player, EPlayerRole Role)
{
    if (!PlayerRoles.Contains(Player)) return;
    PlayerRoles[Player] = Role;

    if (AITPlayerState* PS = Player->GetPlayerState<AITPlayerState>())
        PS->SetRole(Role);
}

EPlayerRole AITGameMode::GetPlayerRole(APlayerController* Player) const
{
    const EPlayerRole* Role = PlayerRoles.Find(Player);
    return Role ? *Role : EPlayerRole::Spectator;
}

bool AITGameMode::HasPermission(APlayerController* Player, const FString& Action) const
{
    EPlayerRole Role = GetPlayerRole(Player);
    const TArray<EPlayerRole>* Allowed = PermissionMatrix.Find(Action);
    if (!Allowed) return false;
    return Allowed->Contains(Role);
}

void AITGameMode::LoadTableState(const FString& SaveSlotName)
{
    if (USaveGameSubsystem* SGS = GetGameInstance()->GetSubsystem<USaveGameSubsystem>())
        SGS->LoadTable(SaveSlotName);
}

void AITGameMode::SaveTableState(const FString& SaveSlotName)
{
    if (USaveGameSubsystem* SGS = GetGameInstance()->GetSubsystem<USaveGameSubsystem>())
        SGS->SaveTable(SaveSlotName, SaveSlotName);
}
