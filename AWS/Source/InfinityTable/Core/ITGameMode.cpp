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

void AITGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    if (ULuaSubsystem* Lua = GetGameInstance()->GetSubsystem<ULuaSubsystem>())
        Lua->Initialize();
}

void AITGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    EPlayerRole Role = PlayerRoles.IsEmpty() ? EPlayerRole::Host : EPlayerRole::Player;
    PlayerRoles.Add(NewPlayer, Role);
    if (AITPlayerState* PS = NewPlayer->GetPlayerState<AITPlayerState>())
        PS->SetRole(Role);
    if (ULuaSubsystem* Lua = GetGameInstance()->GetSubsystem<ULuaSubsystem>())
        Lua->FireEvent(TEXT("PlayerJoined"));
}

void AITGameMode::Logout(AController* Exiting)
{
    if (APlayerController* PC = Cast<APlayerController>(Exiting))
    {
        if (ULuaSubsystem* Lua = GetGameInstance()->GetSubsystem<ULuaSubsystem>())
            Lua->FireEvent(TEXT("PlayerLeft"));
        PlayerRoles.Remove(PC);
    }
    Super::Logout(Exiting);
}

void AITGameMode::SetPlayerPermission(APlayerController* Player, EPlayerRole Role)
{
    if (PlayerRoles.Contains(Player)) PlayerRoles[Player] = Role;
}

EPlayerRole AITGameMode::GetPlayerRole(APlayerController* Player) const
{
    const EPlayerRole* R = PlayerRoles.Find(Player);
    return R ? *R : EPlayerRole::Spectator;
}

bool AITGameMode::HasPermission(APlayerController* Player, const FString& Action) const
{
    EPlayerRole Role = GetPlayerRole(Player);
    if (Action == "SpawnObjects") return Role != EPlayerRole::Spectator;
    if (Action == "KickPlayers")  return Role == EPlayerRole::Host;
    if (Action == "GMTools")      return Role == EPlayerRole::Host;
    return true;
}
