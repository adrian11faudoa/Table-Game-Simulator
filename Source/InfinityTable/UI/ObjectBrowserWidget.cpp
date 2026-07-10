#include "UI/ObjectBrowserWidget.h"
#include "Components/TileView.h"
#include "Components/EditableTextBox.h"
#include "Objects/TableSpawnManager.h"
#include "Assets/AssetImportSubsystem.h"
#include "Core/ITPlayerController.h"
#include "Kismet/GameplayStatics.h"

void UObjectBrowserWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (SearchBox)
    {
        SearchBox->OnTextChanged.AddDynamic(this, &UObjectBrowserWidget::FilterByText);
    }
    RefreshObjectList();
}

void UObjectBrowserWidget::RefreshObjectList()
{
    AllEntries.Empty();

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UTableSpawnManager* Spawner = GI->GetSubsystem<UTableSpawnManager>())
        {
            for (const FString& TypeID : Spawner->GetRegisteredTypeIDs())
            {
                FObjectBrowserEntry Entry;
                Entry.TypeID      = TypeID;
                Entry.DisplayName = FName(*TypeID).ToString();
                Entry.Category    = TypeID.Contains(TEXT("d")) && TypeID.Len() <= 3 ? TEXT("Dice")
                                   : TypeID == TEXT("card")      ? TEXT("Cards")
                                   : TypeID == TEXT("deck")      ? TEXT("Cards")
                                   : TypeID == TEXT("discard_pile") ? TEXT("Cards")
                                   : TypeID == TEXT("miniature") ? TEXT("Miniatures")
                                   : TypeID == TEXT("token")     ? TEXT("Tokens")
                                   : TEXT("Misc");
                AllEntries.Add(Entry);
            }
        }

        // User-imported custom meshes (GLTF/GLB are spawnable at runtime via
        // glTFRuntime; FBX/OBJ are catalogued but not yet runtime-spawnable —
        // see Docs/Asset_Import_Guide.md) surface here as their own category
        // so players can browse and place content they brought in themselves.
        if (UAssetImportSubsystem* Assets = GI->GetSubsystem<UAssetImportSubsystem>())
        {
            for (const FImportedAssetInfo& Asset : Assets->GetImportedAssets(EImportedAssetType::Mesh3D))
            {
                FObjectBrowserEntry Entry;
                Entry.TypeID      = Asset.AssetID;
                Entry.DisplayName = Asset.DisplayName;
                Entry.Category    = Asset.Category;
                AllEntries.Add(Entry);
            }
        }
    }
    ApplyFilters();
}

void UObjectBrowserWidget::FilterByCategory(const FString& Category)
{
    ActiveCategory = Category;
    ApplyFilters();
}

void UObjectBrowserWidget::FilterByText(const FString& SearchText)
{
    ActiveSearch = SearchText;
    ApplyFilters();
}

void UObjectBrowserWidget::ApplyFilters()
{
    FilteredEntries.Empty();
    for (const FObjectBrowserEntry& Entry : AllEntries)
    {
        bool bCategoryMatch = (ActiveCategory == TEXT("All") || Entry.Category == ActiveCategory);
        bool bSearchMatch = ActiveSearch.IsEmpty() || Entry.DisplayName.Contains(ActiveSearch);
        if (bCategoryMatch && bSearchMatch)
            FilteredEntries.Add(Entry);
    }

    if (ObjectTileView)
    {
        ObjectTileView->ClearListItems();
        // ListItems would be populated via UObject wrapper in a full Blueprint implementation
    }
    OnObjectListUpdated(FilteredEntries);
}

void UObjectBrowserWidget::SpawnSelectedObject(const FString& TypeID)
{
    if (AITPlayerController* PC = Cast<AITPlayerController>(GetOwningPlayer()))
    {
        if (UGameInstance* GI = GetGameInstance())
        {
            if (UTableSpawnManager* Spawner = GI->GetSubsystem<UTableSpawnManager>())
            {
                FVector SpawnLoc = PC->GetPawn() ? PC->GetPawn()->GetActorLocation() + FVector(0,0,150.f) : FVector::ZeroVector;
                Spawner->SpawnObject(TypeID, SpawnLoc, FRotator::ZeroRotator);
            }
        }
    }
}
