#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ITGameMode.h"
#include "ITPlayerState.generated.h"

UCLASS()
class INFINITYTABLE_API AITPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable) void SetRole(EPlayerRole NewRole);
    UFUNCTION(BlueprintCallable) EPlayerRole GetRole() const { return Role; }

    UFUNCTION(BlueprintCallable) void SetPlayerColor(FLinearColor NewColor);
    UFUNCTION(BlueprintCallable) FLinearColor GetPlayerColor() const { return PlayerColor; }

    UPROPERTY(Replicated, BlueprintReadOnly) EPlayerRole  Role = EPlayerRole::Player;
    UPROPERTY(Replicated, BlueprintReadOnly) FLinearColor PlayerColor = FLinearColor::White;
    UPROPERTY(Replicated, BlueprintReadOnly) int32        Score = 0;
    UPROPERTY(Replicated, BlueprintReadOnly) bool         bReady = false;

private:
    static TArray<FLinearColor> DefaultColors;
};
