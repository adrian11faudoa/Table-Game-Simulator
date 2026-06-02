#include "ITPlayerController.h"
#include "Physics/GrabComponent.h"

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
    InputComponent->BindAction("GrabObject",  IE_Pressed,  this, &AITPlayerController::TryGrabObject);
    InputComponent->BindAction("GrabObject",  IE_Released, this, &AITPlayerController::ReleaseObject);
    InputComponent->BindAction("FlipObject",  IE_Pressed,  this, &AITPlayerController::FlipHeldObject);
}

void AITPlayerController::TryGrabObject()   { if (GrabComponent) GrabComponent->AttemptGrab(); }
void AITPlayerController::ReleaseObject()   { if (GrabComponent) GrabComponent->Release(); }
void AITPlayerController::OpenContextMenu() {}
void AITPlayerController::FlipHeldObject()
{
    if (GrabComponent && GrabComponent->GetHeldObject())
        GrabComponent->GetHeldObject()->Server_Flip();
}

bool AITPlayerController::Server_SendChatMessage_Validate(const FString& Msg)
{ return Msg.Len() > 0 && Msg.Len() <= 256; }
void AITPlayerController::Server_SendChatMessage_Implementation(const FString& Msg) {}
