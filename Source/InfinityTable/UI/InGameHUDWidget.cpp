#include "UI/InGameHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UInGameHUDWidget::ShowNotification(const FString& Text, float Duration)
{
    OnNotificationShown(Text);

    if (NotificationStack)
    {
        UTextBlock* NotifText = NewObject<UTextBlock>(this);
        NotifText->SetText(FText::FromString(Text));
        NotificationStack->AddChildToVerticalBox(NotifText);

        if (UWorld* World = GetWorld())
        {
            FTimerHandle Handle;
            TWeakObjectPtr<UTextBlock> WeakText = NotifText;
            World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([this, WeakText]()
            {
                if (WeakText.IsValid()) RemoveNotification(WeakText.Get());
            }), Duration, false);
        }
    }
}

void UInGameHUDWidget::SetPlayerCount(int32 Count)
{
    if (PlayerCountText)
        PlayerCountText->SetText(FText::FromString(FString::Printf(TEXT("%d Players"), Count)));
}

void UInGameHUDWidget::SetEnvironmentLabel(const FString& EnvName)
{
    if (EnvironmentText) EnvironmentText->SetText(FText::FromString(EnvName));
}

void UInGameHUDWidget::SetRoleLabel(const FString& RoleName)
{
    if (RoleText) RoleText->SetText(FText::FromString(RoleName));
}

void UInGameHUDWidget::RemoveNotification(UWidget* NotifWidget)
{
    if (NotificationStack && NotifWidget)
        NotificationStack->RemoveChild(NotifWidget);
}
