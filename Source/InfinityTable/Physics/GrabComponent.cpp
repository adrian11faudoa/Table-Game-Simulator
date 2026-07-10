#include "GrabComponent.h"
#include "GameFramework/PlayerController.h"

UGrabComponent::UGrabComponent() { PrimaryComponentTick.bCanEverTick = true; }

void UGrabComponent::AttemptGrab()
{
    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC) return;
    FVector Loc, Dir;
    PC->DeprojectMousePositionToWorld(Loc, Dir);
    FHitResult Hit;
    FCollisionQueryParams P; P.AddIgnoredActor(GetOwner());
    if (GetWorld()->LineTraceSingleByChannel(Hit, Loc, Loc + Dir * GrabReachCM, ECC_PhysicsBody, P))
    {
        if (ATableObject* Obj = Cast<ATableObject>(Hit.GetActor()))
        {
            HeldObject = Obj;
            HoldDistance = Hit.Distance;
            LastCursorWorldPos = Loc + Dir * HoldDistance;
            PrevCursorWorldPos = LastCursorWorldPos;
            VelocityHistory.Empty();
            Obj->Server_PickUp(PC);
        }
    }
}

void UGrabComponent::Release()
{
    if (!HeldObject) return;
    FVector Avg = FVector::ZeroVector;
    for (auto& V : VelocityHistory) Avg += V;
    if (VelocityHistory.Num()) Avg /= VelocityHistory.Num();
    HeldObject->Server_Release(Avg * ThrowMultiplier);
    HeldObject = nullptr;
    VelocityHistory.Empty();
}

void UGrabComponent::TickComponent(float DeltaTime, ELevelTick T, FActorComponentTickFunction* F)
{
    Super::TickComponent(DeltaTime, T, F);
    if (!HeldObject) return;
    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC) return;
    FVector Loc, Dir;
    PC->DeprojectMousePositionToWorld(Loc, Dir);
    PrevCursorWorldPos = LastCursorWorldPos;
    LastCursorWorldPos = Loc + Dir * HoldDistance;
    VelocityHistory.Add((LastCursorWorldPos - PrevCursorWorldPos) / DeltaTime);
    if (VelocityHistory.Num() > 5) VelocityHistory.RemoveAt(0);
    FVector Target = LastCursorWorldPos;
    FVector NewPos = FMath::VInterpTo(HeldObject->GetActorLocation(), Target, DeltaTime, GrabStiffness);
    NewPos.X = FMath::Clamp(NewPos.X, -800.f, 800.f);
    NewPos.Y = FMath::Clamp(NewPos.Y, -500.f, 500.f);
    NewPos.Z = FMath::Clamp(NewPos.Z, 0.f,    400.f);
    HeldObject->Server_SetTransform(NewPos, HeldObject->GetActorRotation());
}
