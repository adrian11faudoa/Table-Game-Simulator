#include "UI/ChatWidget.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Core/ITPlayerController.h"
#include "Kismet/GameplayStatics.h"

void UChatWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (SendButton)
        SendButton->OnClicked.AddDynamic(this, &UChatWidget::OnSendClicked);
    if (ChatInput)
        ChatInput->OnTextCommitted.AddDynamic(this, &UChatWidget::OnInputCommitted);
}

void UChatWidget::AddMessage(const FString& PlayerName, const FString& Message)
{
    if (!MessageScrollBox) return;

    UTextBlock* Line = NewObject<UTextBlock>(this);
    Line->SetText(FText::FromString(FString::Printf(TEXT("[%s]: %s"), *PlayerName, *Message)));
    MessageScrollBox->AddChild(Line);

    if (++MessageCount > MaxMessages && MessageScrollBox->GetChildrenCount() > 0)
    {
        MessageScrollBox->RemoveChildAt(0);
        --MessageCount;
    }
    MessageScrollBox->ScrollToEnd();
}

void UChatWidget::AddSystemMessage(const FString& Message)
{
    if (!MessageScrollBox) return;

    UTextBlock* Line = NewObject<UTextBlock>(this);
    Line->SetText(FText::FromString(Message));
    Line->SetColorAndOpacity(FSlateColor(SystemMessageColor));
    MessageScrollBox->AddChild(Line);
    MessageScrollBox->ScrollToEnd();
}

void UChatWidget::ClearChat()
{
    if (MessageScrollBox) MessageScrollBox->ClearChildren();
    MessageCount = 0;
}

void UChatWidget::OnSendClicked()
{
    SubmitChatText();
}

void UChatWidget::OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter)
        SubmitChatText();
}

void UChatWidget::SubmitChatText()
{
    if (!ChatInput) return;
    FString Msg = ChatInput->GetText().ToString();
    if (Msg.IsEmpty()) return;

    if (AITPlayerController* PC = Cast<AITPlayerController>(GetOwningPlayer()))
    {
        PC->Server_SendChatMessage(Msg);
    }
    ChatInput->SetText(FText::GetEmpty());
}
