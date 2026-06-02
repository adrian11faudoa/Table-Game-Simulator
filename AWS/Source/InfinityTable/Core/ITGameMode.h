#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
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

    UPROPERTY(EditDefaultsOnly) int32   MaxPlayers = 6;
    UPROPERTY(EditDefaultsOnly) bool    bRequirePassword = false;
    UPROPERTY(EditDefaultsOnly) FString RoomPassword;
private:
    UPROPERTY() TMap<APlayerController*, EPlayerRole> PlayerRoles;
};
