#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Objects/TableObject.h"
#include "GrabComponent.generated.h"

UCLASS(ClassGroup=(InfinityTable), meta=(BlueprintSpawnableComponent))
class INFINITYTABLE_API UGrabComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UGrabComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable) void AttemptGrab();
    UFUNCTION(BlueprintCallable) void Release();
    UFUNCTION(BlueprintCallable) void RotateHeld(float Yaw, float Pitch);
    UFUNCTION(BlueprintCallable) void ScaleHeld(float Delta);

    UFUNCTION(BlueprintCallable)
    ATableObject* GetHeldObject() const { return HeldObject; }

    UFUNCTION(BlueprintCallable)
    bool IsHolding() const { return HeldObject != nullptr; }

    UPROPERTY(EditDefaultsOnly) float GrabReachCM   = 500.0f;
    UPROPERTY(EditDefaultsOnly) float GrabStiffness  = 15.0f;
    UPROPERTY(EditDefaultsOnly) float ThrowMultiplier = 2.5f;
    UPROPERTY(EditDefaultsOnly) float RotateSensitivity = 1.5f;

private:
    UPROPERTY() ATableObject* HeldObject = nullptr;

    FVector GrabOffset         = FVector::ZeroVector;
    float   HoldDistance       = 200.0f;
    FVector LastCursorWorldPos = FVector::ZeroVector;
    FVector PrevCursorWorldPos = FVector::ZeroVector;

    // Velocity history for smooth throw
    TArray<FVector> VelocityHistory;
    static const int32 VelocityHistorySize = 5;
};
