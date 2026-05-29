#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ModManifest.h"
#include "ModLoadingSubsystem.generated.h"

UENUM(BlueprintType)
enum class EModState : uint8 { NotLoaded, Loading, Loaded, Failed };

USTRUCT(BlueprintType)
struct FLoadedMod
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FITModManifest Manifest;
    UPROPERTY(BlueprintReadOnly) EModState      State = EModState::NotLoaded;
    UPROPERTY(BlueprintReadOnly) FString        ModPath;
    UPROPERTY(BlueprintReadOnly) FDateTime      LoadedAt;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModLoaded,  const FString&, ModID, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModUnloaded, const FString&, ModID);

UCLASS()
class INFINITYTABLE_API UModLoadingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category="Mods")
    bool LoadMod(const FString& ModPath);

    UFUNCTION(BlueprintCallable, Category="Mods")
    bool UnloadMod(const FString& ModID);

    UFUNCTION(BlueprintCallable, Category="Mods")
    TArray<FLoadedMod> GetLoadedMods() const;

    UFUNCTION(BlueprintCallable, Category="Mods")
    bool IsModLoaded(const FString& ModID) const;

    UFUNCTION(BlueprintCallable, Category="Mods")
    FString GetModsDirectory() const;

    UFUNCTION(BlueprintCallable, Category="Mods")
    void ScanAndLoadAll();

    UPROPERTY(BlueprintAssignable) FOnModLoaded   OnModLoaded;
    UPROPERTY(BlueprintAssignable) FOnModUnloaded OnModUnloaded;

private:
    TMap<FString, FLoadedMod> LoadedMods;

    bool ParseManifest(const FString& ModPath, FITModManifest& OutManifest);
    bool LoadModAssets(const FLoadedMod& Mod);
    bool ExecuteModScript(const FLoadedMod& Mod);
};
