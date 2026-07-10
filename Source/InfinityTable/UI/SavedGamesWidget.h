#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Save/SaveGameSubsystem.h"
#include "SavedGamesWidget.generated.h"

class UEditableTextBox;
class UButton;
class UListView;
class UTextBlock;

UCLASS()
class INFINITYTABLE_API USavedGamesWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable) void RefreshSaveList();
    UFUNCTION(BlueprintCallable) void SaveToNewSlot();
    UFUNCTION(BlueprintCallable) void LoadSlot(const FString& Slot);
    UFUNCTION(BlueprintCallable) void DeleteSlot(const FString& Slot);
    UFUNCTION(BlueprintCallable) void OverwriteSlot(const FString& Slot);

    UFUNCTION(BlueprintImplementableEvent) void OnSaveListUpdated(const TArray<FSaveSlotInfo>& Slots);
    UFUNCTION(BlueprintImplementableEvent) void OnOperationResult(bool bSuccess, const FString& Message);

protected:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UEditableTextBox* SaveNameInput;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UButton*          SaveButton;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UButton*          RefreshButton;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UTextBlock*       StatusText;

private:
    UFUNCTION() void OnSaveClicked();
    UFUNCTION() void OnRefreshClicked();
    UPROPERTY() USaveGameSubsystem* SaveSystem = nullptr;
    FString GenerateSlotID() const;
};
