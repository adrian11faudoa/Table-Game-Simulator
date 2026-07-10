#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ObjectBrowserWidget.generated.h"

class UTileView;
class UEditableTextBox;
class UTableSpawnManager;

USTRUCT(BlueprintType)
struct FObjectBrowserEntry
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FString TypeID;
    UPROPERTY(BlueprintReadOnly) FString DisplayName;
    UPROPERTY(BlueprintReadOnly) FString Category;
    UPROPERTY(BlueprintReadOnly) UTexture2D* Thumbnail = nullptr;
};

UCLASS()
class INFINITYTABLE_API UObjectBrowserWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable) void RefreshObjectList();
    UFUNCTION(BlueprintCallable) void FilterByCategory(const FString& Category);
    UFUNCTION(BlueprintCallable) void FilterByText(const FString& SearchText);
    UFUNCTION(BlueprintCallable) void SpawnSelectedObject(const FString& TypeID);

    UFUNCTION(BlueprintImplementableEvent) void OnObjectListUpdated(const TArray<FObjectBrowserEntry>& Entries);

protected:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UTileView* ObjectTileView;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UEditableTextBox* SearchBox;

private:
    TArray<FObjectBrowserEntry> AllEntries;
    TArray<FObjectBrowserEntry> FilteredEntries;
    FString ActiveCategory = TEXT("All");
    FString ActiveSearch;
    void ApplyFilters();
};
