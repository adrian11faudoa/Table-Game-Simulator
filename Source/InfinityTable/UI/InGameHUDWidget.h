#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InGameHUDWidget.generated.h"

class UTextBlock;
class UVerticalBox;

UCLASS()
class INFINITYTABLE_API UInGameHUDWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) void ShowNotification(const FString& Text, float Duration = 3.f);
    UFUNCTION(BlueprintCallable) void SetPlayerCount(int32 Count);
    UFUNCTION(BlueprintCallable) void SetEnvironmentLabel(const FString& EnvName);
    UFUNCTION(BlueprintCallable) void SetRoleLabel(const FString& RoleName);

    UFUNCTION(BlueprintImplementableEvent) void OnNotificationShown(const FString& Text);

protected:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UTextBlock* PlayerCountText;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UTextBlock* EnvironmentText;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UTextBlock* RoleText;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UVerticalBox* NotificationStack;

private:
    UFUNCTION() void RemoveNotification(class UWidget* NotifWidget);
};
