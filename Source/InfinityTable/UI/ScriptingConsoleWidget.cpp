#include "UI/ScriptingConsoleWidget.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Scripting/LuaSubsystem.h"
#include "Core/ITPlayerState.h"
#include "Core/ITGameMode.h"

void UScriptingConsoleWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ExecuteButton) ExecuteButton->OnClicked.AddDynamic(this, &UScriptingConsoleWidget::OnExecuteClicked);
    if (CommandInput)  CommandInput->OnTextCommitted.AddDynamic(this, &UScriptingConsoleWidget::OnInputCommitted);

    PrintLine(TEXT("InfinityTable Scripting Console — type Lua, press Enter to run."));
}

bool UScriptingConsoleWidget::HasConsolePermission() const
{
    if (!bRequireHostPermission) return true;

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return false;

    if (AITPlayerState* PS = PC->GetPlayerState<AITPlayerState>())
    {
        EPlayerRole Role = PS->GetRole();
        return Role == EPlayerRole::Host || Role == EPlayerRole::CoHost;
    }
    return false;
}

void UScriptingConsoleWidget::ExecuteCurrentInput()
{
    if (!CommandInput) return;
    FString Command = CommandInput->GetText().ToString();
    if (Command.IsEmpty()) return;

    ExecuteCommand(Command);
    CommandInput->SetText(FText::GetEmpty());
}

void UScriptingConsoleWidget::ExecuteCommand(const FString& Command)
{
    PrintLine(FString::Printf(TEXT("> %s"), *Command));

    if (!HasConsolePermission())
    {
        PrintLine(TEXT("Permission denied: only the Host or Co-Host may run console commands."), true);
        return;
    }

    CommandHistory.Add(Command);
    if (CommandHistory.Num() > MaxHistoryEntries)
        CommandHistory.RemoveAt(0);
    HistoryCursor = CommandHistory.Num();

    if (UGameInstance* GI = GetGameInstance())
    {
        if (ULuaSubsystem* Lua = GI->GetSubsystem<ULuaSubsystem>())
        {
            bool bOK = Lua->RunString(Command);
            if (!bOK)
            {
                PrintLine(TEXT("Error executing command — see server log for details."), true);
            }
            return;
        }
    }
    PrintLine(TEXT("Lua subsystem unavailable."), true);
}

void UScriptingConsoleWidget::ClearConsole()
{
    if (OutputScrollBox) OutputScrollBox->ClearChildren();
}

void UScriptingConsoleWidget::HistoryUp()
{
    if (CommandHistory.Num() == 0 || !CommandInput) return;
    HistoryCursor = FMath::Max(0, HistoryCursor - 1);
    CommandInput->SetText(FText::FromString(CommandHistory[HistoryCursor]));
}

void UScriptingConsoleWidget::HistoryDown()
{
    if (CommandHistory.Num() == 0 || !CommandInput) return;
    HistoryCursor = FMath::Min(CommandHistory.Num(), HistoryCursor + 1);
    if (HistoryCursor >= CommandHistory.Num())
        CommandInput->SetText(FText::GetEmpty());
    else
        CommandInput->SetText(FText::FromString(CommandHistory[HistoryCursor]));
}

void UScriptingConsoleWidget::PrintLine(const FString& Line, bool bIsError)
{
    if (!OutputScrollBox) return;

    UTextBlock* TextLine = NewObject<UTextBlock>(this);
    TextLine->SetText(FText::FromString(Line));
    if (bIsError) TextLine->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.25f, 0.25f)));
    OutputScrollBox->AddChild(TextLine);
    OutputScrollBox->ScrollToEnd();
}

void UScriptingConsoleWidget::OnExecuteClicked() { ExecuteCurrentInput(); }

void UScriptingConsoleWidget::OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter)
        ExecuteCurrentInput();
}
