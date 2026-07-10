#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ITGameMode.h"
#include "ITGameState.generated.h"

UCLASS()
class INFINITYTABLE_API AITGameState : public AGameStateBase
{
    GENERATED_BODY()
public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const override;
    UPROPERTY(Replicated, BlueprintReadOnly) FString CurrentEnvironmentID = TEXT("FantasyTavern");
    UPROPERTY(Replicated, BlueprintReadOnly) bool    bRPGModeEnabled       = false;
    UPROPERTY(Replicated, BlueprintReadOnly) bool    bFogOfWarEnabled      = false;
};
