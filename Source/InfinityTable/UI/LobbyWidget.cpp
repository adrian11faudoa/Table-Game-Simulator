#include "UI/LobbyWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Components/SpinBox.h"

void ULobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UGameInstance* GI = GetGameInstance())
    {
        Sessions = GI->GetSubsystem<UITSessionSubsystem>();
    }

    if (Sessions)
    {
        Sessions->OnSessionCreated.AddDynamic(this, &ULobbyWidget::HandleSessionCreated);
        Sessions->OnSessionsFound.AddDynamic(this, &ULobbyWidget::HandleSessionsFound);
        Sessions->OnSessionJoined.AddDynamic(this, &ULobbyWidget::HandleSessionJoined);
    }

    if (HostButton)    HostButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnHostClicked);
    if (RefreshButton) RefreshButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnRefreshClicked);

    RefreshSessionList();
}

void ULobbyWidget::NativeDestruct()
{
    if (Sessions)
    {
        Sessions->OnSessionCreated.RemoveDynamic(this, &ULobbyWidget::HandleSessionCreated);
        Sessions->OnSessionsFound.RemoveDynamic(this, &ULobbyWidget::HandleSessionsFound);
        Sessions->OnSessionJoined.RemoveDynamic(this, &ULobbyWidget::HandleSessionJoined);
    }
    Super::NativeDestruct();
}

void ULobbyWidget::HostGame()
{
    if (!Sessions) return;

    FITSessionConfig Config;
    Config.GameName      = GameNameInput ? GameNameInput->GetText().ToString() : TEXT("Untitled Table");
    Config.MaxPlayers    = MaxPlayersInput ? FMath::RoundToInt(MaxPlayersInput->GetValue()) : 6;
    Config.bLAN          = LANCheckbox ? LANCheckbox->IsChecked() : false;
    Config.bPrivate      = PrivateCheckbox ? PrivateCheckbox->IsChecked() : false;

    if (Config.GameName.IsEmpty())
        Config.GameName = TEXT("Untitled Table");

    if (StatusText) StatusText->SetText(FText::FromString(TEXT("Hosting...")));
    Sessions->HostSession(Config);
}

void ULobbyWidget::RefreshSessionList()
{
    if (Sessions) Sessions->FindSessions(50);
}

void ULobbyWidget::JoinSelectedSession(int32 ResultIndex)
{
    if (Sessions)
    {
        if (StatusText) StatusText->SetText(FText::FromString(TEXT("Joining...")));
        Sessions->JoinSession(ResultIndex);
    }
}

void ULobbyWidget::OnHostClicked()    { HostGame(); }
void ULobbyWidget::OnRefreshClicked() { RefreshSessionList(); }

void ULobbyWidget::HandleSessionCreated(bool bSuccess)
{
    FString Msg = bSuccess ? TEXT("Session hosted successfully.") : TEXT("Failed to host session.");
    if (StatusText) StatusText->SetText(FText::FromString(Msg));
    OnHostStatusChanged(bSuccess, Msg);
}

void ULobbyWidget::HandleSessionsFound(const TArray<FITSessionResult>& Results)
{
    if (StatusText)
        StatusText->SetText(FText::FromString(FString::Printf(TEXT("%d sessions found."), Results.Num())));
    OnSessionListUpdated(Results);
}

void ULobbyWidget::HandleSessionJoined(bool bSuccess)
{
    FString Msg = bSuccess ? TEXT("Joined session.") : TEXT("Failed to join session.");
    if (StatusText) StatusText->SetText(FText::FromString(Msg));
    OnJoinStatusChanged(bSuccess, Msg);
}
