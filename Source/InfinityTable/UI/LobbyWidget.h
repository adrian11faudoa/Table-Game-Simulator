#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Networking/ITSessionSubsystem.h"
#include "LobbyWidget.generated.h"

class UEditableTextBox;
class UButton;
class UCheckBox;
class UListView;
class UTextBlock;
class USpinBox;

UCLASS()
class INFINITYTABLE_API ULobbyWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintCallable) void HostGame();
    UFUNCTION(BlueprintCallable) void RefreshSessionList();
    UFUNCTION(BlueprintCallable) void JoinSelectedSession(int32 ResultIndex);

    UFUNCTION(BlueprintImplementableEvent) void OnSessionListUpdated(const TArray<FITSessionResult>& Results);
    UFUNCTION(BlueprintImplementableEvent) void OnHostStatusChanged(bool bSuccess, const FString& Message);
    UFUNCTION(BlueprintImplementableEvent) void OnJoinStatusChanged(bool bSuccess, const FString& Message);

protected:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UEditableTextBox* GameNameInput;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) USpinBox*         MaxPlayersInput;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UCheckBox*        LANCheckbox;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UCheckBox*        PrivateCheckbox;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UButton*          HostButton;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UButton*          RefreshButton;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UTextBlock*       StatusText;

private:
    UFUNCTION() void OnHostClicked();
    UFUNCTION() void OnRefreshClicked();
    UFUNCTION() void HandleSessionCreated(bool bSuccess);
    UFUNCTION() void HandleSessionsFound(const TArray<FITSessionResult>& Results);
    UFUNCTION() void HandleSessionJoined(bool bSuccess);

    UPROPERTY() UITSessionSubsystem* Sessions = nullptr;
};
