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

    UFUNCTION(BlueprintCallable) void TryGrabObject();
    UFUNCTION(BlueprintCallable) void ReleaseObject();
    UFUNCTION(BlueprintCallable) void OpenContextMenu();
    UFUNCTION(BlueprintCallable) void FlipHeldObject();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SendChatMessage(const FString& Message);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UGrabComponent* GrabComponent;

    UPROPERTY(EditDefaultsOnly) float GrabReach = 500.0f;
    UPROPERTY(EditDefaultsOnly) float CameraZoomSpeed = 200.0f;
};
