#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "ITSpectatorPawn.generated.h"

UCLASS()
class INFINITYTABLE_API AITSpectatorPawn : public ASpectatorPawn
{
    GENERATED_BODY()
public:
    AITSpectatorPawn();
    virtual void SetupPlayerInputComponent(UInputComponent* IC) override;

    UPROPERTY(EditDefaultsOnly) float MinZoom  = 200.0f;
    UPROPERTY(EditDefaultsOnly) float MaxZoom  = 1800.0f;
    UPROPERTY(EditDefaultsOnly) float PanSpeed = 600.0f;

private:
    void MoveForward(float Val);
    void MoveRight(float Val);
    void Zoom(float Val);
};
