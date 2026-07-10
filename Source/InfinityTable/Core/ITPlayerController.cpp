#include "ITPlayerController.h"
#include "Physics/GrabComponent.h"
#include "UI/InGameHUD.h"
#include "Objects/TableDice.h"
#include "GameFramework/PlayerState.h"

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
    InputComponent->BindAction("ContextMenu", IE_Pressed,  this, &AITPlayerController::OpenContextMenu);
    InputComponent->BindAction("LockObject",  IE_Pressed,  this, &AITPlayerController::LockHeldObject);
    InputComponent->BindAction("DeleteObject",IE_Pressed,  this, &AITPlayerController::DeleteHeldObject);
    InputComponent->BindAction("ToggleChat",  IE_Pressed,  this, &AITPlayerController::ToggleChatFocus);
    InputComponent->BindAction("RollDice",    IE_Pressed,  this, &AITPlayerController::RollHeldDice);
    InputComponent->BindAction("ToggleConsole",  IE_Pressed, this, &AITPlayerController::ToggleScriptingConsole);
    InputComponent->BindAction("ToggleSettings", IE_Pressed, this, &AITPlayerController::ToggleSettingsPanel);
    InputComponent->BindAction("ToggleWorkshop", IE_Pressed, this, &AITPlayerController::ToggleWorkshopPanel);
    InputComponent->BindAction("TogglePlayerList", IE_Pressed, this, &AITPlayerController::TogglePlayerListPanel);
}

AITInGameHUD* AITPlayerController::GetITHUD()
{
    if (!CachedHUD) CachedHUD = Cast<AITInGameHUD>(GetHUD());
    return CachedHUD;
}

void AITPlayerController::TryGrabObject()   { if (GrabComponent) GrabComponent->AttemptGrab(); }
void AITPlayerController::ReleaseObject()   { if (GrabComponent) GrabComponent->Release(); }

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
    }
}

void AITPlayerController::DeleteHeldObject()
{
    if (GrabComponent && GrabComponent->GetHeldObject())
    {
        GrabComponent->Release();
        GrabComponent->GetHeldObject()->Destroy();
    }
}

void AITPlayerController::RollHeldDice()
{
    if (GrabComponent && GrabComponent->GetHeldObject())
    {
        if (ATableDice* Dice = Cast<ATableDice>(GrabComponent->GetHeldObject()))
        {
            FVector Impulse(FMath::RandRange(-50.f, 50.f), FMath::RandRange(-50.f, 50.f), 400.f);
            Dice->Server_Roll(Impulse);
        }
    }
}

void AITPlayerController::OpenContextMenu()
{
    if (AITInGameHUD* HUD = GetITHUD())
    {
        if (GrabComponent && GrabComponent->GetHeldObject())
        {
            HUD->OpenRadialMenuForObject(GrabComponent->GetHeldObject());
        }
        else
        {
            HUD->ShowRadialMenu(true);
        }
    }
}

void AITPlayerController::ToggleChatFocus()
{
    if (AITInGameHUD* HUD = GetITHUD())
    {
        HUD->ShowChat(true);
    }
}

void AITPlayerController::ToggleScriptingConsole()
{
    if (AITInGameHUD* HUD = GetITHUD())
    {
        HUD->ToggleScriptingConsole();
    }
}

void AITPlayerController::ToggleSettingsPanel()
{
    if (AITInGameHUD* HUD = GetITHUD())
    {
        HUD->ShowSettings(true);
    }
}

void AITPlayerController::ToggleWorkshopPanel()
{
    if (AITInGameHUD* HUD = GetITHUD())
    {
        HUD->ShowWorkshop(true);
    }
}

void AITPlayerController::TogglePlayerListPanel()
{
    if (AITInGameHUD* HUD = GetITHUD())
    {
        HUD->ShowPlayerList(true);
    }
}

bool AITPlayerController::Server_SendChatMessage_Validate(const FString& Msg)
{
    return Msg.Len() > 0 && Msg.Len() <= 256;
}

void AITPlayerController::Server_SendChatMessage_Implementation(const FString& Msg)
{
    FString SenderName = PlayerState ? PlayerState->GetPlayerName() : TEXT("Player");

    // Broadcast to every connected player controller
    if (UWorld* World = GetWorld())
    {
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            if (AITPlayerController* PC = Cast<AITPlayerController>(It->Get()))
            {
                PC->Client_ReceiveChatMessage(SenderName, Msg);
            }
        }
    }
}

void AITPlayerController::Client_ReceiveChatMessage_Implementation(const FString& SenderName, const FString& Message)
{
    if (AITInGameHUD* HUD = GetITHUD())
    {
        HUD->AddChatMessage(SenderName, Message);
    }
}
