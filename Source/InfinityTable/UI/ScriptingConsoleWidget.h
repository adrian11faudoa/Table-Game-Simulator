#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScriptingConsoleWidget.generated.h"

class UScrollBox;
class UEditableTextBox;
class UButton;

UCLASS()
class INFINITYTABLE_API UScriptingConsoleWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable) void ExecuteCurrentInput();
    UFUNCTION(BlueprintCallable) void ExecuteCommand(const FString& Command);
    UFUNCTION(BlueprintCallable) void ClearConsole();
    UFUNCTION(BlueprintCallable) void HistoryUp();
    UFUNCTION(BlueprintCallable) void HistoryDown();

    UPROPERTY(EditDefaultsOnly) int32 MaxHistoryEntries = 50;
    UPROPERTY(EditDefaultsOnly) bool  bRequireHostPermission = true;

protected:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UScrollBox*       OutputScrollBox;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UEditableTextBox* CommandInput;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UButton*         ExecuteButton;

private:
    UFUNCTION() void OnExecuteClicked();
    UFUNCTION() void OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);
    void PrintLine(const FString& Line, bool bIsError = false);
    bool HasConsolePermission() const;

    TArray<FString> CommandHistory;
    int32 HistoryCursor = -1;
};
