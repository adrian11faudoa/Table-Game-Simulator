#include "ITGameMode.h"
#include "ITGameState.h"
#include "ITPlayerController.h"
#include "ITPlayerState.h"
#include "ITSpectatorPawn.h"
#include "Save/SaveGameSubsystem.h"
#include "Scripting/LuaSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "OnlineSubsystemUtils.h"

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

bool AITGameMode::KickPlayer(APlayerController* Requester, APlayerController* Target, const FString& Reason)
{
    if (!Target || !HasPermission(Requester, TEXT("KickPlayers"))) return false;
    if (Target == Requester) return false; // hosts can't kick themselves via this path — use logout/leave instead

    UE_LOG(LogTemp, Log, TEXT("AITGameMode: %s kicked by %s (%s)"),
        *Target->GetName(),
        Requester ? *Requester->GetName() : TEXT("server"),
        Reason.IsEmpty() ? TEXT("no reason given") : *Reason);

    Target->ClientWasKicked(FText::FromString(Reason.IsEmpty() ? TEXT("Kicked by host") : Reason));
    Target->Destroy();
    return true;
}

bool AITGameMode::BanPlayer(APlayerController* Requester, APlayerController* Target, const FString& Reason)
{
    if (!Target || !HasPermission(Requester, TEXT("KickPlayers"))) return false;

    if (Target->PlayerState && Target->PlayerState->GetUniqueId().IsValid())
    {
        BannedNetIDs.Add(Target->PlayerState->GetUniqueId().ToString());
    }

    return KickPlayer(Requester, Target, Reason.IsEmpty() ? TEXT("Banned by host") : Reason);
}

void AITGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

    if (UniqueId.IsValid() && BannedNetIDs.Contains(UniqueId.ToString()))
    {
        ErrorMessage = TEXT("You have been banned from this session.");
    }
}
