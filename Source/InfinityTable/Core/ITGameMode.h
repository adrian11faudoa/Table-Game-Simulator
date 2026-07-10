#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/CoreOnline.h"
#include "ITGameMode.generated.h"

UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
    Host UMETA(DisplayName="Host"),
    CoHost UMETA(DisplayName="Co-Host"),
    Player UMETA(DisplayName="Player"),
    Spectator UMETA(DisplayName="Spectator")
};

UCLASS()
class INFINITYTABLE_API AITGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AITGameMode();
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    UFUNCTION(BlueprintCallable) void SetPlayerPermission(APlayerController* Player, EPlayerRole Role);
    UFUNCTION(BlueprintCallable) EPlayerRole GetPlayerRole(APlayerController* Player) const;
    UFUNCTION(BlueprintCallable) bool HasPermission(APlayerController* Player, const FString& Action) const;

    // Multiplayer moderation (SECURITY requirement). Requesting must hold
    // the "KickPlayers" permission (Host only by default — see
    // HasPermission). KickPlayer disconnects immediately; BanPlayer also
    // kicks and additionally records the player's unique net ID so
    // PreLogin rejects any future reconnect attempt from that ID for the
    // remainder of the session (the ban list is in-memory only and does
    // not persist across server restarts — see the note in
    // Docs/Multiplayer_Guide.md if a persistent ban list is needed).
    UFUNCTION(BlueprintCallable) bool KickPlayer(APlayerController* Requester, APlayerController* Target, const FString& Reason);
    UFUNCTION(BlueprintCallable) bool BanPlayer(APlayerController* Requester, APlayerController* Target, const FString& Reason);
    virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

    UPROPERTY(EditDefaultsOnly) int32   MaxPlayers = 6;
    UPROPERTY(EditDefaultsOnly) bool    bRequirePassword = false;
    UPROPERTY(EditDefaultsOnly) FString RoomPassword;
private:
    UPROPERTY() TMap<APlayerController*, EPlayerRole> PlayerRoles;
    TSet<FString> BannedNetIDs;
};
