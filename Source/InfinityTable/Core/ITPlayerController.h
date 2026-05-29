#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ITPlayerController.generated.h"

UCLASS()
class INFINITYTABLE_API AITPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AITPlayerController();

    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    // Object interaction
    UFUNCTION(BlueprintCallable) void TryGrabObject();
    UFUNCTION(BlueprintCallable) void ReleaseObject();
    UFUNCTION(BlueprintCallable) void OpenContextMenu();
    UFUNCTION(BlueprintCallable) void OpenRadialMenu();
    UFUNCTION(BlueprintCallable) void FlipHeldObject();
    UFUNCTION(BlueprintCallable) void LockHeldObject();
    UFUNCTION(BlueprintCallable) void DeleteSelected();
    UFUNCTION(BlueprintCallable) void DuplicateSelected();

    // Camera
    UFUNCTION(BlueprintCallable) void ZoomCamera(float AxisValue);
    UFUNCTION(BlueprintCallable) void PanCamera(float X, float Y);

    // Server RPCs
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SendChatMessage(const FString& Message);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_RollDice(class ATableDice* Dice, FVector Impulse);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UGrabComponent* GrabComponent;

    UPROPERTY(EditDefaultsOnly)
    float GrabReach = 500.0f;

    UPROPERTY(EditDefaultsOnly)
    float CameraZoomSpeed = 200.0f;

private:
    FVector2D LastMousePos;
    bool bRadialMenuOpen = false;
};
