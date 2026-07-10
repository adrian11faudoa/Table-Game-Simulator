#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatWidget.generated.h"

class UScrollBox;
class UEditableTextBox;
class UButton;

UCLASS()
class INFINITYTABLE_API UChatWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable) void AddMessage(const FString& PlayerName, const FString& Message);
    UFUNCTION(BlueprintCallable) void AddSystemMessage(const FString& Message);
    UFUNCTION(BlueprintCallable) void ClearChat();

    UPROPERTY(EditDefaultsOnly) int32 MaxMessages = 100;
    UPROPERTY(EditDefaultsOnly) FLinearColor SystemMessageColor = FLinearColor(0.6f, 0.6f, 0.6f);

protected:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UScrollBox* MessageScrollBox;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UEditableTextBox* ChatInput;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UButton* SendButton;

private:
    UFUNCTION() void OnSendClicked();
    UFUNCTION() void OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);
    void SubmitChatText();
    int32 MessageCount = 0;
};
