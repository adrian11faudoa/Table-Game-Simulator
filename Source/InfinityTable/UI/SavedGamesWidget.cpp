#include "UI/SavedGamesWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Misc/Guid.h"

void USavedGamesWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UGameInstance* GI = GetGameInstance())
    {
        SaveSystem = GI->GetSubsystem<USaveGameSubsystem>();
    }

    if (SaveButton)    SaveButton->OnClicked.AddDynamic(this, &USavedGamesWidget::OnSaveClicked);
    if (RefreshButton) RefreshButton->OnClicked.AddDynamic(this, &USavedGamesWidget::OnRefreshClicked);

    RefreshSaveList();
}

FString USavedGamesWidget::GenerateSlotID() const
{
    return FGuid::NewGuid().ToString(EGuidFormats::Digits).Left(12);
}

void USavedGamesWidget::RefreshSaveList()
{
    if (!SaveSystem) return;
    TArray<FSaveSlotInfo> Slots = SaveSystem->GetSaveSlots();
    OnSaveListUpdated(Slots);
}

void USavedGamesWidget::SaveToNewSlot()
{
    if (!SaveSystem) return;

    FString DisplayName = SaveNameInput ? SaveNameInput->GetText().ToString() : FString();
    if (DisplayName.IsEmpty())
        DisplayName = FString::Printf(TEXT("Save %s"), *FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M")));

    bool bOK = SaveSystem->SaveTable(GenerateSlotID(), DisplayName);
    FString Msg = bOK ? TEXT("Game saved.") : TEXT("Save failed.");
    if (StatusText) StatusText->SetText(FText::FromString(Msg));
    OnOperationResult(bOK, Msg);
    RefreshSaveList();
}

void USavedGamesWidget::LoadSlot(const FString& Slot)
{
    if (!SaveSystem) return;
    bool bOK = SaveSystem->LoadTable(Slot);
    FString Msg = bOK ? TEXT("Game loaded.") : TEXT("Load failed — slot not found or corrupt.");
    if (StatusText) StatusText->SetText(FText::FromString(Msg));
    OnOperationResult(bOK, Msg);
}

void USavedGamesWidget::DeleteSlot(const FString& Slot)
{
    if (!SaveSystem) return;
    bool bOK = SaveSystem->DeleteSlot(Slot);
    FString Msg = bOK ? TEXT("Save deleted.") : TEXT("Delete failed.");
    if (StatusText) StatusText->SetText(FText::FromString(Msg));
    OnOperationResult(bOK, Msg);
    RefreshSaveList();
}

void USavedGamesWidget::OverwriteSlot(const FString& Slot)
{
    if (!SaveSystem) return;

    // Preserve the existing display name on overwrite, rather than blanking
    // it, by looking it up from the current slot list first.
    FString DisplayName = Slot;
    for (const FSaveSlotInfo& Info : SaveSystem->GetSaveSlots())
    {
        if (Info.Slot == Slot) { DisplayName = Info.Name; break; }
    }

    bool bOK = SaveSystem->SaveTable(Slot, DisplayName);
    FString Msg = bOK ? TEXT("Save overwritten.") : TEXT("Overwrite failed.");
    if (StatusText) StatusText->SetText(FText::FromString(Msg));
    OnOperationResult(bOK, Msg);
    RefreshSaveList();
}

void USavedGamesWidget::OnSaveClicked()    { SaveToNewSlot(); }
void USavedGamesWidget::OnRefreshClicked() { RefreshSaveList(); }
