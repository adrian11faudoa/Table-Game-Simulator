#include "UI/WorkshopWidget.h"
#include "Mod/ModLoadingSubsystem.h"
#include "Assets/AssetImportSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"

void UWorkshopWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UGameInstance* GI = GetGameInstance())
    {
        Mods   = GI->GetSubsystem<UModLoadingSubsystem>();
        Assets = GI->GetSubsystem<UAssetImportSubsystem>();
    }

    if (Assets)
    {
        Assets->OnAssetImported.AddDynamic(this, &UWorkshopWidget::HandleAssetImported);
    }

    if (RefreshModsButton) RefreshModsButton->OnClicked.AddDynamic(this, &UWorkshopWidget::OnRefreshModsClicked);
    if (ImportAssetButton) ImportAssetButton->OnClicked.AddDynamic(this, &UWorkshopWidget::OnImportAssetClicked);

    RefreshModList();
    RefreshAssetList();
}

void UWorkshopWidget::RefreshModList()
{
    if (!Mods) return;

    TArray<FString> Available = Mods->DiscoverMods();
    OnAvailableModsUpdated(Available);

    TArray<FITModManifest> Loaded = Mods->GetLoadedMods();
    OnLoadedModsUpdated(Loaded);
}

void UWorkshopWidget::RefreshAssetList()
{
    if (!Assets) return;
    OnImportedAssetsUpdated(Assets->GetAllImportedAssets());
}

void UWorkshopWidget::LoadModByFolder(const FString& ModFolder)
{
    if (!Mods)
    {
        if (StatusText) StatusText->SetText(FText::FromString(TEXT("Mod system unavailable.")));
        OnWorkshopOperationResult(false, TEXT("Mod system unavailable."));
        return;
    }

    FString ValidationError;
    if (!Mods->ValidateMod(ModFolder, ValidationError))
    {
        FString Msg = FString::Printf(TEXT("Mod failed validation: %s"), *ValidationError);
        if (StatusText) StatusText->SetText(FText::FromString(Msg));
        OnWorkshopOperationResult(false, Msg);
        return;
    }

    bool bOK = Mods->LoadMod(ModFolder);
    FString Msg = bOK ? TEXT("Mod loaded.") : TEXT("Mod failed to load — see server log.");
    if (StatusText) StatusText->SetText(FText::FromString(Msg));
    OnWorkshopOperationResult(bOK, Msg);
    RefreshModList();
}

void UWorkshopWidget::UnloadMod(const FString& ModID)
{
    if (!Mods) return;
    bool bOK = Mods->UnloadMod(ModID);
    FString Msg = bOK ? TEXT("Mod unloaded.") : TEXT("Mod was not loaded.");
    if (StatusText) StatusText->SetText(FText::FromString(Msg));
    OnWorkshopOperationResult(bOK, Msg);
    RefreshModList();
}

void UWorkshopWidget::ImportAssetFromPath(const FString& FilePath)
{
    if (!Assets)
    {
        OnWorkshopOperationResult(false, TEXT("Asset import system unavailable."));
        return;
    }

    FImportedAssetInfo Info;
    // Result is reported via HandleAssetImported, bound to
    // Assets->OnAssetImported, so we don't duplicate the status update here.
    Assets->ImportAsset(FilePath, Info);
}

void UWorkshopWidget::RemoveImportedAsset(const FString& AssetID)
{
    if (!Assets) return;
    bool bOK = Assets->RemoveImportedAsset(AssetID);
    FString Msg = bOK ? TEXT("Asset removed.") : TEXT("Asset not found.");
    if (StatusText) StatusText->SetText(FText::FromString(Msg));
    OnWorkshopOperationResult(bOK, Msg);
    RefreshAssetList();
}

void UWorkshopWidget::HandleAssetImported(FImportedAssetInfo AssetInfo, EAssetImportResult Result)
{
    FString Msg;
    switch (Result)
    {
        case EAssetImportResult::Success:           Msg = FString::Printf(TEXT("Imported '%s'."), *AssetInfo.DisplayName); break;
        case EAssetImportResult::UnsupportedFormat:  Msg = TEXT("Unsupported file format.");  break;
        case EAssetImportResult::FileTooLarge:       Msg = TEXT("File exceeds size limit.");  break;
        case EAssetImportResult::FileNotFound:       Msg = TEXT("File not found.");           break;
        case EAssetImportResult::ImportFailed:       Msg = TEXT("Import failed.");             break;
    }

    if (StatusText) StatusText->SetText(FText::FromString(Msg));
    OnWorkshopOperationResult(Result == EAssetImportResult::Success, Msg);

    if (Result == EAssetImportResult::Success)
        RefreshAssetList();
}

void UWorkshopWidget::OnRefreshModsClicked() { RefreshModList(); }

void UWorkshopWidget::OnImportAssetClicked()
{
    if (ImportPathInput)
        ImportAssetFromPath(ImportPathInput->GetText().ToString());
}
