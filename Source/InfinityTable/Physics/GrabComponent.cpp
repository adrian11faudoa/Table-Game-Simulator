#include "GrabComponent.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"

UGrabComponent::UGrabComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UGrabComponent::AttemptGrab()
{
    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC) return;

    FVector WorldLoc, WorldDir;
    PC->DeprojectMousePositionToWorld(WorldLoc, WorldDir);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit, WorldLoc, WorldLoc + WorldDir * GrabReachCM,
        ECC_PhysicsBody, Params);

    if (bHit)
    {
        if (ATableObject* Obj = Cast<ATableObject>(Hit.GetActor()))
        {
            HeldObject         = Obj;
            GrabOffset         = FVector::ZeroVector;
            HoldDistance       = Hit.Distance;
            LastCursorWorldPos = WorldLoc + WorldDir * HoldDistance;
            PrevCursorWorldPos = LastCursorWorldPos;
            VelocityHistory.Empty();

            Obj->Server_PickUp(PC);
        }
    }
}

void UGrabComponent::Release()
{
    if (!HeldObject) return;

    // Average velocity over history for smooth throw
    FVector AvgVelocity = FVector::ZeroVector;
    if (VelocityHistory.Num() > 0)
    {
        for (const FVector& V : VelocityHistory)
            AvgVelocity += V;
        AvgVelocity /= VelocityHistory.Num();
    }

    FVector ThrowVelocity = AvgVelocity * ThrowMultiplier;
    ThrowVelocity.Z = FMath::Max(ThrowVelocity.Z, 0.0f); // No downward throws

    HeldObject->Server_Release(ThrowVelocity);
    HeldObject = nullptr;
    VelocityHistory.Empty();
}

void UGrabComponent::RotateHeld(float Yaw, float Pitch)
{
    if (!HeldObject) return;
    FRotator R = HeldObject->GetActorRotation();
    R.Yaw   += Yaw   * RotateSensitivity;
    R.Pitch += Pitch  * RotateSensitivity;
    HeldObject->Server_SetTransform(HeldObject->GetActorLocation(), R);
}

void UGrabComponent::ScaleHeld(float Delta)
{
    if (!HeldObject) return;
    FVector S = HeldObject->GetActorScale3D();
    S = (S + Delta * 0.1f).ComponentMax(FVector(0.1f));
    S = S.ComponentMin(FVector(5.0f));
    HeldObject->Server_Scale(S);
}

void UGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!HeldObject) return;

    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC) return;

    FVector WorldLoc, WorldDir;
    PC->DeprojectMousePositionToWorld(WorldLoc, WorldDir);

    PrevCursorWorldPos = LastCursorWorldPos;
    LastCursorWorldPos = WorldLoc + WorldDir * HoldDistance;

    // Track velocity
    FVector FrameVelocity = (LastCursorWorldPos - PrevCursorWorldPos) / DeltaTime;
    VelocityHistory.Add(FrameVelocity);
    if (VelocityHistory.Num() > VelocityHistorySize)
        VelocityHistory.RemoveAt(0);

    // Smooth interpolation toward target
    FVector TargetPos = LastCursorWorldPos + GrabOffset;
    FVector CurrentPos = HeldObject->GetActorLocation();
    FVector NewPos = FMath::VInterpTo(CurrentPos, TargetPos, DeltaTime, GrabStiffness);

    // Clamp to table bounds
    NewPos.X = FMath::Clamp(NewPos.X, -800.0f, 800.0f);
    NewPos.Y = FMath::Clamp(NewPos.Y, -500.0f, 500.0f);
    NewPos.Z = FMath::Clamp(NewPos.Z, 0.0f, 400.0f);

    HeldObject->Server_SetTransform(NewPos, HeldObject->GetActorRotation());
}
