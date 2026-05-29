#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ITGameMode.h"
#include "ITGameState.generated.h"

USTRUCT(BlueprintType)
struct FPlayerInfo
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FString   Name;
    UPROPERTY(BlueprintReadOnly) FLinearColor Color;
    UPROPERTY(BlueprintReadOnly) EPlayerRole  Role;
    UPROPERTY(BlueprintReadOnly) int32     Ping;
};

UCLASS()
class INFINITYTABLE_API AITGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "InfinityTable")
    FString CurrentEnvironmentID = TEXT("FantasyTavern");

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "InfinityTable")
    TArray<FPlayerInfo> ConnectedPlayers;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "InfinityTable")
    FString ActiveGameModID;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "InfinityTable")
    bool bRPGModeEnabled = false;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "InfinityTable")
    bool bFogOfWarEnabled = false;

    UFUNCTION(BlueprintCallable, Category = "InfinityTable")
    void SetEnvironment(const FString& EnvID);

    UFUNCTION(BlueprintCallable, Category = "InfinityTable")
    void SetRPGMode(bool bEnabled);
};
