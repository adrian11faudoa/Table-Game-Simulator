#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Mod/ModManifest.h"
#include "Assets/AssetImportSubsystem.h"
#include "WorkshopWidget.generated.h"

class UListView;
class UButton;
class UTextBlock;
class UEditableTextBox;
class UModLoadingSubsystem;
class UAssetImportSubsystem;

UCLASS()
class INFINITYTABLE_API UWorkshopWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable) void RefreshModList();
    UFUNCTION(BlueprintCallable) void RefreshAssetList();
    UFUNCTION(BlueprintCallable) void LoadModByFolder(const FString& ModFolder);
    UFUNCTION(BlueprintCallable) void UnloadMod(const FString& ModID);
    UFUNCTION(BlueprintCallable) void ImportAssetFromPath(const FString& FilePath);
    UFUNCTION(BlueprintCallable) void RemoveImportedAsset(const FString& AssetID);

    UFUNCTION(BlueprintImplementableEvent) void OnAvailableModsUpdated(const TArray<FString>& ModFolders);
    UFUNCTION(BlueprintImplementableEvent) void OnLoadedModsUpdated(const TArray<FITModManifest>& Mods);
    UFUNCTION(BlueprintImplementableEvent) void OnImportedAssetsUpdated(const TArray<FImportedAssetInfo>& Assets);
    UFUNCTION(BlueprintImplementableEvent) void OnWorkshopOperationResult(bool bSuccess, const FString& Message);

protected:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UButton*    RefreshModsButton;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UButton*    ImportAssetButton;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UEditableTextBox* ImportPathInput;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UTextBlock* StatusText;

private:
    UFUNCTION() void OnRefreshModsClicked();
    UFUNCTION() void OnImportAssetClicked();

    UPROPERTY() UModLoadingSubsystem*  Mods   = nullptr;
    UPROPERTY() UAssetImportSubsystem* Assets = nullptr;

    UFUNCTION() void HandleAssetImported(FImportedAssetInfo AssetInfo, EAssetImportResult Result);
};
