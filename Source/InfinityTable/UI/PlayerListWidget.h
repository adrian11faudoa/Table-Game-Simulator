#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/ITGameMode.h"
#include "PlayerListWidget.generated.h"

class UListView;
class UButton;
class UTextBlock;

USTRUCT(BlueprintType)
struct FPlayerListEntry
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FString DisplayName;
    UPROPERTY(BlueprintReadOnly) EPlayerRole Role = EPlayerRole::Player;
    UPROPERTY(BlueprintReadOnly) FLinearColor PlayerColor = FLinearColor::White;
    UPROPERTY(BlueprintReadOnly) bool bIsLocalPlayer = false;

    // Not BlueprintReadOnly-exposed by value (raw actor pointers in a UMG
    // list item are fine for native C++ use, but this struct is primarily
    // consumed by the OnPlayerListUpdated Blueprint event for *display*;
    // actual kick/promote actions go through KickByName/PromoteByName below
    // rather than requiring Blueprint to round-trip a raw APlayerController*).
    APlayerController* Controller = nullptr;
};

/**
 * In-session player roster — the panel the prompt's "Multiplayer
 * moderation" (SECURITY) and "player permissions" (MULTIPLAYER NETWORKING)
 * requirements actually need a UI surface for. AITGameMode::KickPlayer/
 * BanPlayer/SetPlayerPermission already existed (or were added) as backend
 * functions; this widget is what makes them clickable.
 */
UCLASS()
class INFINITYTABLE_API UPlayerListWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable) void RefreshPlayerList();
    UFUNCTION(BlueprintCallable) bool KickByName(const FString& DisplayName, const FString& Reason);
    UFUNCTION(BlueprintCallable) bool BanByName(const FString& DisplayName, const FString& Reason);
    UFUNCTION(BlueprintCallable) bool PromoteByName(const FString& DisplayName, EPlayerRole NewRole);

    UFUNCTION(BlueprintImplementableEvent) void OnPlayerListUpdated(const TArray<FPlayerListEntry>& Players);
    UFUNCTION(BlueprintImplementableEvent) void OnModerationResult(bool bSuccess, const FString& Message);

protected:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UButton*    RefreshButton;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UTextBlock* StatusText;

private:
    UFUNCTION() void OnRefreshClicked();
    APlayerController* FindControllerByName(const FString& DisplayName) const;
    bool HasModerationPermission() const;
};
