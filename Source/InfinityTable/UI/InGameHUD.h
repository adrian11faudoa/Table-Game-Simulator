#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "InGameHUD.generated.h"

class UInGameHUDWidget;
class UChatWidget;
class UObjectBrowserWidget;
class URadialMenuWidget;
class ULobbyWidget;
class USavedGamesWidget;
class USettingsWidget;
class UScriptingConsoleWidget;
class UWorkshopWidget;
class UPlayerListWidget;

UCLASS()
class INFINITYTABLE_API AITInGameHUD : public AHUD
{
    GENERATED_BODY()
public:
    AITInGameHUD();
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable) void ShowRadialMenu(bool bShow);
    UFUNCTION(BlueprintCallable) void OpenRadialMenuForObject(class ATableObject* Target);
    UFUNCTION(BlueprintCallable) void ShowObjectBrowser(bool bShow);
    UFUNCTION(BlueprintCallable) void ShowChat(bool bShow);
    UFUNCTION(BlueprintCallable) void AddChatMessage(const FString& PlayerName, const FString& Message);
    UFUNCTION(BlueprintCallable) void ShowNotification(const FString& Text, float Duration = 3.f);

    // The remaining six required panels (see prompt's UI/UX panel list,
    // plus Player List which exists to give the SECURITY/moderation and
    // MULTIPLAYER NETWORKING "player permissions" requirements an actual
    // clickable surface): Lobby, Saved Games, Settings, Scripting Console,
    // Workshop, Player List. Each follows the same toggle pattern as the
    // panels above — single instance created on BeginPlay, shown/hidden
    // rather than re-created per toggle.
    UFUNCTION(BlueprintCallable) void ShowLobby(bool bShow);
    UFUNCTION(BlueprintCallable) void ShowSavedGames(bool bShow);
    UFUNCTION(BlueprintCallable) void ShowSettings(bool bShow);
    UFUNCTION(BlueprintCallable) void ShowScriptingConsole(bool bShow);
    UFUNCTION(BlueprintCallable) void ToggleScriptingConsole();
    UFUNCTION(BlueprintCallable) void ShowWorkshop(bool bShow);
    UFUNCTION(BlueprintCallable) void ShowPlayerList(bool bShow);

    UPROPERTY(EditDefaultsOnly) TSubclassOf<UInGameHUDWidget>      HUDWidgetClass;
    UPROPERTY(EditDefaultsOnly) TSubclassOf<UChatWidget>           ChatWidgetClass;
    UPROPERTY(EditDefaultsOnly) TSubclassOf<UObjectBrowserWidget>  ObjectBrowserClass;
    UPROPERTY(EditDefaultsOnly) TSubclassOf<URadialMenuWidget>     RadialMenuClass;
    UPROPERTY(EditDefaultsOnly) TSubclassOf<ULobbyWidget>          LobbyWidgetClass;
    UPROPERTY(EditDefaultsOnly) TSubclassOf<USavedGamesWidget>     SavedGamesWidgetClass;
    UPROPERTY(EditDefaultsOnly) TSubclassOf<USettingsWidget>       SettingsWidgetClass;
    UPROPERTY(EditDefaultsOnly) TSubclassOf<UScriptingConsoleWidget> ScriptingConsoleWidgetClass;
    UPROPERTY(EditDefaultsOnly) TSubclassOf<UWorkshopWidget>       WorkshopWidgetClass;
    UPROPERTY(EditDefaultsOnly) TSubclassOf<UPlayerListWidget>     PlayerListWidgetClass;

private:
    UPROPERTY() UInGameHUDWidget*    HUDWidget        = nullptr;
    UPROPERTY() UChatWidget*         ChatWidget_       = nullptr;
    UPROPERTY() UObjectBrowserWidget* ObjectBrowser   = nullptr;
    UPROPERTY() URadialMenuWidget*   RadialMenu_      = nullptr;
    UPROPERTY() ULobbyWidget*        LobbyWidget_     = nullptr;
    UPROPERTY() USavedGamesWidget*   SavedGamesWidget_ = nullptr;
    UPROPERTY() USettingsWidget*     SettingsWidget_  = nullptr;
    UPROPERTY() UScriptingConsoleWidget* ScriptingConsoleWidget_ = nullptr;
    UPROPERTY() UWorkshopWidget*     WorkshopWidget_  = nullptr;
    UPROPERTY() UPlayerListWidget*   PlayerListWidget_ = nullptr;
};
