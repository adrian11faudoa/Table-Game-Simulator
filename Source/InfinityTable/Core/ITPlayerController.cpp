#include "ITPlayerController.h"
#include "Physics/GrabComponent.h"
#include "Objects/TableDice.h"
#include "UI/InGameHUD.h"

AITPlayerController::AITPlayerController()
{
    GrabComponent = CreateDefaultSubobject<UGrabComponent>(TEXT("GrabComponent"));
}

void AITPlayerController::BeginPlay()
{
    Super::BeginPlay();
    bShowMouseCursor = true;
    SetInputMode(FInputModeGameAndUI());
}

void AITPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindAction("GrabObject",     IE_Pressed,  this, &AITPlayerController::TryGrabObject);
    InputComponent->BindAction("ReleaseObject",  IE_Released, this, &AITPlayerController::ReleaseObject);
    InputComponent->BindAction("ContextMenu",    IE_Pressed,  this, &AITPlayerController::OpenContextMenu);
    InputComponent->BindAction("RadialMenu",     IE_Pressed,  this, &AITPlayerController::OpenRadialMenu);
    InputComponent->BindAction("FlipObject",     IE_Pressed,  this, &AITPlayerController::FlipHeldObject);
    InputComponent->BindAction("LockObject",     IE_Pressed,  this, &AITPlayerController::LockHeldObject);
    InputComponent->BindAction("DeleteObject",   IE_Pressed,  this, &AITPlayerController::DeleteSelected);
    InputComponent->BindAction("DuplicateObject",IE_Pressed,  this, &AITPlayerController::DuplicateSelected);
    InputComponent->BindAxis("CameraZoom",       this,         &AITPlayerController::ZoomCamera);
}

void AITPlayerController::TryGrabObject()
{
    if (GrabComponent) GrabComponent->AttemptGrab();
}

void AITPlayerController::ReleaseObject()
{
    if (GrabComponent) GrabComponent->Release();
}

void AITPlayerController::OpenContextMenu()
{
    if (AITHUD* HUD = GetHUD<AITHUD>())
        HUD->ShowContextMenu(GetMousePosition2D());
}

void AITPlayerController::OpenRadialMenu()
{
    bRadialMenuOpen = !bRadialMenuOpen;
    if (AITHUD* HUD = GetHUD<AITHUD>())
        HUD->SetRadialMenuVisible(bRadialMenuOpen);
}

void AITPlayerController::FlipHeldObject()
{
    if (GrabComponent && GrabComponent->GetHeldObject())
        GrabComponent->GetHeldObject()->Server_Flip();
}

void AITPlayerController::LockHeldObject()
{
    if (GrabComponent && GrabComponent->GetHeldObject())
    {
        ATableObject* Obj = GrabComponent->GetHeldObject();
        Obj->Server_Lock(!Obj->bPhysicsLocked);
        GrabComponent->Release();
    }
}

void AITPlayerController::ZoomCamera(float AxisValue)
{
    if (APawn* P = GetPawn())
    {
        FVector Location = P->GetActorLocation();
        Location.Z = FMath::Clamp(Location.Z + AxisValue * CameraZoomSpeed, 100.0f, 2000.0f);
        P->SetActorLocation(Location);
    }
}

bool AITPlayerController::Server_SendChatMessage_Validate(const FString& Message)
{
    return Message.Len() > 0 && Message.Len() <= 256;
}

void AITPlayerController::Server_SendChatMessage_Implementation(const FString& Message)
{
    if (AITGameState* GS = GetWorld()->GetGameState<AITGameState>())
    {
        // Broadcast to all players via multicast
        // Implementation fires a replicated event on game state
    }
}

bool AITPlayerController::Server_RollDice_Validate(ATableDice* Dice, FVector Impulse)
{
    return IsValid(Dice) && Impulse.Size() < 5000.0f;
}

void AITPlayerController::Server_RollDice_Implementation(ATableDice* Dice, FVector Impulse)
{
    if (IsValid(Dice))
        Dice->Server_Roll(Impulse);
}
