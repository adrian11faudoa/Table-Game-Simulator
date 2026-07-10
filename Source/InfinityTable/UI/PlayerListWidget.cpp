#include "UI/PlayerListWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Core/ITPlayerController.h"
#include "Core/ITPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

void UPlayerListWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (RefreshButton) RefreshButton->OnClicked.AddDynamic(this, &UPlayerListWidget::OnRefreshClicked);

    RefreshPlayerList();
}

bool UPlayerListWidget::HasModerationPermission() const
{
    UWorld* World = GetWorld();
    if (!World) return false;

    AITGameMode* GM = World->GetAuthGameMode<AITGameMode>();
    if (!GM) return false; // clients never have authority over the GameMode

    return GM->HasPermission(GetOwningPlayer(), TEXT("KickPlayers"));
}

void UPlayerListWidget::RefreshPlayerList()
{
    UWorld* World = GetWorld();
    if (!World) return;

    AITGameMode* GM = World->GetAuthGameMode<AITGameMode>();

    TArray<FPlayerListEntry> Entries;
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC || !PC->PlayerState) continue;

        FPlayerListEntry Entry;
        Entry.DisplayName    = PC->PlayerState->GetPlayerName();
        Entry.Controller     = PC;
        Entry.bIsLocalPlayer = PC->IsLocalController();

        if (AITPlayerState* ITState = Cast<AITPlayerState>(PC->PlayerState))
        {
            Entry.PlayerColor = ITState->PlayerColor;
        }

        // GetPlayerRole is only meaningful when called with GameMode
        // authority (server); on a client this widget can still list names
        // but role/kick controls should be hidden — see
        // HasModerationPermission(), which already returns false on
        // clients since GetAuthGameMode<>() is null there.
        if (GM)
        {
            Entry.Role = GM->GetPlayerRole(PC);
        }

        Entries.Add(Entry);
    }

    OnPlayerListUpdated(Entries);
}

APlayerController* UPlayerListWidget::FindControllerByName(const FString& DisplayName) const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->PlayerState && PC->PlayerState->GetPlayerName() == DisplayName)
        {
            return PC;
        }
    }
    return nullptr;
}

bool UPlayerListWidget::KickByName(const FString& DisplayName, const FString& Reason)
{
    UWorld* World = GetWorld();
    AITGameMode* GM = World ? World->GetAuthGameMode<AITGameMode>() : nullptr;
    APlayerController* Target = FindControllerByName(DisplayName);

    if (!GM || !Target)
    {
        FString Msg = TEXT("Player not found (or you don't have server authority — moderation only works from the host/server).");
        if (StatusText) StatusText->SetText(FText::FromString(Msg));
        OnModerationResult(false, Msg);
        return false;
    }

    bool bOK = GM->KickPlayer(GetOwningPlayer(), Target, Reason);
    FString Msg = bOK ? FString::Printf(TEXT("Kicked %s."), *DisplayName)
                       : TEXT("Kick failed — you may not have permission.");
    if (StatusText) StatusText->SetText(FText::FromString(Msg));
    OnModerationResult(bOK, Msg);

    if (bOK) RefreshPlayerList();
    return bOK;
}

bool UPlayerListWidget::BanByName(const FString& DisplayName, const FString& Reason)
{
    UWorld* World = GetWorld();
    AITGameMode* GM = World ? World->GetAuthGameMode<AITGameMode>() : nullptr;
    APlayerController* Target = FindControllerByName(DisplayName);

    if (!GM || !Target)
    {
        FString Msg = TEXT("Player not found (or you don't have server authority).");
        if (StatusText) StatusText->SetText(FText::FromString(Msg));
        OnModerationResult(false, Msg);
        return false;
    }

    bool bOK = GM->BanPlayer(GetOwningPlayer(), Target, Reason);
    FString Msg = bOK ? FString::Printf(TEXT("Banned %s."), *DisplayName)
                       : TEXT("Ban failed — you may not have permission.");
    if (StatusText) StatusText->SetText(FText::FromString(Msg));
    OnModerationResult(bOK, Msg);

    if (bOK) RefreshPlayerList();
    return bOK;
}

bool UPlayerListWidget::PromoteByName(const FString& DisplayName, EPlayerRole NewRole)
{
    UWorld* World = GetWorld();
    AITGameMode* GM = World ? World->GetAuthGameMode<AITGameMode>() : nullptr;
    APlayerController* Target = FindControllerByName(DisplayName);

    if (!GM || !Target)
    {
        FString Msg = TEXT("Player not found (or you don't have server authority).");
        if (StatusText) StatusText->SetText(FText::FromString(Msg));
        OnModerationResult(false, Msg);
        return false;
    }

    // SetPlayerPermission has no return value and no internal permission
    // gate of its own (it's a direct role-assignment utility, typically
    // called from gameplay flows the host already controls, e.g. an
    // in-lobby "promote to co-host" button) — so this widget enforces the
    // same "requester must hold KickPlayers" bar as kick/ban, since
    // promoting/demoting other players is at least as sensitive an action.
    if (!HasModerationPermission())
    {
        FString Msg = TEXT("You don't have permission to change player roles.");
        if (StatusText) StatusText->SetText(FText::FromString(Msg));
        OnModerationResult(false, Msg);
        return false;
    }

    GM->SetPlayerPermission(Target, NewRole);
    FString Msg = FString::Printf(TEXT("%s is now %s."), *DisplayName,
        *StaticEnum<EPlayerRole>()->GetDisplayNameTextByValue((int64)NewRole).ToString());
    if (StatusText) StatusText->SetText(FText::FromString(Msg));
    OnModerationResult(true, Msg);

    RefreshPlayerList();
    return true;
}

void UPlayerListWidget::OnRefreshClicked()
{
    RefreshPlayerList();
}
