#include "UI/InGameHUD.h"
#include "UI/InGameHUDWidget.h"
#include "UI/ChatWidget.h"
#include "UI/ObjectBrowserWidget.h"
#include "UI/RadialMenuWidget.h"
#include "UI/LobbyWidget.h"
#include "UI/SavedGamesWidget.h"
#include "UI/SettingsWidget.h"
#include "UI/ScriptingConsoleWidget.h"
#include "UI/WorkshopWidget.h"
#include "UI/PlayerListWidget.h"
#include "Objects/TableObject.h"
#include "Blueprint/UserWidget.h"

AITInGameHUD::AITInGameHUD() {}

void AITInGameHUD::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    auto Create = [&]<typename T>(TSubclassOf<T> Class) -> T*
    {
        if (!Class) return nullptr;
        T* Widget = CreateWidget<T>(PC, Class);
        if (Widget) Widget->AddToViewport();
        return Widget;
    };

    HUDWidget    = Create(HUDWidgetClass);
    ChatWidget_  = Create(ChatWidgetClass);
    ObjectBrowser = Create(ObjectBrowserClass);
    RadialMenu_  = Create(RadialMenuClass);
    LobbyWidget_ = Create(LobbyWidgetClass);
    SavedGamesWidget_ = Create(SavedGamesWidgetClass);
    SettingsWidget_   = Create(SettingsWidgetClass);
    ScriptingConsoleWidget_ = Create(ScriptingConsoleWidgetClass);
    WorkshopWidget_ = Create(WorkshopWidgetClass);
    PlayerListWidget_ = Create(PlayerListWidgetClass);

    // Start with radial, browser, and the secondary panels hidden
    ShowRadialMenu(false);
    ShowObjectBrowser(false);
    ShowLobby(false);
    ShowSavedGames(false);
    ShowSettings(false);
    ShowScriptingConsole(false);
    ShowWorkshop(false);
    ShowPlayerList(false);
}

void AITInGameHUD::ShowRadialMenu(bool bShow)
{
    if (RadialMenu_)
    {
        if (bShow) RadialMenu_->SetVisibility(ESlateVisibility::Visible);
        else       RadialMenu_->Close();
    }
}

void AITInGameHUD::OpenRadialMenuForObject(ATableObject* Target)
{
    if (RadialMenu_ && Target)
        RadialMenu_->OpenForObject(Target);
}

void AITInGameHUD::ShowObjectBrowser(bool bShow)
{
    if (ObjectBrowser) ObjectBrowser->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void AITInGameHUD::ShowChat(bool bShow)
{
    if (ChatWidget_) ChatWidget_->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void AITInGameHUD::AddChatMessage(const FString& PlayerName, const FString& Message)
{
    if (ChatWidget_) ChatWidget_->AddMessage(PlayerName, Message);
}

void AITInGameHUD::ShowNotification(const FString& Text, float Duration)
{
    if (HUDWidget) HUDWidget->ShowNotification(Text, Duration);
}

void AITInGameHUD::ShowLobby(bool bShow)
{
    if (LobbyWidget_)
    {
        LobbyWidget_->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        if (bShow) LobbyWidget_->RefreshSessionList();
    }
}

void AITInGameHUD::ShowSavedGames(bool bShow)
{
    if (SavedGamesWidget_)
    {
        SavedGamesWidget_->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        if (bShow) SavedGamesWidget_->RefreshSaveList();
    }
}

void AITInGameHUD::ShowSettings(bool bShow)
{
    if (SettingsWidget_)
        SettingsWidget_->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void AITInGameHUD::ShowScriptingConsole(bool bShow)
{
    if (ScriptingConsoleWidget_)
        ScriptingConsoleWidget_->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void AITInGameHUD::ToggleScriptingConsole()
{
    if (ScriptingConsoleWidget_)
    {
        bool bCurrentlyVisible = ScriptingConsoleWidget_->GetVisibility() == ESlateVisibility::Visible;
        ShowScriptingConsole(!bCurrentlyVisible);
    }
}

void AITInGameHUD::ShowWorkshop(bool bShow)
{
    if (WorkshopWidget_)
    {
        WorkshopWidget_->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        if (bShow)
        {
            WorkshopWidget_->RefreshModList();
            WorkshopWidget_->RefreshAssetList();
        }
    }
}

void AITInGameHUD::ShowPlayerList(bool bShow)
{
    if (PlayerListWidget_)
    {
        PlayerListWidget_->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        if (bShow) PlayerListWidget_->RefreshPlayerList();
    }
}
