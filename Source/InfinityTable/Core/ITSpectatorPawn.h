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
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditDefaultsOnly) float MinZoom   = 200.0f;
    UPROPERTY(EditDefaultsOnly) float MaxZoom   = 1800.0f;
    UPROPERTY(EditDefaultsOnly) float PanSpeed  = 600.0f;
    UPROPERTY(EditDefaultsOnly) float ZoomSpeed = 200.0f;
    UPROPERTY(EditDefaultsOnly) float RotateSpeed = 90.0f;

    UFUNCTION(BlueprintCallable) void ZoomIn(float Value);
    UFUNCTION(BlueprintCallable) void PanX(float Value);
    UFUNCTION(BlueprintCallable) void PanY(float Value);
    UFUNCTION(BlueprintCallable) void RotateAroundTable(float Value);

private:
    bool bRotating = false;
    FVector TableCenter = FVector::ZeroVector;
};
