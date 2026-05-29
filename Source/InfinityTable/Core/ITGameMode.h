#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ITGameMode.generated.h"

UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
    Host        UMETA(DisplayName = "Host"),
    CoHost      UMETA(DisplayName = "Co-Host"),
    Player      UMETA(DisplayName = "Player"),
    Spectator   UMETA(DisplayName = "Spectator")
};

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

    UFUNCTION(BlueprintCallable, Category = "InfinityTable")
    EPlayerRole GetPlayerRole(APlayerController* Player) const;

    UFUNCTION(BlueprintCallable, Category = "InfinityTable")
    bool HasPermission(APlayerController* Player, const FString& Action) const;

    UPROPERTY(EditDefaultsOnly, Category = "InfinityTable")
    int32 MaxPlayers = 6;

    UPROPERTY(EditDefaultsOnly, Category = "InfinityTable")
    bool bRequirePassword = false;

    UPROPERTY(EditDefaultsOnly, Category = "InfinityTable")
    FString RoomPassword;

private:
    UPROPERTY()
    TMap<APlayerController*, EPlayerRole> PlayerRoles;

    static TMap<FString, TArray<EPlayerRole>> PermissionMatrix;
    static void InitPermissionMatrix();
};
