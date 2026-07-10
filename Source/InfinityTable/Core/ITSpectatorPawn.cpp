#include "ITSpectatorPawn.h"
#include "GameFramework/FloatingPawnMovement.h"

AITSpectatorPawn::AITSpectatorPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    if (UFloatingPawnMovement* Move = Cast<UFloatingPawnMovement>(GetMovementComponent()))
    {
        Move->MaxSpeed = PanSpeed;
    }
}

void AITSpectatorPawn::SetupPlayerInputComponent(UInputComponent* IC)
{
    Super::SetupPlayerInputComponent(IC);
    IC->BindAxis("MoveForward", this, &AITSpectatorPawn::MoveForward);
    IC->BindAxis("MoveRight",   this, &AITSpectatorPawn::MoveRight);
    IC->BindAxis("ZoomTable",   this, &AITSpectatorPawn::Zoom);
}

void AITSpectatorPawn::MoveForward(float Val)
{
    AddMovementInput(GetActorForwardVector(), Val);
}

void AITSpectatorPawn::MoveRight(float Val)
{
    AddMovementInput(GetActorRightVector(), Val);
}

void AITSpectatorPawn::Zoom(float Val)
{
    FVector Loc = GetActorLocation();
    float NewZ = FMath::Clamp(Loc.Z - Val * 10.f, MinZoom, MaxZoom);
    SetActorLocation(FVector(Loc.X, Loc.Y, NewZ));
}
